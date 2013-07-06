/*
 
 r.comparison.plot
 Creates a text file with two columns, the first containing the values of 
 raster 1, and the second the values of raster 2. Only values different from 
 NULL are considered. This file can be used for creating a comparison plot 
 between two different raster files.
 
 Version:	1.0
 Date:		22.10.2009
 Author:	Christian Kaiser, christian.kaiser@unil.ch
 
 Copyright (C) 2009 Christian Kaiser.
 
 r.comparison.plot is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 2, or (at your option) any
 later version.
 
 r.comparison.plot is distributed in the hope that it will be useful, but 
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with r.comparison.plot; see the file COPYING.  If not, write to the Free
 Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301, USA.
 
 */



#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <float.h>


#include <GDAL/gdal.h>
#include "comparison_plot.h"




static char *usage[] = {
"\nr.comparison.plot -- creates a text file with two columns, the first \n",
"                       containing the values of raster 1, and the second the\n",
"                       values of raster 2. Only values different from NULL are\n",
"                       considered. This file can be used for creating a \n",
"                       comparison plot between two different raster files.\n\n",
"SYNOPSIS\n",
"   r.comparison.plot \n",
"      [--help]\n",
"      --raster1 raster1 --raster2 raster2 [--band1 band1] [--band2 band2]\n",
"      [--null1 nullValue1] [--null2 nullValue2] [--output output_text_path]\n\n",
"DESCRIPTION\n",
"   The following options are available:\n\n",
"   -h\n",
"   --help\n",
"      Shows this usage note.\n\n",
"   --raster1 raster1\n",
"      The path to the raster 1 file used for the comparison plot.\n\n",
"   --raster2 raster2\n",
"      The path to the raster 2 file used for the comparison plot.\n\n",
"   --band1 band1\n",
"      The band of raster 1 to use. Default value is 1.\n\n",
"   --band2 band2\n",
"      The band of raster 2 to use. Default value is 1.\n\n",
"   --null1 nullValue1\n"
"      The NULL value for the raster 1. If both rasters have a NULL value at a\n",
"      given location, the values are not printed out. Default value is \n",
"      -DBL_MAX.\n\n",
"   --null2 nullValue2\n"
"      The NULL value for the raster 2. If both rasters have a NULL value at a\n",
"      given location, the values are not printed out. Default value is \n",
"      -DBL_MAX.\n\n",
"   -o output_text_path\n",
"   --output output_text_path\n",
"      The path to the output text file. If you don't provide an output file,\n",
"      the result is written to the standard output.\n\n",
"BUGS\n",
"   Please send any comments or bug reports to christian@361degres.ch.\n\n",
"VERSION\n",
"   1.0.0 (22.10.2009)\n\n",
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
	
	char *raster1;					// Path to the input raster 1 file.
	char *raster2;					// Path to the input raster 2 file.
	int band1;						// Input raster 1 band.
	int band2;						// Input raster 2 band.
	char *output_file;				// Path to the output text file.
	double null1, null2;			// NULL values for raster 1 and 2.
	
	int ok;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	
	// Initialize variables with default values.
	raster1 = NULL;
	raster2 = NULL;
	band1 = 1;
	band2 = 1;
	output_file = NULL;	
	null1 = -DBL_MAX;
	null2 = -DBL_MAX;
	
	
	
	// Process command line
	while (1)
	{
		
		static struct option long_options[] =
		{
			{"help",				no_argument,		0,	'h'},
			{"raster1",				required_argument,	0,	'u'},
			{"raster2",				required_argument,	0,	'v'},
			{"band1",				required_argument,	0,	'b'},
			{"band2",				required_argument,	0,  'c'},
			{"output",				required_argument,	0,	'o'},
			{"null1",				required_argument,  0,  'm'},
			{"null2",				required_argument,	0,	'n'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long(argc, (char**)argv, "hu:v:b:c:o:m:n:", long_options, NULL);
		
		// Detect the end of the options.
		if (c == -1)
			break;
		
		
		switch (c) {
				
			case 'h':
				index = 0;
				while (usage[index] != NULL) {
					printf(usage[index]);
					index++;
				}
				return 0;
				
			case 'u':
				raster1 = optarg;
				break;
				
			case 'v':
				raster2 = optarg;
				break;
				
			case 'b':
				band1 = atoi(optarg);
				break;
				
			case 'o':
				output_file = optarg;
				break;
			
			case 'm':
				null1 = atof(optarg);
				break;
			
			case 'n':
				null2 = atof(optarg);
				break;

			case '?':
				return 1;
				
			default:
				abort();
		}
	}
	
	
	
	
	
	if (raster1 == NULL)
	{
		fprintf(stderr, "Error. You must provide an input raster 1.\n");
		return 1;
	}
	
	if (raster2 == NULL)
	{
		fprintf(stderr, "Error. You must provide an input raster 2.\n");
		return 1;
	}
	
	
	GDALAllRegister();
	
	
	
	ok = raster_comparison_plot(raster1, band1, null1, raster2, band2, null2, output_file);

	
	fprintf(stdout, "r.comparison.plot done.\n");
	
	return ok;
}








