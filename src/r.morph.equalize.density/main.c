/*
 
 r.morph.equalize.density
 
 Code for the density equalization algorithm of Gastner & Newmann.
 
 Syntax:
 r.morph.equalize.density --input inraster --output morphfile 
 [--slower] [--less_accurate]
 
 Author:	Christian Kaiser, chri.kais@gmail.com
 
 Copyright (C) 2011 Christian Kaiser.
 
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <GDAL/gdal.h>

#include "equalize_density.h"


static char *usage[] = {
	"\n",
	"r.morph.equalize.density\n",
	"   PURPOSE\n",
	"      Applies the density equalization algorithm on a raster file for.\n",
	"      creating an area cartogram. The resulting output is a morph file\n",
	"      that can be used for transforming vector and raster files into the\n",
	"      new cartogram space.\n",
	"   SYNOPSIS\n",
	"      r.morph.equalize.density [--help] --input input_raster\n",
	"         --output output_morph_file\n",
	"         [--slower] [--less_accurate]\n",
	"   DESCRIPTION\n",
	"      The following options are available:\n",
	"         --help           Shows this usage note.\n",
	"         --input          Path to a GDAL compatible raster image.\n",
	"         --output         Path to the output morph file image.\n",
	"         --slower         Allows to adjust the speed and consequently the\n",
	"                          memory requirement. Memory usage is approximately\n",
	"                          50%.\n",
	"         --less_accurate  Allows to save more memory by applying a \n",
	"                          slightly less accurate algorithm. The memory \n",
	"                          requirement is about 40% less with the fast\n",
	"                          algorithm, and 70% less with the slow\n",
	"                          algorithm.\n",
	"   BUGS\n",
	"      Please send any comments or bug reports to chri.kais@gmail.com.\n",
	"   VERSION\n",
	"      1.0.1 (18.01.2011)\n",
	"   AUTHOR\n"
	"      Christian Kaiser, National Centre for Geocomputation,\n",
	"      National University of Ireland Maynooth\n",
	"      <chri.kais@gmail.com>\n",
	"   ACKNOWLEDGEMENTS\n",
	"      The algorithm used has been created by Michael Gastner and Mark \n",
	"      Newman, and is described in their paper 'Diffusion-based method for\n",
	"      producing density equalizing maps' (Proc. Natl. Acad. Sci. USA 101,\n",
	"      7499-7504, 2004). See http://www-personal.umich.edu/~mejn/cart/ for\n",
	"      more details.\n",
	"\n",
	NULL};


void printUsage();


int main (int argc, const char *argv[]) {
	
	int c;
	int ok;
	
	char *infile, *outfile;
	int fast, accurate;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	if (argc < 4) {
		printUsage();
		return(0);
	}
	
	
	// Provide default values.
	infile = outfile = NULL;
	fast = accurate = 1;
	
	
	// Process command line
	while (1) {
		static struct option long_options[] =
		{
			{"help",			no_argument,		0,	'h'},
			{"input",			required_argument,	0,	'i'},
			{"output",			required_argument,	0,	'o'},
			{"slower",			no_argument,		0,	's'},
			{"less_accurate",	no_argument,		0,	'l'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long(argc, (char**)argv, "hi:o:sl", long_options, NULL);
		if (c == -1) {
			break;
		}
		
		switch (c) {
			case 'h':
				printUsage();
				return 0;
			case 'i':
				infile = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 's':
				fast = 0;
				break;
			case 'l':
				accurate = 0;
				break;
			case '?':
				return 1;
			default:
				printUsage();
				abort();
		}
	}
	
	
	// Input value checking.
	if (infile == NULL) {
		fprintf(stderr, "Error. You must supply an input raster file.\n");
		exit(1);
	}
	if (outfile == NULL) {
		fprintf(stderr, "Error. You must supply a path to an output morph file.\n");
		exit(1);
	}
	
	
	printf("r.morph.equalize.density starting\n");
	ok = equalize_density(infile, outfile, fast, accurate);
	printf("r.morph.equalize.density done\n");
	
	return ok;
}




void printUsage () {
	int index = 0;
	while (usage[index] != NULL) {
		printf(usage[index]);
		index++;
	}
}



