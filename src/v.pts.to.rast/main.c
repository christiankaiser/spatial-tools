/*
 
 v.pts.to.rast
 Converts a point vector file to a raster file 
 
 Version:	1.0
 Date:		11.1.2011
 Author:	Christian Kaiser, chri.kais@gmail.com
 
 Copyright (C) 2011 Christian Kaiser.
 
 */



#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "pts_to_rast.h"


static char *usage[] = {
	"\nv.pts.to.rast -- converts a point vector file to a raster file.\n\n",
	"SYNOPSIS\n",
	"   v.pts.to.rast \n",
	"      [--help]\n",
	"      --resolution res\n",
	"      [--xmin xmin] [--xmax xmax] [--ymin ymin] [--ymax ymax]\n",
	"      --input point_vector_file \n",
	"      [--attr ATTR] [--stat mean] [--filter filter]\n",
	"      --output output_raster\n",
	"      [--type RASTER_TYPE] [--null 0] [--format HFA]\n",
	"DESCRIPTION\n",
	"   The following options are available:\n",
	"   --help\n",
	"       Shows this usage note.\n",
	"   --attr     If provided, it takes the attribute value for each point\n",
	"              and summarizes the values according to the stat flag if more\n",
	"              than one point falls inside a pixel.\n",
	"              If not provided, the number of points inside each pixel is\n",
	"              used instead as pixel value.\n",
	"   --stat     Selects the method to summarize the attribute values.\n",
	"              Can be mean or sum, default is mean.\n",
	"   --type     Raster type of the output file. Can be Int32, Float32 or\n",
	"              Float64. Default is Int32.\n",
	"   --filter\n",
	"       Allows filtering the input vector using an SQL-style selection\n",
	"       on the attributes of the vector features.\n",
	"\n",
	"BUGS\n",
	"   Please send any comments or bug reports to chri.kais@gmail.com.\n",
	"VERSION\n",
	"   1.1.0 (23.01.2011)\n",
	"AUTHOR\n"
	"   Christian Kaiser, National Centre for Geocomputation, National University of Irelan Maynooth\n",
	"   <chri.kais@gmail.com>\n\n",
	NULL};



void printUsage();


int main (int argc, const char *argv[]) {
	
	int c;
	
	double resolution;					// The pixel size
	double xmin, xmax, ymin, ymax;		// The spatial extent of the raster file.
										// If not given, the vector file extent will be taken instead.
	int nullValue;						// The NULL value
	char *input_file;					// Path to the input vector file
	char *input_layer;					// Name of the input layer; might be NULL.
	char *attr, *stat;					// Attribute to take the value, and method for summarizing the values.
	char *output_file;					// Path to the output raster file
	char *format;						// Output image file format;
	char *filter;						// Attribute filter.
	char *raster_type;					// Output raster type.
	
	int ok;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	if (argc < 6) {
		printUsage();
		exit(0);
	}
	
	
	// Initialize variables with default values.
	resolution = 0.0f;
	xmin = xmax = ymin = ymax = 0.0f;
	nullValue = 0;
	input_file = input_layer = NULL;
	attr = NULL;
	stat = "mean";
	output_file = NULL;
	format = "HFA";
	filter = NULL;
	raster_type = "Int32";
	
	
	// Process command line
	while (1) {
		
		static struct option long_options[] =
		{
			{"help",				no_argument,		0,	'h'},
			{"resolution",			required_argument,	0,	'r'},
			{"xmin",				required_argument,	0,	'1'},
			{"xmax",				required_argument,	0,	'2'},
			{"ymin",				required_argument,	0,  '3'},
			{"ymax",				required_argument,	0,	'4'},
			{"input",				required_argument,	0,	'i'},
			{"attr",				required_argument,	0,  'a'},
			{"stat",				required_argument,	0,	's'},
			{"layer",				required_argument,	0,	'l'},
			{"output",				required_argument,  0,	'o'},
			{"type",				required_argument,	0,	't'},
			{"null",				required_argument,  0,  'n'},
			{"format",				required_argument,  0,  'f'},
			{"filter",				required_argument,  0,  'w'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long(argc, (char**)argv, "hr:1:2:3:4:i:a:s:l:o:n:f:w:", long_options, NULL);
		
		// Detect the end of the options.
		if (c == -1)
			break;
		
		switch (c) {
			case 'h':
				printUsage();
				return 0;
			case 'r':
				resolution = atof(optarg);
				break;
			case '1':
				xmin = atof(optarg);
				break;
			case '2':
				xmax = atof(optarg);
				break;
			case '3':
				ymin = atof(optarg);
				break;
			case '4':
				ymax = atof(optarg);
				break;			
			case 'i':
				input_file = optarg;
				break;
			case 'a':
				attr = optarg;
				break;
			case 's':
				stat = optarg;
				break;
			case 'l':
				input_layer = optarg;
				break;
			case 'o':
				output_file = optarg;
				break;
			case 't':
				raster_type = optarg;
				break;
			case 'n':
				nullValue = atoi(optarg);
				break;
			case 'f':
				format = optarg;
				break;
			case 'w':
				filter = optarg;
				break;
			case '?':
				return 1;
			default:
				abort();
		}
	}
	
	
	
	
	
	if (input_file == NULL) {
		fprintf(stderr, "Error. You must provide at an input vector file.\n");
		return 1;
	}
	
	if (output_file == NULL) {
		fprintf(stderr, "Error. You must provide at an output raster file.\n");
		return 1;
	}
	
	if (resolution <= 0.0f) {
		fprintf(stderr, "Error. You must provide at an output raster resolution.\n");
		return 1;
	}
	
	
	ok = pts_to_rast(input_file, input_layer, attr, stat, 
					 output_file, 
					 resolution, xmin, xmax, ymin, ymax, 
					 nullValue, format, raster_type, filter);
	
	fprintf(stdout, "v.pts.to.rast done.\n");
	
	return ok;
}



void printUsage () {
	int index = 0;
	while (usage[index] != NULL) {
		printf(usage[index]);
		index++;
	}
}





