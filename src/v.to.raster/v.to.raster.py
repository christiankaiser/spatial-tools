#!/usr/bin/env python
"""
Converts a vector layer to a raster
"""

__version__ = '1.0.0'
__date__ = '2011-03-24'
__author__ = 'Christian Kaiser <chri.kais@gmail.com>'

import commands
import math
import osgeo.ogr as ogr
import osgeo.gdal as gdal
import osgeo.osr as osr
from optparse import OptionParser
import sys


USAGE = """v.to.raster 
    --input INPUT_LAYER --output OUTPUT_RASTER --attr ATTR 
    [--res RESOLUTION] [--size RASTERX,RASTERY] 
    [--envelope XMIN,YMIN,XMAX,YMAX]
    """


def vector_to_raster(ogrfile, attribute, rasterfile, rastersize=None, 
    res=None, minx=None, maxx=None, miny=None, maxy=None):
    """
    Transforms an OGR compatible vector layer into a raster layer in HFA 
    format. The value of the provided attribute is used as value for the 
    raster. This function is based on gdal_rasterize, so Python needs access 
    to this tool.
    """
    
    #print("vector_to_raster: opening %s" % ogrfile)
    # Open the vector file
    inds = ogr.Open(ogrfile)
    if inds is None:
        raise Exception("Unable to open %s\n" % ogrfile)
    
    # Check if there is at least one layer in the OGR datasource.
    nlyrs = inds.GetLayerCount()
    if nlyrs < 1:
        raise Exception("Data source %s does not have any layer.\n" % ogrfile)
        
    # Get the layer from the vector file
    #lyrname = os.path.splitext(os.path.basename(ogrfile))[0]
    #try:
    #   lyr = inds.GetLayerByName(lyrname)
    #except:
    lyr = inds.GetLayer(0)
    lyrname = lyr.GetLayerDefn().GetName()
    
    if lyr == None:
        raise Exception("Unable to open OGR layer in %s\n" % ogrfile)
    
    # We have to create a new raster dataset first.
    # Determine the extent of the vector file if the extent is not provided.
    if minx == None or maxx == None or miny == None or maxy == None:
        extent = lyr.GetExtent()
        if minx == None: minx = extent[0]
        if maxx == None: maxx = extent[1]
        if miny == None: miny = extent[2]
        if maxy == None: maxy = extent[3]
        if minx > maxx:
            minx = extent[0]
            maxx = extent[1]
        if miny > maxy:
            miny = extent[2]
            maxy = extent[3]
    
    # Compute the resolution if not provided
    if res is None:
        xres = (maxx - minx) / rastersize[0]
        yres = (maxy - miny) / rastersize[1]
        res = xres
        if yres > xres: res = yres
    
    # Adjust the raster size to fit the extent proportions
    sizex = int(math.ceil((maxx - minx) / res))
    sizey = int(math.ceil((maxy - miny) / res))
    
    # Create a new raster layer
    rasterDriver = gdal.GetDriverByName('HFA')
    outds = rasterDriver.Create(rasterfile, sizex, sizey, 1, gdal.GDT_Float64)
    rasterTransform = [minx, res, 0.0, maxy, 0.0, -res]
    outds.SetGeoTransform(rasterTransform)
    
    # Get projection of OGR file and assign to raster
    srs = osr.SpatialReference()
    srs.ImportFromWkt(lyr.GetSpatialRef().__str__())
    outds.SetProjection(srs.ExportToWkt())
    
    # Close the vector and raster files.
    inds = None
    outds = None
    
    # Execute gdal_rasterize
    commandString = "gdal_rasterize -a %s -l %s %s %s" % (attribute, lyrname, ogrfile, rasterfile)
    commandOutput = commands.getoutput(commandString)





if __name__ == "__main__":
    parser = OptionParser(usage=USAGE)
    parser.add_option(
        '-i', '--input', dest="input",
        help="OGR compatible input vector layer",
        metavar="INPUT_LAYER"
    )
    parser.add_option(
        '-o', '--output', dest="output",
        help="Path to output raster file",
        metavar="OUTPUT_RASTER"
    )
    parser.add_option(
        '-a', '--attr', dest="attr",
        help="Attribute name containing the pixel value",
        metavar="ATTR"
    )
    parser.add_option(
        '-r', '--res', dest="res",
        help="Raster pixel size (image resolution)"
    )
    parser.add_option(
        '-s', '--size', dest="size",
        help="Raster size"
    )
    parser.add_option(
        '-e', '--env', dest="env",
        help="Bounding box"
    )
    (options, args) = parser.parse_args()
    if options.size != None:
        size = map(int, options.size.split(','))
    else:
        size = None
    if options.res != None:
        res = float(options.res)
    else:
        res = None
    if options.env != None:
        xmin, ymin, xmax, ymax = map(float, options.env.split(','))
    else:
        xmin = ymin = xmax = ymax = None
    if options.input == None or options.output == None or options.attr == None:
        print USAGE
        sys.exit(0)
    print("v.to.raster starting...")
    vector_to_raster(
        ogrfile = options.input, 
        attribute = options.attr, 
        rasterfile = options.output, 
        rastersize = size, 
        res = res, 
        minx = xmin, maxx = xmax, miny = ymin, maxy = ymax
    )
    print("v.to.raster done")


