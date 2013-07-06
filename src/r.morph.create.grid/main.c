/*
 
 r.morph.create.grid
 
 Creates a vector grid for visualising the morphing grid.
 
 Syntax:
 r.morph.create.grid --input MORPH_FILE --output OUTPUT_VECTOR 
 --gridsize GRID_SIZE [--format FORMAT] [--layer LAYER_NAME]
 
 Version:	1.0.0
 Date:		13.1.2011
 Author:	Christian Kaiser, chri.kais@gmail.com
 
 Copyright (C) 2011 Christian Kaiser.
 
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <GDAL/gdal.h>

#include "create_grid.h"


static char *usage[] = {
	"\n",
	"r.morph.create.grid\n",
	"   PURPOSE\n",
	"      Creates a OGR-compatible vector layer with the morphing grid as \n",
	"   SYNOPSIS\n",
	"      r.morph.create.grid [--help] --input MORPH_FILE --output OUTPUT_VECTOR\n",
	"         --gridsize GRID_SIZE [--format FORMAT] [--layer LAYER_NAME]\n",
	"   DESCRIPTION\n",
	"      The following options are available:\n",
	"         --help       Shows this usage note.\n",
	"         --input      Path to a morph file such as the one generated\n",
	"                      by r.morph.equalize.density.\n",
	"         --output     A OGR compatible output vector layer.\n",
	"         --gridsize   The size of the output grid (on its longer side).\n",
	"         --format     The format of the output vector grid.\n",
	"                      Can be one of the following: BNA, DXF, CSV,\n",
	"                      ESRI Shapefile, GeoJSON, Geoconcept, GeoRSS, GML,\n",
	"                      GMT, GPSBabel, GPX, GPSTrackMaker, IDB*, INTERLIS*,\n",
	"                      INGRES*, KML, LIBKML*, MapInfo File, DGN, OCI*\n",
	"                      (Oracle Spatial), MSSQLSpatial*, PostgreSQL SQL dump,\n",
	"                      PostgreSQL/PostGIS*, SQLite*. Format denoted with *\n",
	"                      need a special library and is depending of the \n",
	"                      installed OGR library.\n",
	"                      Default is 'ESRI Shapefile'.\n",
	"         --layer      An optional layer name (depending on the chosen\n",
	"                      output format. Default is 'morph_grid'.\n",
	"   BUGS\n",
	"      Please send any comments or bug reports to chri.kais@gmail.com.\n",
	"   VERSION\n",
	"      1.0.0 (13.01.2011)\n",
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
	
	char *infile, *outfile, *format, *layer;
	int gridsize;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	if (argc < 4) {
		printUsage();
		return(0);
	}
	
	
	// Provide default values.
	infile = outfile = NULL;
	format = "ESRI Shapefile";
	layer = "morph_grid";
	gridsize = 0;
	
	
	// Process command line
	while (1) {
		static struct option long_options[] =
		{
			{"help",			no_argument,		0,	'h'},
			{"input",			required_argument,	0,	'i'},
			{"output",			required_argument,	0,	'o'},
			{"gridsize",		required_argument,	0,	'g'},
			{"format",			required_argument,	0,	'f'},
			{"layer",			required_argument,	0,	'l'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long(argc, (char**)argv, "hi:o:g:f:l:", long_options, NULL);
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
			case 'g':
				gridsize = atol(optarg);
				break;
			case 'f':
				format = optarg;
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
		fprintf(stderr, "Error. You must supply an input morph grid file.\n");
		exit(1);
	}
	if (outfile == NULL) {
		fprintf(stderr, "Error. You must supply a path to an output vector file.\n");
		exit(1);
	}
	if (gridsize <= 0) {
		fprintf(stderr, "Error. You must supply a grid size.\n");
		exit(1);
	}
	
	
	printf("r.morph.create.grid starting\n");
	ok = create_grid(infile, outfile, gridsize, format, layer);
	printf("r.morph.create.grid done\n");
	
	return ok;
}




void printUsage () {
	int index = 0;
	while (usage[index] != NULL) {
		printf(usage[index]);
		index++;
	}
}



