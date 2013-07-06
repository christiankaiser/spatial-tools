#!/usr/bin/env python

#
############################################################################
#
# MODULE:      r.to.image
# AUTHOR(S):   Christian Kaiser
# PURPOSE:     Converts a raster to a PNG image.
# VERSION:     1.0.0 (08.02.2011)
#
# COPYRIGHT:   (C) 2011 by Christian Kaiser
#
#############################################################################
#
# REQUIREMENTS:
#      - numpy: http://numpy.scipy.org



"""
Applies a Python instruction to a GDAL-compatible raster image.
Input array is in and output should be stored in out
"""

import numpy as np
import numpy
from numpy import *

from optparse import OptionParser
import os
import osgeo.gdal as gdal
import shutil
import struct
import sys


USAGE = """
r.pycalc --input INPUT_RASTER 
         --output OUTPUT_RASTER 
         --instruction PYTHON_INSTRUCTION
The Python instruction can use the input Numpy raster array in variable 'rast'.
EXAMPLES:
    Computing the logarithm of an image:
        r.pycalc ... --instruction 'log(rast)'
    Putting all values of 2 or smaller to 0:
        r.pycalc ... --instruction 'where(rast > 2, rast, 0)'
"""


def pycalc(options):
    # Check the input options
    infile = options.input
    if infile == None:
        print("Error. You must provide a input raster file.")
        sys.exit(1)
    outfile = options.output
    if outfile == None:
        print("Error. You must provide a path to an output file.")
        sys.exit(1)
    instruction = options.instruction
    if instruction == None:
        print("Error. No instruction provided.")
        sys.exit(1)
    
    # Read the input raster into a numpy array.
    in_ds = gdal.Open(infile, gdal.GA_ReadOnly)
    if in_ds == None:
        raise Exception("Error. Unable to open input raster file.")
    in_driver = in_ds.GetDriver()
    in_band = in_ds.GetRasterBand(1)
    size = (in_band.XSize, in_band.YSize)
    in_data = in_band.ReadRaster(
        0, 0, in_band.XSize, in_band.YSize, 
        in_band.XSize, in_band.YSize, gdal.GDT_Float32
    )
    data_array = struct.unpack('f' * size[0] * size[1], in_data)
    rast = np.array(data_array, dtype=float)
    rast = rast.reshape((size[1], size[0]))
    
    # Execute the user input
    rast = eval(instruction)
    
    # Save the result into the output file
    
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
    out_band.WriteArray(rast)
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
        help="Path to output image file",
        metavar="OUTPUT_IMAGE"
    )
    parser.add_option(
        '-x', '--instruction',
        help="Python instruction to execute",
        metavar="PYTHON_INSTRUCTION"
    )
    (options, args) = parser.parse_args()
    if options.input == None and options.output == None:
        print USAGE
        sys.exit(0)
    print("r.pycalc starting...")
    pycalc(options)
    print("r.pycalc done")



