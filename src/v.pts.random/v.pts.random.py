#!/usr/bin/env python

#
############################################################################
#
# MODULE:      v.random
# AUTHOR(S):   Christian Kaiser
# PURPOSE:     Selects random points inside a raster image with the frequency
#              proportional to the raster value
# VERSION:     1.0.0 (2011-11-14)
#
# COPYRIGHT:   (C) 2011 by Christian Kaiser
#
#############################################################################
#
# REQUIREMENTS:
#      - numpy: http://numpy.scipy.org





import math as m
import numpy as np
from optparse import OptionParser
import os
import osgeo.gdal as gdal
import shutil
import struct
import sys


USAGE = """v.pts.random --input input_raster --output output_txt --npts number_of_points --error_prob error_probability"""


def random_points(options):
    # Check the input options
    infile = options.input
    if infile == None:
        raise Exception("Error. You must provide a input raster file.")
    outfile = options.output
    if outfile == None:
        raise Exception("Error. You must provide a path to an output raster file.")
    npts = int(options.npts)
    if options.error_prob is not None:
        err_prob = float(options.error_prob)
    else:
        err_prob = 0.0
    
    # Read the input raster into a numpy array.
    in_ds = gdal.Open(infile, gdal.GA_ReadOnly)
    if in_ds == None:
        raise Exception("Error. Unable to open input raster file.")
    in_driver = in_ds.GetDriver()
    georef = in_ds.GetGeoTransform()
    xres, yres = (georef[1], -1*georef[5])
    xmin, ymax = georef[0], georef[3]
    in_band = in_ds.GetRasterBand(1)
    xsize, ysize = (in_band.XSize, in_band.YSize)
    ymin = ymax - (ysize*yres)
    xmax = xmin + (xsize*xres)
    in_data = in_band.ReadRaster(
        0, 0, in_band.XSize, in_band.YSize, 
        in_band.XSize, in_band.YSize, gdal.GDT_Float32
    )
    data_array = struct.unpack('f' * xsize * ysize, in_data)
    rast = np.array(data_array, dtype=float)
    rast = rast.reshape((ysize, xsize)).T
    zmax = np.max(rast)
    npts_generated = 0
    fout = open(outfile, 'w')
    fout.write("x\ty\n")
    while npts_generated < npts:
        cx = np.random.rand() * xsize
        cy = np.random.rand() * ysize
        cz = np.random.rand() * zmax
        err = np.random.rand()
        if err < err_prob or cz <= rast[int(cx), int(cy)]:
            wx = ((cx/xsize) * (xmax - xmin)) + xmin
            wy = (((ysize-cy)/ysize) * (ymax - ymin)) + ymin
            fout.write("%f\t%f\n" % (wx, wy))
            npts_generated += 1
    fout.close()
    






if __name__ == "__main__":
    parser = OptionParser(usage=USAGE)
    parser.add_option(
        '-i', '--input', dest="input",
        help="GDAL compatible input raster file",
        metavar="INPUT_RASTER"
    )
    parser.add_option(
        '-o', '--output', dest='output',
        help="Path to output text file",
        metavar="OUTPUT_TXT"
    )
    parser.add_option(
        '-n', '--npts', dest='npts',
        help="Number of random points to generate",
        metavar="N_PTS"
    )
    parser.add_option(
        '-e', '--error_prob', dest='error_prob',
        help="Probability of error of respecting frequency distribution",
        metavar="ERROR"
    )
    (options, args) = parser.parse_args()
    if options.input == None and options.output == None:
        print USAGE
        sys.exit(0)
    print("v.pts.random starting...")
    random_points(options)
    print("v.pts.random done.")



