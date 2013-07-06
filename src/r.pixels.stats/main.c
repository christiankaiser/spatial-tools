/*
 
 r.pixels.stats
 Computes pixel-by-pixel statistics for several raster files.
 The statistics can be: sum, average, minimum, maximum, variance.
 Computes statistics for different clusters. The clusters are defined in a
 raster file where the value corresponds to the cluster ID. The statistics
 are computed for the statistic raster file according to the spatial clusters
 defined in the cluster raster file. The statistics include the sum, number of
 cells, average, minimum and maximum.
 
 Version:	1.0
 Date:		24.11.2009
 Author:	Christian Kaiser, christian.kaiser@unil.ch
 
 Copyright (C) 2009 Christian Kaiser.
 
 r.clusters.stats is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 2, or (at your option) any
 later version.
 
 r.clusters.stats is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with r.clusters.stats; see the file COPYING.  If not, write to the Free
 Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301, USA.
 
 */



#include <stdio.h>

int main (int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    return 0;
}
