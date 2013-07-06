/*
 
 r.groupcells
 Groups several cells of a raster together. This is a special case
 of resampling, or a sort of spatial group by.
 
 Version:	1.0
 Date:		21.5.2009
 Author:	Christian Kaiser, christian@361degres.ch
 
 Copyright (C) 2009 Christian Kaiser.
 
 r.groupcells is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 2, or (at your option) any
 later version.
 
 r.groupcells is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with Octave; see the file COPYING.  If not, write to the Free
 Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301, USA.
 
 */



#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "groupcells.h"




static char *usage[] = {
"\n",
"r.groupcells -- groups several cells of a raster together; this is a special case of\n",
"                resampling, or a sort of spatial group by.\n\n",
"SYNOPSIS\n",
"   r.groupcells -r value [-f format] [-s statistic] input_raster output_raster\n\n",
"DESCRIPTION\n",
"   The following options are available:\n\n",
"   -r value      The number of cells to group into one cell.\n\n",
"   -s statistic  Statistic to use for grouping the cells together. Following options are\n",
"                 available: sum, min, max, mean. Default is mean.\n\n",
"   -f format     Format for the output raster file. Default is GTiff.\n",
"                 The following formats are supported:\n",
"                 GTiff    : GeoTIFF                     |  HFA      : Erdas Imagine (.img)\n",
"                 AAIGrid  : Arc/Info ASCII Grid         |  PNG      : Portable Network Graphics\n",
"                 JPEG     : JPEG image                  |  GIF      : Graphics Interchange Format\n",
"                 PCIDSK   : PCIDSK Database File        |  PCRaster : PCRaster Raster File\n",
"                 GMT      : GMT NetCDF Grid Format      |  JPEG2000 : JPEG-2000\n",
"                 RST      : Idrisi Raster A.1           |  ENVI     : ENVI .hdr Labelled\n",
"\n",
"BUGS\n",
"   Please send any comments or bug reports to christian@361degres.ch.\n\n",
"VERSION\n",
"   1.0.0 (21.5.2009)\n\n",
"AUTHOR\n"
"   Christian Kaiser, University of Lausanne <christian@361degres.ch>\n\n",
"ACKNOWLEDGEMENTS\n",
"   This work has been supported by the Swiss National Foundation, through the projects \"Urbanization\n",
"   Regime and Environmental Impact: Analysis and Modelling of Urban Patters, Clustering and\n",
"   Methamorphoses\" (no 100012-113506, see www.clusterville.org for more information) and \n",
"   \"GeoKernels\": Kernel-Based Methods for Geo- and Environmental Sciences, phase 2 \n",
"   (no 200020-121835, see www.geokernels.org for more information).\n\n",
NULL};





int main (int argc, const char *argv[])
{
	int c;
	int index;
	int r;				// Number of cells to group together.
	char *iraster;		// Input raster.
	char *oraster;		// Output raster.
	char *statistic;	// Statistic to use.
	char *oformat;		// Output raster format.
	char defaultFormat[] = "GTiff";
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	// Initialize variables with default values.
	iraster = NULL;
	oraster = NULL;
	statistic = NULL;
	oformat = defaultFormat;
	
	
	// Process command line
	while ((c = getopt(argc, (char**)argv, "hr:f:s:")) != -1) {
		switch (c) {
				
			case 'h':
				index = 0;
				while (usage[index] != NULL) {
					printf(usage[index]);
					index++;
				}
				return 0;
				
			case 'r':
				r = atoi(optarg);
				break;
				
			case 'f':
				oformat = optarg;
				break;
				
			case 's':
				statistic = optarg;
				break;
				
			case '?':
				if (optopt == 'b' || optopt == 'c') {
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				} else {
					fprintf(stderr, "Unknown option -%c.\n", optopt);
				}
				return 1;
				
			default:
				abort();
		}
	}
	
	for (index = optind; index < argc; index++)
	{
		if (iraster == NULL)
			iraster = (char*)argv[index];
		else
			oraster = (char*)argv[index];
	}
	
	if (iraster == NULL && oraster == NULL) {
		
		while (usage[index] != NULL) {
			printf(usage[index]);
			index++;
		}
		return 0;
	}
	
	
	int ok = groupcells(iraster, oraster, oformat, r, statistic);
	
    return ok;
}








