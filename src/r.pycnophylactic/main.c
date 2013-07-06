/*
 
 r.pycnophylactic
 Computes a pycnophylactic interpolation for a raster image.
 
 Version:	0.1
 Date:		10.12.2010
 Author:	Christian Kaiser, chri.kais@gmail.com
 
 Copyright (C) 2010 Christian Kaiser.
 
 r.pycnophylactic is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 2, or (at your option) any
 later version.
 
 r.pycnophylactic is distributed in the hope that it will be useful, but WITHOUT
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

#include "pycnophylactic.h"





static char *usage[] = {
	"\nr.pycnophylactic -- computes a pycnophylactic interpolation for a raster image.\n",
	"SYNOPSIS\n",
	"   r.pycnophylactic \n",
	"      feature_raster, value_raster output_raster\n",
	"DESCRIPTION\n",
	"   The following options are available:\n",
	"   -d\n",
	"      Compute the density. In this case, the values are divided by the number of pixels.\n",	
	"   -i\n",
	"      Maximum number of iterations.\n",
	"   feature_raster     Raster dataset containing the features from 1 to N, and 0 for not \n",
	"                      being in any feature\n",
	"   value_raster       Raster dataset containing the population values. It must have same \n",
	"                      extent and resolution as the feature_raster.\n",
	"   output_raster      The output raster dataset.\n\n",
	"BUGS\n",
	"   Please send any comments or bug reports to chri.kais@gmail.com.\n",
	"VERSION\n",
	"   0.0.1 (10.12.1020)\n",
	"AUTHOR\n"
	"   Christian Kaiser, National Centre of Geocomputation, \n",
	"   National University of Ireland Maynooth\n\n",
	NULL};





int main (int argc, const char *argv[]) {
	
	char *frast;			// Feature raster.
	char *vrast;			// Value raster.
	char *orast;			// Output raster.
	char c;
	int index;
	int computeDensity;		// Should we compute the density or not?
	int maxiters;
	
	extern int optind;
	extern int optopt;
	extern char *optarg;
	
	// Initialize variables with default values.
	frast = NULL;
	vrast = NULL;
	orast = NULL;
	computeDensity = 0;
	maxiters = 1000;
	
	
	// Process command line
	while ((c = getopt(argc, (char**)argv, "dhi:")) != -1) {
		switch (c) {
			case 'h':
				index = 0;
				while (usage[index] != NULL) {
					printf(usage[index]);
					index++;
				}
				return 0;
				
			case 'd':
				computeDensity = 1;
				break;
				
			case 'i':
				maxiters = (int)atol(optarg);
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
		if (frast == NULL)
			frast = (char*)argv[index];
		else if (vrast == NULL)
			vrast = (char*)argv[index];
		else
			orast = (char*)argv[index];
	}
	
	if (frast == NULL || vrast == NULL || orast == NULL)
	{
		fprintf(stderr, "You must provide a raster for the features and the values, and an output raster.\n");
		return 1;
	}
	
	int ok = pycnophylactic(frast, vrast, orast, computeDensity, maxiters);
    return ok;
}
