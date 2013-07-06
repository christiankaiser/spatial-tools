/*
 
 r.percolation
 Computes the spatial clusters in a raster map, based on percolation.
 
 Version:	1.0
 Date:		18.5.2009
 Author:	Christian Kaiser, christian.kaiser@unil.ch
 
 Copyright (C) 2009 Christian Kaiser.
 
 r.percolation is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 3, or (at your option) any
 later version.
 
 r.percolation is distributed in the hope that it will be useful, but WITHOUT
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

#include "percolation.h"




static char *usage[] = {
"\nr.percolation -- computes the spatial clusters in a raster map based on percolation\n\n",
"SYNOPSIS\n",
"   r.percolation \n",
"      [-x] [-s stat] [-b value] [-m value] [-f format] \n",
"      input_raster output_raster [output_statistics]\n\n",
"DESCRIPTION\n",
"   The following options are available:\n\n",
"   -x                Use extended neighborhood\n",
"                     (next nearest neighbors instead of nearest neighbors)\n\n",
"   -s stat           Use the statistical value in the output raster rather than\n",
"                     the cluster number. The following options are available:\n",
"                     id (default), mean, sum, min, max.\n",
"                     Note that id is a 32 bits integer, while the other\n",
"                     statistics are double float (64 bits). Not all output \n",
"                     formats may support these data types.\n\n",
"   -b band           The raster band to be considered. First band has number\n",
"                     1. Default is 1.\n\n",
"   -m value          Bias value. This is the minimum raster value which is \n",
"                     considered as being part of a cluster. Default is 0.\n\n",
"   -c value          Cluster value. This is the minimum value (sum) for a\n",
"                     cluster for being retained. Default is 0.\n\n",
"   -f format         Format for the output raster file. Default is HFA as it\n",
"                     supports all needed data types.\n",
"                     The following formats are supported:\n",
"                      GTiff    : GeoTIFF\n",
"                      HFA      : Erdas Imagine (.img)\n",
"                      AAIGrid  : Arc/Info ASCII Grid\n",
"                      PNG      : Portable Network Graphics\n",
"                      JPEG     : JPEG image\n",
"                      GIF      : Graphics Interchange Format\n",
"                      PCIDSK   : PCIDSK Database File\n",
"                      PCRaster : PCRaster Raster File\n",
"                      GMT      : GMT NetCDF Grid Format\n",
"                      JPEG2000 : JPEG-2000\n",
"                      RST      : Idrisi Raster A.1\n",
"                      ENVI     : ENVI .hdr Labelled\n",
"\n",
"   input_raster       \n\n",
"   output_raster      \n\n",
"   output_statistics  Statistics for the clusters are computed and stored in a\n",
"                      text file.\n\n",
"BUGS\n",
"   Please send any comments or bug reports to christian@361degres.ch.\n\n",
"VERSION\n",
"   1.0.1 (22.5.2009)\n\n",
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
	int extneigh;			// Extended neighborhood (0|1)
	int band;				// Raster band.
	double bias;			// Bias value
	double cluval;			// Cluster value
	char *iraster;			// Input raster.
	char *oraster;			// Output raster.
	char *rasterStats;		// Output raster values: which statistic should we use?
	char *ostats;			// Output statistics.
	char *oformat;			// Output raster format.
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	// Initialize variables with default values.
	band = 1;
	extneigh = 0;
	bias = 0.0f;
	cluval = 0.0f;
	iraster = NULL;
	oraster = NULL;
	rasterStats = "id";		// Use the cluster number as default.
	ostats = NULL;
	oformat = "HFA";
	

	// Process command line
	while ((c = getopt(argc, (char**)argv, "hxs:b:f:m:c:")) != -1) {
		switch (c) {
				
			case 'h':
				index = 0;
				while (usage[index] != NULL) {
					printf(usage[index]);
					index++;
				}
				return 0;
				
			case 'x':
				extneigh = 1;
				break;
			
			case 'b':
				band = atoi(optarg);
				break;
			
			case 'f':
				oformat = optarg;
				break;
				
			case 'm':
				bias = atof(optarg);
				break;
			
			case 'c':
				cluval = atof(optarg);
				break;
			
			case 's':
				rasterStats = optarg;
				break;
			
			case '?':
				if (optopt == 'b' || optopt == 'c') {
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				} else {
					fprintf(stderr, "Unknown option -%c.\n", optopt);
				}
				fprintf(stderr, "Use the -h flag for getting help.\n\n");
				return 1;
			
			default:
				abort();
		}
	}
	
	for (index = optind; index < argc; index++)
	{
		if (iraster == NULL)
			iraster = (char*)argv[index];
		else if (oraster == NULL)
			oraster = (char*)argv[index];
		else
			ostats = (char*)argv[index];
	}
	
	if (iraster == NULL && oraster == NULL)
	{
		fprintf(stderr, "You must provide an input and output raster.\n");
		return 1;
	}

	
	int ok = percolation(iraster, oraster, rasterStats, oformat, ostats, band, extneigh, bias, cluval);
	
    return ok;
}








