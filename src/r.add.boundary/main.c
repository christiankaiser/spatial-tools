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


static char *usage[] = {
	"\n",
	"r.add.boundary\n",
	"   PURPOSE\n,"
	"      Adds a boundary to an existing raster image.\n",
	"      The amount of boundary to add can be specified.\n",
	"      Two modes are available for specifying how to compute the pixel\n",
	"      values inside the boundary to add: mean of all pixels, or a value\n",
	"      to provide.\n",
	"   SYNOPSIS\n",
	"      r.add.boundary [--help] --input input_raster --output output_raster\n",
	"         --boundary amount_of_boundary [--nullValue null] [--value value]\n",
	"   DESCRIPTION\n",
	"      The following options are available:\n",
	"         --help        Shows this usage note.\n",
	"         --input       Path to a GDAL compatible raster image.\n",
	"         --output      Path to the output raster image.\n",
	"         --boundary    Proportion of the image to add as boundary.\n",
	"                       A value of 0.5 will double the image size, as half\n",
	"                       of the image size is appended on each side.\n",
	"         --nullValue   The value to be considered as no data value.\n",
	"                       Default value: -9999\n",
	"         --value       The pixels inside the added boundary will take this\n",
	"                       value. If not specified, the mean value of all\n",
	"                       pixels will be used.\n",
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
	
	char *infile, *outfile;
	double boundary;						// Amount of boundary to add.
	int use_mean;							// 1 if we should use the mean, 0 otherwise
	double value;							// If mode=value, then this value will be filled in.
	double nullvalue;						// This value is considered as being empty.
	GDALDatasetH in_ds, out_ds;				// The input and output dataset.
	GDALRasterBandH in_band, out_band;		// The input and output raster band.
	GDALDriverH hDriver;					// Raster driver to use.
	char **papszMetadata;					// Raster dataset metadata.
	double adfGeoTransform[6];				// Raster geo transform.
	const char *projectionRef;				// Raster projection reference.
	int in_size_x, in_size_y;				// Size of input raster dataset.
	int out_size_x, out_size_y;				// Size of output raster dataset.
	int left_margin, right_margin;			// Size difference of input and output dataset.
	int upper_margin, lower_margin;
	int nBands;								// The number of bands.
	GDALDataType rasterType;				// Raster type.
	double *in_values, *out_values;			// Content of raster bands.
	double *in_values_ptr, *out_values_ptr;
	int i, cnt, x, y;
	double sum;								// Used for computing the mean value.
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	if (argc < 4) {
		printUsage();
		return(0);
	}
	
	use_mean = 1;
	
	// Process command line
	while (1) {
		
		static struct option long_options[] =
		{
			{"help",			no_argument,		0,	'h'},
			{"input",			required_argument,	0,	'i'},
			{"output",			required_argument,	0,	'o'},
			{"boundary",		required_argument,	0,	'b'},
			{"nullValue",		required_argument,	0,  'n'},
			{"value",			required_argument,	0,	'v'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, (char**)argv, "hi:o:b:n:v:", long_options, NULL);
		// Detect the end of the options.
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
			case 'b':
				boundary = atof(optarg);
				if (boundary < 0.0) {
					boundary = 0.5;
					fprintf(stderr, "Warning. Boundary value cannot be negative. Changed to 0.5.\n");
				}
				break;
			case 'n':
				nullvalue = atof(optarg);
				break;
			case 'v':
				value = atof(optarg);
				use_mean = 0;
				break;
			case '?':
				return 1;
			default:
				printUsage();
				abort();
		}
	}
	
	
	printf("r.add.boundary starting\n");
	
    GDALAllRegister();
	
	// Open the input raster file.
	in_ds = GDALOpen(infile, GA_ReadOnly);
    if (in_ds == NULL) {
		fprintf(stderr,"Error. Unable to open file `%s'\n", infile);
		exit(1);
	}

	// Get the input raster size.
	in_size_x = GDALGetRasterXSize(in_ds);
	in_size_y = GDALGetRasterYSize(in_ds);
	
	// Compute the output raster size.
	out_size_x = ((2.0 * boundary) + 1.0) * in_size_x;
	out_size_y = ((2.0 * boundary) + 1.0) * in_size_y;
	
	// Get the number of bands.
	nBands = GDALGetRasterCount(in_ds);
	
	// Get the raster type.
	in_band = GDALGetRasterBand(in_ds, 1);
	rasterType = GDALGetRasterDataType(in_band);
	
	// Determine the driver to use for creating the output file.
	hDriver = GDALGetDatasetDriver(in_ds);		// Driver of input dataset.
	papszMetadata = GDALGetMetadata(hDriver, NULL);
	if (CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE) == 0) {
		// The current driver does not support raster creation.
		// We will use HFA (ERDAS Imagine) driver instead.
		hDriver = GDALGetDriverByName("HFA");
	}
	
	// Create the output raster dataset.
	out_ds = GDALCreate(hDriver, outfile, out_size_x, out_size_y, nBands, rasterType, NULL);
	if (out_ds == NULL) {
		fprintf(stderr,"Error. Unable to create file `%s'\n", outfile);
		GDALClose(in_ds);
		exit(1);
	}
	
	// Compute the margins.
	left_margin = (out_size_x - in_size_x) / 2;
	right_margin = out_size_x - in_size_x - left_margin;
	upper_margin = (out_size_y - in_size_y) / 2;
	lower_margin = out_size_y - in_size_y - upper_margin;
	
	// Set the georeference.
	GDALGetGeoTransform(in_ds, adfGeoTransform);
	adfGeoTransform[0] -= left_margin * adfGeoTransform[1];
	adfGeoTransform[3] -= upper_margin * adfGeoTransform[5];
	GDALSetGeoTransform(out_ds, adfGeoTransform);
	
	projectionRef = GDALGetProjectionRef(in_ds);
	GDALSetProjection(out_ds, projectionRef);
	
	// Allocate the memory for holding the raster band content.
	in_values = malloc(in_size_x * in_size_y * sizeof(double));
	if (in_values == NULL) {
		fprintf(stderr,"Error. Not enough memory for reading `%s'\n", infile);
		GDALClose(in_ds);
		GDALClose(out_ds);
		exit(1);
	}
	out_values = malloc(out_size_x * out_size_y * sizeof(double));
	if (out_values == NULL) {
		fprintf(stderr,"Error. Not enough memory for writing `%s'\n", outfile);
		GDALClose(in_ds);
		GDALClose(out_ds);
		exit(1);
	}
	
	// Read all the input raster bands, transform them, and write them out.
	for (i = 0; i < nBands; i++) {
		in_band = GDALGetRasterBand(in_ds, i+1);
		GDALRasterIO(in_band, GF_Read, 0, 0, in_size_x, in_size_y, in_values, in_size_x, in_size_y, GDT_Float64, 0, 0);
		
		// If we should replace the new pixel values with the mean, compute it now.
		if (use_mean) {
			in_values_ptr = in_values;
			cnt = 0;
			for (y = 0; y < in_size_y; y++) {
				for (x = 0; x < in_size_x; x++) {
					if (*in_values_ptr != nullvalue) {
						sum += *in_values_ptr;
						cnt++;
					}
					in_values_ptr++;
				}
			}
			value = sum / cnt;
			fprintf(stdout, "   Using mean value...\n");
			fprintf(stdout, "   Sum of non null cells:       %f\n", sum);
			fprintf(stdout, "   Number of non null cells:    %i\n", cnt);
			fprintf(stdout, "   Mean value of non null cells: %f\n", value);
		}
		
		fprintf(stdout, "   Value for filling the new pixels: %f\n", value);
		
		// Copy all the values from the input to the output raster.
		in_values_ptr = in_values;
		out_values_ptr = out_values;
		for (y = 0; y < upper_margin; y++) {
			for (x = 0; x < out_size_x; x++) {
				*out_values_ptr = value;
				out_values_ptr++;
			}
		}
		for (y = 0; y < in_size_y; y++) {
			for (x = 0; x < left_margin; x++) {
				*out_values_ptr = value;
				out_values_ptr++;
			}
			for (x = 0; x < in_size_x; x++) {
				if (*in_values_ptr == nullvalue) {
					*out_values_ptr = value;
				} else {
					*out_values_ptr = *in_values_ptr;
				}
				in_values_ptr++;
				out_values_ptr++;
			}
			for (x = left_margin+in_size_x; x < out_size_x; x++) {
				*out_values_ptr = value;
				out_values_ptr++;
			}
		}
		for (y = upper_margin+in_size_y; y < out_size_y; y++) {
			for (x = 0; x < out_size_x; x++) {
				*out_values_ptr = value;
				out_values_ptr++;
			}
		}
		
		// Write the raster band out.
		out_band = GDALGetRasterBand(out_ds, i+1);
		GDALRasterIO(out_band, GF_Write, 0, 0, out_size_x, out_size_y, out_values, 
					 out_size_x, out_size_y, GDT_Float64, 0, 0);
		
	}
	
	// Free the allocated memory.
	free(in_values);
	free(out_values);
	
	// Close the input and output files
	GDALClose(in_ds);
	GDALClose(out_ds);
	
	
	printf("r.add.boundary done\n");
	
	return 0;
}




void printUsage () {
	int index = 0;
	while (usage[index] != NULL) {
		printf(usage[index]);
		index++;
	}
}



