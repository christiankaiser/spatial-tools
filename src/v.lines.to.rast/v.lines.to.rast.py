#!/usr/bin/env python
"""
v.line.to.rast

AUTHOR: Christian Kaiser <chri.kais@gmail.com>
COPYRIGHT: Released under the GNU General Public License (GPL) version 3
"""



import numpy as np
from optparse import OptionParser
import osgeo.gdal as gdal


USAGE = """
v.lines.to.rast 
    --input line_file --output output_raster 
    --xmin xmin --xmax xmax --ymin ymin --ymax ymax --res res 
    [--weighted] [--sum]

    --input line_file
        The line file is a simple tab-separated text file with one line per
        line :-) Line is defined with x-origin, y-origin, x-destination,
        y-destination and an attribute value. The first line is assumed to
        be a header line.
    
    --output output_raster
        The output raster image. This is going to be a georeferenced ERDAS
        Imagine file readable with most GIS software (e.g. QGIS).
    
    --xmin xmin
    --xmax xmax
    --ymin ymin
    --ymax ymax
        The bounding box for the resulting output raster.
    
    --res res
        The resolution (cell size) of the resulting output raster.
    
    --weighted
        Optional. Flag saying if the cell values should be weighted by the
        length of the line portion inside the cell. Default is treating all
        lines equally.
    
    --sum
        Optional. Compute the sum (weighted or not) of the attribute values
        for all lines falling inside the cell. If not given, the average is
        computed instead.
"""



class Grid:
    def __init__(self, xmin, xmax, ymin, ymax, resolution, nbands=1):
        self.xmin = float(xmin)
        self.ymin = float(ymin)
        self.res = float(resolution)
        # Adjust maximum values to fit the resolution values
        self.nx = int(np.ceil((float(xmax) - float(xmin)) / self.res))
        self.ny = int(np.ceil((float(ymax) - float(ymin)) / self.res))
        self.xmax = self.xmin + (self.nx * self.res)
        self.ymax = self.ymin + (self.ny * self.res)
        # Create the bands
        self.values = np.zeros((nbands, self.nx, self.ny))

    def intersectLine(self, l):
        i1,j1 = self.worldToGrid((l.x1, l.y1))
        i2,j2 = self.worldToGrid((l.x2, l.y2))
        imin, imax = np.min([i1, i2]), np.max([i1, i2])
        jmin, jmax = np.min([j1, j2]), np.max([j1, j2])
        ts = []
        i, j = int(np.ceil(imin)), int(np.ceil(jmin))
        while i < imax:
            ts.append(l.tAtCoordX((i*self.res) + self.xmin))
            i += 1
        while j < jmax:
            ts.append(l.tAtCoordY((j*self.res) + self.ymin))
            j += 1
        ts.append(1.0)
        ts.sort()
        return ts
    
    def worldToGrid(self, c):
        cx, cy = c
        i = int((cx - self.xmin) / self.res)
        j = int((cy - self.ymin) / self.res)
        return i,j
        


class Line:
    def __init__(self, x1, y1, x2, y2):
        self.x1 = float(x1)
        self.y1 = float(y1)
        self.x2 = float(x2)
        self.y2 = float(y2)
    
    def coordAtValueT(self, t):
        x = self.x1 + float(t) * (self.x2 - self.x1)
        y = self.y1 + float(t) * (self.y2 - self.y1)
        return (x,y)
    
    def tAtCoordX(self, x):
        return (float(x) - self.x1) / (self.x2 - self.x1)
    
    def tAtCoordY(self, y):
        return (float(y) - self.y1) / (self.y2 - self.y1)




def lines2rast(opt):
    # Create the grid first
    g = Grid(opt.xmin, opt.xmax, opt.ymin, opt.ymax, opt.res, 4)
    # Start reading the lines from the text file
    f = open(opt.input)
    h = f.readline()
    cnt = 0
    for r in f:
        v = map(float, r.strip().split('\t'))
        l = Line(v[0], v[1], v[2], v[3])
        ts = g.intersectLine(l)
        t0 = 0.0
        for t1 in ts:
            t = (t0 + t1) / 2
            i,j = g.worldToGrid(l.coordAtValueT(t))
            w = t1 - t0
            if i >= 0 and j >= 0 and i < g.nx and j < g.ny:
                g.values[0, i, j] += v[4]       # sum of attribute values
                g.values[1, i, j] += 1          # number of lines
                g.values[2, i, j] += w * v[4]   # weighted sum of attribute values
                g.values[3, i, j] += w          # sum of weights
            t0 = t1     # Current t1 is next t0
        cnt += 1
        if cnt % 1000 == 0: print "Flows done: %i" % cnt
    # Write the grid out
    if opt.sum:
        if opt.weighted: rast = g.values[2,:,:]
        else: rast = g.values[0,:,:]
    else:
        if opt.weighted: rast = g.values[2,:,:] / g.values[3,:,:]
        else: rast = g.values[0,:,:] / g.values[1,:,:]
    np.save('flows.npy', g.values)
    # Create the GDAL file
    driver = gdal.GetDriverByName("HFA")
    ds = driver.Create(opt.output, g.nx, g.ny, 1, gdal.GDT_Float64)
    ds.SetGeoTransform([g.xmin, g.res, 0, g.ymax, 0, -g.res])
    band = ds.GetRasterBand(1)
    band.WriteArray(rast)
    ds = None
    


if __name__ == "__main__":
    parser = OptionParser(usage=USAGE)
    parser.add_option(
        '-i', '--input', dest="input",
        help="Path to line file",
        metavar="LINE_FILE"
    )
    parser.add_option(
        '-o', '--output',
        help="Path to output raster file",
        metavar="OUTPUT_RASTER"
    )
    parser.add_option(
        '-1', '--xmin', dest="xmin",
        help="Minimum x-coordinate for output raster",
        metavar="XMIN"
    )
    parser.add_option(
        '-2', '--xmax', dest="xmax",
        help="Maximum x-coordinate for output raster",
        metavar="XMAX"
    )
    parser.add_option(
        '-3', '--ymin', dest="ymin",
        help="Minimum y-coordinate for output raster",
        metavar="YMIN"
    )
    parser.add_option(
        '-4', '--ymax', dest="ymax",
        help="Maximum y-coordinate for output raster",
        metavar="YMAX"
    )
    parser.add_option(
        '-r', '--res', dest="res",
        help="Output raster resolution",
        metavar="RES"
    )
    parser.add_option(
        "-w", "--weighted",
        action="store_true", dest="weighted", default=False,
        help="Weight the line length inside the raster cells"
    )
    parser.add_option(
        "-s", "--sum",
        action="store_true", dest="sum", default=False,
        help="Compute sum instead of average"
    )

    (options, args) = parser.parse_args()
    if options.input == None and options.output == None:
        print USAGE
        sys.exit(0)
    print("v.lines.to.rast starting...")
    lines2rast(options)
    print("v.lines.to.rast done.")




