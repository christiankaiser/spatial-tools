/*
 
 r.add.boundary
 
 Adds a boundary to an existing GDAL compatible raster image.
 The amount of boundary to add can be specified. For the raster values
 of the added boundary, two modes are available: mean or a specified value.
 
 Syntax:
 raster_add_boundary inraster outraster boundary null [value]
 
 If no value is given, the mean value will be used instead.
 The null value will be replaced by the given value, or the mean value.
 
 Version:	1.0
 Date:		11.1.2011
 Author:	Christian Kaiser, chri.kais@gmail.com
 
 Copyright (C) 2011 Christian Kaiser.
 
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <GDAL/gdal.h>
#include <GDAL/cpl_string.h>

#include "r_change_type.h"


static char *usage[] = {
	"\n",
	"r.change.type\n",
	"   PURPOSE\n",
	"      Changes the type of a GDAL compatible raster file.\n",
	"      Depending on the new type, some data loss might result.\n",
	"   SYNOPSIS\n",
	"      r.change.type [--help] --input input_raster --output output_raster\n",
	"         --type new_type\n",
	"   DESCRIPTION\n",
	"      The following options are available:\n",
	"         --help        Shows this usage note.\n",
	"         --input       Path to a GDAL compatible raster image.\n",
	"         --output      Path to the output raster image.\n",
	"         --type        The raster type of the output raster image.\n",
	"                       Can take one of the following values:\n",
	"                       Byte 	 Eight bit unsigned integer\n",
	"                       UInt16 	 Sixteen bit unsigned integer\n",
	"                       Int16 	 Sixteen bit signed integer (short)\n",
	"                       UInt32 	 Thirty two bit unsigned integer\n",
	"                       Int32 	 Thirty two bit signed integer (int)\n",
	"                       Float32  Thirty two bit floating point (float)\n",
	"                       Float64  Sixty four bit floating point (double)\n",
	"   BUGS\n",
	"      Please send any comments or bug reports to chri.kais@gmail.com.\n",
	"   VERSION\n",
	"      1.0.0 (11.01.2011)\n",
	"   AUTHOR\n"
	"      Christian Kaiser, National Centre for Geocomputation,\n",
	"      National University of Ireland Maynooth\n",
	"      <chri.kais@gmail.com>\n\n",
	NULL};


void printUsage();


int main (int argc, const char *argv[]) {
	
	int c;
	int ok;
	
	char *infile, *outfile;
	char *new_type;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	if (argc < 4) {
		printUsage();
		return(0);
	}
	
	
	// Provide default values.
	infile = outfile = new_type = NULL;
	
	
	// Process command line
	while (1) {
		static struct option long_options[] =
		{
			{"help",			no_argument,		0,	'h'},
			{"input",			required_argument,	0,	'i'},
			{"output",			required_argument,	0,	'o'},
			{"type",			required_argument,	0,	't'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long(argc, (char**)argv, "hi:o:t:", long_options, NULL);
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
			case 't':
				new_type = optarg;
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
		fprintf(stderr, "Error. You must supply a path to an output raster file.\n");
		exit(1);
	}
	if (new_type == NULL) {
		fprintf(stderr, "Error. You must supply a new raster type.\n");
		fprintf(stderr, "Use the --help flag for more information.\n");
		exit(1);
	}
	
	
	printf("r.change.type starting\n");
	ok = r_change_type(infile, outfile, new_type);
	printf("r.change.type done\n");
	
	return ok;
}




void printUsage () {
	int index = 0;
	while (usage[index] != NULL) {
		printf(usage[index]);
		index++;
	}
}



