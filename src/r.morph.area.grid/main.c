/*
 
 r.morph.area.grid
 
 Creates a raster grid where the pixel values contain the area of each
 morphing grid cell.
 
 Syntax:
 r.morph.area.grid --input MORPH_FILE --output OUTPUT_RASTER
 [--format FORMAT]
 
 Version:	1.0.0
 Date:		13.2.2011
 Author:	Christian Kaiser, chri.kais@gmail.com
 
 Copyright (C) 2011 Christian Kaiser.
 
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <GDAL/gdal.h>

#include "area_grid.h"


static char *usage[] = {
	"\n",
	"r.morph.area.grid\n",
	"   PURPOSE\n",
	"      Creates a raster grid where the pixel values contain the area of each\n",
	"      morphing grid cell.\n",
	"   SYNOPSIS\n",
	"      r.morph.area.grid [--help] --input MORPH_FILE --output OUTPUT_RASTER\n",
	"         [--format FORMAT]\n",
	"   DESCRIPTION\n",
	"      The following options are available:\n",
	"         --help       Shows this usage note.\n",
	"         --input      Path to a morph file such as the one generated\n",
	"                      by r.morph.equalize.density.\n",
	"         --output     A GDAL compatible output raster layer.\n",
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
	"   BUGS\n",
	"      Please send any comments or bug reports to chri.kais@gmail.com.\n",
	"   VERSION\n",
	"      1.0.0 (13.02.2011)\n",
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
	
	char *infile, *outfile, *format;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	if (argc < 4) {
		printUsage();
		return(0);
	}
	
	
	// Provide default values.
	infile = outfile = NULL;
	format = "HFA";
	
	
	// Process command line
	while (1) {
		static struct option long_options[] =
		{
			{"help",			no_argument,		0,	'h'},
			{"input",			required_argument,	0,	'i'},
			{"output",			required_argument,	0,	'o'},
			{"format",			required_argument,	0,	'f'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long(argc, (char**)argv, "hi:o:f:", long_options, NULL);
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
			case 'f':
				format = optarg;
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
	
	
	printf("r.morph.area.grid starting\n");
	ok = area_grid(infile, outfile, format);
	printf("r.morph.area.grid done\n");
	
	return ok;
}




void printUsage () {
	int index = 0;
	while (usage[index] != NULL) {
		printf(usage[index]);
		index++;
	}
}



