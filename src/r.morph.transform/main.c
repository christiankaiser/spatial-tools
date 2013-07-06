/*
 
 r.morph.transform 
 Transform an input layer according to the provided morph file.
 The input layer can be any GDAL or OGR compatible layer.
 
 Version:	1.1.2
 Date:		17.1.2011
 Author:	Christian Kaiser, chri.kais@gmail.com
 
 Copyright (C) 2011 Christian Kaiser.
 
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "transform.h"


static char *usage[] = {
	"\n",
	"r.morph.transform\n",
	"   PURPOSE\n",
	"      Transforms a GDAL or OGR compatible input layer according to the\n",
	"      provided morph file. The output format is going to be the same as\n",
	"      input format, except if otherwise stated (it is of course not\n",
	"      possible to choose a raster output format for a vector input format,\n",
	"      and vice versa.\n",
	"   SYNOPSIS\n",
	"      r.morph.transform [--help]\n",
	"         --input INPUT_DATASOURCE [--input_layer INPUT_LAYER]\n",
	"         --morphfile MORPH_FILE\n",
	"         --output OUTPUT_DATASOURCE [--output_layer OUTPUT_LAYER]\n",
	"         [--raster] [--format FORMAT]\n",
	"   DESCRIPTION\n",
	"      The following options are available:\n",
	"         --help            Shows this usage note.\n",
	"         --input           Path to a GDAL or OGR compatible input layer.\n",
	"         --input_layer     If the input layer is a vector file, it might have\n",
	"                           more than one layer. In this case, this options allows\n",
	"                           to specify the input layer name.\n",
	"         --spatial_filter  Limits the input layer to a given region.\n",
	"                           Should be defined as comma-separated list without\n",
	"                           spaces: xmin,ymin,xmax,ymax\n",
	"         --attr_filter     Applies a filter on the attribute data of the input\n",
	"                           layer. Should be a string in the format of an SQL WHERE\n",
	"                           clause, such as 'pop > 10000 AND pop <= 500000'.\n",
	"         --morphfile       Path to a morph file.\n",
	"         --output          Path to the output file.\n",
	"         --output_layer    An optional layer name (depending on the chosen\n",
	"                           output format. Default is 'transformed_layer'.\n",
	"         --attrs           A list of comma-separated attributes in the output\n",
	"                           vector layer. If not given, all attributes will be\n",
	"                           kept, if empty, none.\n",
	"         --raster          Working in raster mode instead of vector mode.\n",
	"         --back            Make a back transform instead of forward transform.\n",
	"         --format          The format of the output layer.\n",
	"                           By default, it is the same as the input layer.\n",
	"                           Alternatively, it can be one of the following for\n",
	"                           vector files: BNA, DXF, CSV, ESRI Shapefile, GeoJSON,\n",
	"                           Geoconcept, GeoRSS, GML, GMT, GPSBabel, GPX, \n",
	"                           GPSTrackMaker, IDB*, INTERLIS*, INGRES*, KML, LIBKML*,\n",
	"                           MapInfo File, DGN, OCI* (Oracle Spatial), \n",
	"                           MSSQLSpatial*, PostgreSQL SQL dump,\n",
	"                           PostgreSQL/PostGIS*, SQLite*. Formats denoted with *\n",
	"                           need a special library and is depending of the \n",
	"                           installed OGR library.\n",
	"                           For raster layer, the following values are possible:\n",
	"                           AAIGrid, ADRG, BLX, BMP, BT, DTED, ECW*, EHdr, ELAS,\n",
	"                           ENVI, EPSILON*, ERS, FIT, FITS*, GEORASTER*, GIF, \n",
	"                           GMT*, GSAG, GSBG, GSC, GTiff, GTX, HDF4*, HF2, HFA \n",
	"                           (Erdas Imagine), IDA, ILWIS, INGR, JPEG, JPEGLS*, \n",
	"                           JPEG2000*, JP2ECW*, JP2KAK*, JP2MrSID*, JP2OpenJPEG*,\n",
	"                           MFF, MFF*, NITF, netCDF*, NTv2, PAux, PCIDSK, \n",
	"                           PCRaster, PNG, PNM, R, Rasterlite*, RMF, RST (Idrisi)\n",
	"                           SAGA, SGI, SRTMHGT, TERRAGEN, TSX, USGSDEM, VRT, XPM,\n",
	"                           XYZ (ASCII Gridded XYZ).\n",
	"   BUGS\n",
	"      Please send any comments or bug reports to chri.kais@gmail.com.\n",
	"   VERSION\n",
	"      1.1.2 (17.01.2011)\n",
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
	
	char *infile, *morphfile, *outfile, *format, *input_layer, *output_layer;
	char *attrs, *spatial_filter, *attr_filter;
	int raster, back_transform;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	if (argc < 2) {
		printUsage();
		return(0);
	}
	
	
	// Provide default values.
	infile = morphfile = outfile = input_layer = NULL;
	attrs = spatial_filter = attr_filter = NULL;
	format = NULL;
	raster = 0;
	output_layer = "transformed_layer";
	back_transform = 0;
	
	
	// Process command line
	while (1) {
		static struct option long_options[] =
		{
			{"help",			no_argument,		0,	'h'},
			{"input",			required_argument,	0,	'i'},
			{"morphfile",		required_argument,	0,	'm'},
			{"output",			required_argument,	0,	'o'},
			{"format",			required_argument,	0,	'f'},
			{"raster",			no_argument,		0,	'r'},
			{"input_layer",		required_argument,	0,	'1'},
			{"spatial_filter",	required_argument,	0,  's'},
			{"attr_filter",		required_argument,	0,	'w'},
			{"output_layer",	required_argument,  0,  '2'},
			{"attrs",			required_argument,	0,	'a'},
			{"back",			no_argument,		0,	'b'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long(argc, (char**)argv, "hi:m:o:f:r1:s:w:a:2:b", long_options, NULL);
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
			case 'm':
				morphfile = optarg;
				break;
			case 'f':
				format = optarg;
				break;
			case '1':
				input_layer = optarg;
				break;
			case 's':
				spatial_filter = optarg;
				break;
			case 'a':
				attrs = optarg;
				break;
			case 'w':
				attr_filter = optarg;
				break;
			case '2':
				output_layer = optarg;
				break;
			case 'r':
				raster = 1;
				break;
			case 'b':
				back_transform = 1;
				break;
			case '?':
				return 1;
			default:
				printUsage();
				abort();
		}
	}
	
	
	// Input value checking.
	if (morphfile == NULL) {
		fprintf(stderr, "Error. You must supply a morph file.\n");
		exit(1);
	}
	if ((infile == NULL && outfile != NULL) || (infile != NULL && outfile == NULL)) {
		fprintf(stderr, "Error. Specify an input and an output file.\n");
		exit(1);
	}
	
	
	
	// Call the appropriate function for transforming the input layer.
	
	// If input and output files are not given, read the coordinates from standard input and
	// write the transformed values to the standard output.
	if (infile == NULL && outfile == NULL) {
		ok = coord_transform(morphfile, back_transform);
		return ok;
	}
	
	printf("r.morph.transform starting\n");
	
	// We have input and output files. Choose between raster and vector.
	if (raster) {
		ok = raster_transform(infile, morphfile, outfile, format, back_transform);
	} else {
		ok = vector_transform(infile, input_layer, spatial_filter, attr_filter, 
							  morphfile, 
							  outfile, attrs, format, output_layer, 
							  back_transform);
	}

	printf("r.morph.transform done\n");
	
	return ok;
}




void printUsage () {
	int index = 0;
	while (usage[index] != NULL) {
		printf(usage[index]);
		index++;
	}
}



