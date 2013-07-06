#!/usr/bin/env python
"""
Computes the adjacency matrix for a polygon vector layer.
"""

__version__ = '1.0.1'
__date__ = '2011-03-25'
__author__ = 'Christian Kaiser <chri.kais@gmail.com>'

import os
import osgeo.ogr as ogr
from optparse import OptionParser
import shutil
import sqlite3 as sqlite
import sys
import tempfile


USAGE = """v.poly.neighbours --input INPUT_LAYER --output OUTPUT_MATRIX --attr ATTR [--snapdist 0] --list"""


def poly_neighbours(layer, outfile, attr, snap_dist=0, l=False):
    ds = ogr.Open(layer)
    if ds is None:
        print "Error. Unable to open input vector layer."
        sys.exit(1)
    lyr = ds.GetLayerByIndex(0)
    lyr.ResetReading()
    feat_defn = lyr.GetLayerDefn()
    attr_idx = feat_defn.GetFieldIndex(attr)
    pts = {}
    for feat in lyr:
        fid = feat.GetFieldAsString(attr_idx)
        geom = feat.GetGeometryRef()
        geom_pts = get_points(geom)
        for p in geom_pts:
            x = p[0]
            y = p[1]
            # Round if snapping distance is not 0
            if snap_dist > 0:
                x = round(x / snap_dist) * snap_dist
                y = round(y / snap_dist) * snap_dist
            xy = "%0.8f/%0.8f" % (x, y)
            if pts.has_key(xy):
                pts[xy].append(fid)
            else:
                pts[xy] = [fid]
    ds = None
    # Make the list of neighbours based on the points
    if l is True:
        neighbours = neighbours_as_list(pts)
    else:
        neighbours = neighbours_as_pairs(pts)
    # Write the neighbours out
    f = open(outfile, "w")
    f.write("\n".join(neighbours))
    f.close()



def neighbours_as_pairs(pts):
    neighbours = []
    for p in pts:
        fids = pts[p]
        if len(fids) > 1:
            for i in range(len(fids)):
                for j in range(len(fids)):
                    if i < j:
                        if fids[i] != fids[j]:
                            n = ','.join(sorted([fids[i], fids[j]]))
                            if n not in neighbours:
                                neighbours.append(n)
    return neighbours
    

def neighbours_as_list(pts):
    neighbours = {}
    for p in pts:
        fids = pts[p]
        if len(fids) > 1:
            for i in range(len(fids)):
                if neighbours.has_key(fids[i]) == False:
                    neighbours[fids[i]] = []
                for j in range(len(fids)):
                    if fids[i] != fids[j]:
                        if fids[j] not in neighbours[fids[i]]:
                            neighbours[fids[i]].append(fids[j])
    neighbours_txt = []
    for n in sorted(neighbours.iterkeys()):
        neighbours_txt.append(n + "\t" + ','.join(neighbours[n]))
    return neighbours_txt




def get_points(geom):
    pts = []
    if geom.GetGeometryCount() > 0:
        for i in range(geom.GetGeometryCount()):
            g = geom.GetGeometryRef(i)
            pts.extend(get_points(g))
    else:
        for i in range(geom.GetPointCount()):
            pts.append((geom.GetX(i), geom.GetY(i)))
    return pts


if __name__ == "__main__":
    parser = OptionParser(usage=USAGE)
    parser.add_option(
        '-i', '--input', dest="input",
        help="OGR compatible input polygon vector layer",
        metavar="INPUT_LAYER"
    )
    parser.add_option(
        '-o', '--output', dest="output",
        help="Path to output matrix file",
        metavar="OUTPUT_MATRIX"
    )
    parser.add_option(
        '-a', '--attr', dest="attr",
        help="Attribute name containing a unique feature ID",
        metavar="ATTR"
    )
    parser.add_option(
        '-s', '--snapdist', dest="snapdist",
        help="Snapping distance for points. 0 by default.",
        default=0
    )
    parser.add_option(
        '-l', '--list', dest="l", action="store_true",
        help="Should we output a list or pairs of neighbours?",
        default=False
    )
    (options, args) = parser.parse_args()
    if options.snapdist == None:
        options.snapdist = 0
    if options.input == None or options.output == None or options.attr == None:
        print USAGE
        sys.exit(0)
    print("v.poly.neighbours starting...")
    poly_neighbours(options.input, options.output, options.attr, float(options.snapdist), options.l)
    print("v.poly.neighbours done")
