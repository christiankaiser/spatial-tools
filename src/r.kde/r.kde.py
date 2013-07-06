#!/usr/bin/env python

#
############################################################################
#
# MODULE:      r.kde
# AUTHOR(S):   Christian Kaiser
# PURPOSE:     Computes the Gaussian kernel density for a raster map.
# VERSION:     1.0.0 (2010-01-09)
#
# COPYRIGHT:   (C) 2010 by Christian Kaiser
#
#############################################################################
#
# REQUIREMENTS:
#      - numpy: http://numpy.scipy.org



"""
If a bandwidth of 0 is provided, it will be estimated automatically using the 
formula provided in Fotherhingham, S., Brundson, C., and Charlton, M. (2000). 
Quantitative geography: perspectives on spatial data analysis. Los Angeles: 
Sage, p. 149. Automatic bandwidth estimation is default.
"""


import math as m
import numpy as np
from optparse import OptionParser
import os
import osgeo.gdal as gdal
import shutil
import struct
import sys


USAGE = """r.kde --input input_raster --output output_raster [--bandwidth bandwidth] --normalize"""


def kde(options):
    # Check the input options
    infile = options.input
    if infile == None:
        raise Exception("Error. You must provide a input raster file.")
    outfile = options.output
    if outfile == None:
        raise Exception("Error. You must provide a path to an output raster file.")
    bandwidth = float(options.bandwidth)
    normalise = True
    if options.normalize == None:
        normalise = False
    
    # Read the input raster into a numpy array.
    in_ds = gdal.Open(infile, gdal.GA_ReadOnly)
    if in_ds == None:
        raise Exception("Error. Unable to open input raster file.")
    in_driver = in_ds.GetDriver()
    georef = in_ds.GetGeoTransform()
    res = (georef[1], -1*georef[5])
    in_band = in_ds.GetRasterBand(1)
    size = (in_band.XSize, in_band.YSize)
    in_data = in_band.ReadRaster(
        0, 0, in_band.XSize, in_band.YSize, 
        in_band.XSize, in_band.YSize, gdal.GDT_Float32
    )
    data_array = struct.unpack('f' * size[0] * size[1], in_data)
    rast = np.array(data_array, dtype=float)
    rast = rast.reshape((size[1], size[0])).T
    
    # Find the number of non-zero raster cells in the input data.
    npts = np.sum(rast > 0)
    
    # Estimate the bandwidth if it is not given. 
    # For the formula, see Fotheringham et al. 2000.
    if bandwidth == None or bandwidth <= 0:
        print("   No bandwidth provided. Estimating bandwidth...")
        # We need to get the coordinates of all non-zero points.
        # Do it separately for x and y coordinates for minimizing 
        # memory requirements.
        coords = []
        for j in range(0, size[1]):
            for i in range(0, size[0]):
                if rast[i,j] > 0:
                    coords.append((i * res[0]) + georef[0])
        std_x = np.std(coords)
        del(coords)
        coords = []
        for j in range(0, size[1]):
            for i in range(0, size[0]):
                if rast[i,j] > 0:
                    coords.append(georef[3] - (j * res[1]))
        std_y = np.std(coords)
        del(coords)
        std = (std_x + std_y) / 2.0
        bandwidth = (2.0 / (3.0 * npts))**0.25 * std
        print("   Estimated bandwidth [in map units]: %f" % bandwidth)
    
    # We compute the values only up to a distance of 20 * bandwidth from the 
    # centre for performance reasons.
    maxdist = 20 * bandwidth
    maxdist_cells = np.max(maxdist / np.min(res))
    maxdist_cells = int(maxdist_cells+1)        # Round to integer
    if maxdist_cells > np.max(size):
        maxdist_cells = int(np.max(size))
    
    # We can now create the distance matrix for our kernel.
    print("   Computing the distance matrix for the kernel...")
    dist_mat = np.matrix(np.zeros((2*maxdist_cells+1, 2*maxdist_cells+1)))
    for i in range(0, 2*maxdist_cells+1):
        for j in range(0, 2*maxdist_cells+1):
            cell = np.array([
                float(maxdist_cells - i) * res[0], 
                float(maxdist_cells - j) * res[1]
            ])
            dist_mat[i,j] = np.sum(cell * cell)
    
    # Compute the Gaussian kernel for our window
    print('   Computing the Gaussian kernel...')
    k = np.exp((-1 * dist_mat) / (2 * bandwidth * bandwidth))
    
    # Normalise if wanted
    if normalise:
        print('   Normalising kernel density...')
        norm_const = (1.0 / m.sqrt(2.0 * m.pi * bandwidth*bandwidth)) / npts
        k *= norm_const
    
    # Create the output matrix
    kde = np.zeros(shape=size, dtype=float)
    
    # Apply the Gaussian kernel raster points
    print('   Applying the Gaussian kernel to the input raster...')
    cnt = 0
    maxcnt = size[0] * size[1]
    pct_done = 0
    for j in range(0, size[1]):
        for i in range(0, size[0]):
            new_pct_done = int((float(cnt) * 10.0) / float(maxcnt))
            if new_pct_done > pct_done:
                print("      %d0%% done" % new_pct_done)
                pct_done = new_pct_done
            cnt += 1
            if rast[i,j] > 0:
                k1 = rast[i,j] * k
                kde_minx = np.max((i - maxdist_cells, 0))
                kde_maxx = np.min((i + maxdist_cells + 1, size[0]))
                kde_miny = np.max((j - maxdist_cells, 0))
                kde_maxy = np.min((j + maxdist_cells + 1, size[1]))
                k1_minx = kde_minx - (i - maxdist_cells)
                k1_maxx =  np.min([
                    ((2 * maxdist_cells + 1) - ((i + maxdist_cells + 1) - size[0])), 
                    2 * maxdist_cells + 1
                ])
                k1_miny = kde_miny - (j - maxdist_cells)
                k1_maxy = np.min([
                    ((2 * maxdist_cells + 1) - ((j + maxdist_cells + 1) - size[1])), 
                    2 * maxdist_cells + 1])
                kde[kde_minx:kde_maxx, kde_miny:kde_maxy] += k1[k1_minx:k1_maxx, k1_miny:k1_maxy]
    
    # Write out the grid values to a copy of the input raster file
    print('   Writing the kernel density to the output raster file...')
    
    # Check if the input dataset driver is able to create a copy,
    # and create the copy
    drvr_info = in_driver.GetMetadata()
    if (drvr_info.has_key(gdal.DCAP_CREATECOPY) and 
            drvr_info[gdal.DCAP_CREATECOPY] == 'YES'):
        out_ds = in_driver.CreateCopy(outfile, in_ds, 0)
    else:
        in_format = in_driver.LongName
        print("   Warning. '%s' driver cannot create new files." % in_format)
        print("            Writing in Erdas Imagine (HFA) format instead.")
        drvr = gdal.GetDriverByName('HFA')
        out_ds = drvr.CreateCopy(outfile, in_ds, 0)
    in_ds = None    # Close the input dataset; we don't need it anymore
    out_ds = None
    
    # Write the kde result to the GDAL file.
    out_ds = gdal.Open(outfile, gdal.GA_Update)
    out_band = out_ds.GetRasterBand(1)
    out_band.WriteArray(kde.T)
    out_ds = None






if __name__ == "__main__":
    parser = OptionParser(usage=USAGE)
    parser.add_option(
        '-i', '--input', dest="input",
        help="GDAL compatible input raster file",
        metavar="INPUT_RASTER"
    )
    parser.add_option(
        '-o', '--output',
        help="Path to output raster file",
        metavar="OUTPUT_RASTER"
    )
    parser.add_option(
        '-b', '--bandwidth',
        help="Kernel bandwidth",
        metavar="BANDWIDTH"
    )
    parser.add_option(
        '-n', '--normalize',
        help="Normalises the kernel density"
    )
    (options, args) = parser.parse_args()
    if options.input == None and options.output == None:
        print USAGE
        sys.exit(0)
    print("r.kde starting...")
    kde(options)
    print("r.kde done.")



