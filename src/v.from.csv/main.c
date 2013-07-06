/*
 
 v.from.csv
 
 Version:	1.0.0
 Date:		20.1.2011
 Author:	Christian Kaiser, chri.kais@gmail.com
 
 Copyright (C) 2011 Christian Kaiser.
 
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "csvlib.h"
#include "convert_csv.h"


static char *usage[] = {
	"\n",
	"v.from.csv\n",
	"   PURPOSE\n",
	"      Reads a CSV file containing spatial coordinates into a point layer.\n",
	"   SYNOPSIS\n",
	"      v.from.csv [--help]\n",
	"         --input INPUT_CSV_FILE [--xcol X_COLUMN] [--ycol Y_COLUMN]\n",
	"         [--header 1] [--sep SEPARATOR] [--quote QUOTE]\n",
	"         [--format FORMAT] --output OUTPUT_FILE [--layer OUTPUT_LAYER]\n",
	"   DESCRIPTION\n",
	"      The following options are available:\n",
	"         --help         Shows this usage note.\n",
	"         --input        Path to the input CSV file.\n",
	"         --xcol         The column containing the x coordinate.\n",
	"                        It is a number between 1 and ncol. If header is 1,\n",
	"                        this can also be the name of the column.column name.\n",
	"                        If not given, the x column is assumed to be the\n",
	"                        the first column.\n",
    "         --ycol         The column containing the y coordinate.\n",
	"                        If not given, it is assumed to be the second column.\n",	
	"         --header       0 if no header is present, 1 otherwise.\n",
	"                        If not given, it is assumed to be 1.\n",
	"         --sep          The string separating two columns.\n",
	"                        A tab-character is the default value.\n",
	"         --quote        Character or string that is removed around a field.\n",
	"         --format       Output format. Default is 'ESRI Shapefile'.\n",
	"         --output       OGR-compatible output datasource. In the case of\n",
	"                        a Shapefile, this is simply the path to the folder.\n",
	"         --layer        An optional layer name. In the case of a Shapefile,\n",
	"                        this is the name of the file (without extension).\n",
	"   BUGS\n",
	"      Please send any comments or bug reports to chri.kais@gmail.com.\n",
	"   VERSION\n",
	"      1.0.1 (23.01.2011)\n",
	"   AUTHOR\n"
	"      Christian Kaiser, National Centre for Geocomputation,\n",
	"      National University of Ireland Maynooth\n",
	"      <chri.kais@gmail.com>\n",
	"\n",
	NULL};


void printUsage();


int main (int argc, const char *argv[]) {
	
	int c;
	int ok;
	
	char *infile, *xcol, *ycol, *sep, *quote, *format, *outfile, *layer;
	int xidx, yidx, header;
	
	CSVHandle csv;
	
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	if (argc < 4) {
		printUsage();
		return(0);
	}
	
	
	// Provide default values.
	infile = xcol = ycol = outfile = layer = NULL;
	sep = "\t";
	quote = "\"";
	format = "ESRI Shapefile";
	xidx = 1;
	yidx = 2;
	header = 1;
	
	
	// Process command line
	while (1) {
		static struct option long_options[] =
		{
			{"help",		no_argument,		0,	'h'},
			{"input",		required_argument,	0,	'i'},
			{"xcol",		required_argument,	0,	'x'},
			{"ycol",		required_argument,  0,  'y'},
			{"header",      required_argument,	0,	't'},
			{"sep",			required_argument,	0,  's'},
			{"quote",		required_argument,	0,  'q'},
			{"format",		required_argument,	0,  'f'},
			{"output",		required_argument,	0,  'o'},
			{"layer",		required_argument,	0,  'l'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long(argc, (char**)argv, "hi:x:y:t:s:q:f:o:l:", long_options, NULL);
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
			case 'x':
				xcol = optarg;
				break;
			case 'y':
				ycol = optarg;
				break;
			case 't':
				header = atol(optarg);
				break;
			case 's':
				sep = optarg;
				break;
			case 'q':
				quote = optarg;
				break;
			case 'f':
				format = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'l':
				layer = optarg;
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
		fprintf(stderr, "Error. You must provide a input CSV file.\n");
		exit(1);
	}
	if (outfile == NULL) {
		fprintf(stderr, "Error. You must provide an output file.\n");
		exit(1);
	}
	
	// Find the input column indices for the x and y columns.
	if (header == 1) {
		csv = CSVOpen(infile, "r", sep[0], 1);
		
		if (xcol == NULL) {
			xidx = 0;
		} else {
			xidx = CSVGetFieldIndex(csv, xcol);
		}
		if (xidx < 0) {
			xidx = atol(xcol) - 1;
		}
		
		if (ycol == NULL) {
			yidx = 1;
		} else {
			yidx = CSVGetFieldIndex(csv, ycol);
		}
		if (yidx < 0) {
			yidx = atol(ycol) - 1;
		}
		
		CSVClose(csv);
	} else {
		xidx = atol(xcol) - 1;
		yidx = atol(ycol) - 1;
	}
	
	// Let's go!
	ok = csv_to_ogr(infile, xidx, yidx, header, sep, quote, format, outfile, layer);
	
	return ok;
}




void printUsage () {
	int index = 0;
	while (usage[index] != NULL) {
		printf(usage[index]);
		index++;
	}
}



