/*
 *  r_change_type.c
 *  r.change.type
 *
 *  Created by Christian Kaiser on 11.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#include <limits.h>
#include <float.h>

#include <GDAL/cpl_string.h>

#include "r_change_type.h"



int r_change_type(char *infile, char *outfile, char *new_type) {

	int i;
	
	GDALDatasetH in_ds, out_ds;				// Input and output datasets.
	GDALDataType in_type, out_type;			// Input and output data types.
	GDALRasterBandH in_band, out_band;		// Raster band.
	GDALDriverH hDriver;					// The file format driver.
	char **driverMetadata;					// Driver metadata to check if driver is able to create a new file.
	int sizex, sizey;						// Raster size.
	int nbands;								// Number of bands in the input file.
	double georef[6];						// Georeference for the input and output raster file.
	const char *projection;					// Raster SRS.
	
	
	GDALAllRegister();
	
	// Open the input dataset.
	in_ds = GDALOpen(infile, GA_ReadOnly);
	if (in_ds == NULL) {
		fprintf(stderr,"Error. Unable to open file `%s'\n", infile);
		exit(1);
	}
	
	// Get the GDAL driver, and check if we can use it for creating a new file.
	// If not, we will default to the HFA driver instead.
	hDriver = GDALGetDatasetDriver(in_ds);
	driverMetadata = GDALGetMetadata(hDriver, NULL);
	if (!CSLFetchBoolean(driverMetadata, GDAL_DCAP_CREATE, FALSE)) {
		fprintf(stderr, 
				"Warning. Unable to create a `%s' file. Switching to HFA (ERDAS Imagine).\n", 
				GDALGetDriverLongName(hDriver));
		hDriver = GDALGetDriverByName("HFA");
	}
	
	// Read the raster size and the number of bands.
	sizex = GDALGetRasterXSize(in_ds);
	sizey = GDALGetRasterYSize(in_ds);
	nbands = GDALGetRasterCount(in_ds);
	
	// Get the georeference and SRS.
	GDALGetGeoTransform(in_ds, georef);
	projection = GDALGetProjectionRef(in_ds);
	
	// Create a new empty output file.
	out_type = GDALGetDataTypeByName(new_type);
	if (out_type == GDT_Unknown) {
		fprintf(stderr,"Error. New raster type `%s' unknown\n", new_type);
		exit(1);
	}
	out_ds = GDALCreate(hDriver, outfile, sizex, sizey, nbands, out_type, NULL);
	GDALSetGeoTransform(out_ds, georef);
	GDALSetProjection(out_ds, projection);
	
	
	// Loop through all bands and convert each of them separately.
	for (i = 1; i <= nbands; i++) {
		in_band = GDALGetRasterBand(in_ds, i);
		in_type = GDALGetRasterDataType(in_band);
		out_band = GDALGetRasterBand(out_ds, i);
		if (in_type == GDT_Byte) {
			switch (out_type) {
				case GDT_Byte:
					convert_byte_to_byte(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt16:
					convert_byte_to_uint16(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int16:
					convert_byte_to_int16(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt32:
					convert_byte_to_uint32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int32:
					convert_byte_to_int32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float32:
					convert_byte_to_float32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float64:
					convert_byte_to_float64(in_band, out_band, sizex, sizey);
					break;
				default:
					fprintf(stderr,"Error. Unable to convert to raster type `%s'\n", GDALGetDataTypeName(out_type));
					exit(1);
					break;
			}
		} else if (in_type == GDT_UInt16) {
			switch (out_type) {
				case GDT_Byte:
					convert_uint16_to_byte(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt16:
					convert_uint16_to_uint16(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int16:
					convert_uint16_to_int16(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt32:
					convert_uint16_to_uint32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int32:
					convert_uint16_to_int32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float32:
					convert_uint16_to_float32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float64:
					convert_uint16_to_float64(in_band, out_band, sizex, sizey);
					break;
				default:
					fprintf(stderr,"Error. Unable to convert to raster type `%s'\n", GDALGetDataTypeName(out_type));
					exit(1);
					break;
			}
		} else if (in_type == GDT_Int16) {
			switch (out_type) {
				case GDT_Byte:
					convert_int16_to_byte(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt16:
					convert_int16_to_uint16(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int16:
					convert_int16_to_int16(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt32:
					convert_int16_to_uint32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int32:
					convert_int16_to_int32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float32:
					convert_int16_to_float32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float64:
					convert_int16_to_float64(in_band, out_band, sizex, sizey);
					break;
				default:
					fprintf(stderr,"Error. Unable to convert to raster type `%s'\n", GDALGetDataTypeName(out_type));
					exit(1);
					break;
			}
		} else if (in_type == GDT_UInt32) {
			switch (out_type) {
				case GDT_Byte:
					convert_uint32_to_byte(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt16:
					convert_uint32_to_uint16(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int16:
					convert_uint32_to_int16(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt32:
					convert_uint32_to_uint32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int32:
					convert_uint32_to_int32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float32:
					convert_uint32_to_float32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float64:
					convert_uint32_to_float64(in_band, out_band, sizex, sizey);
					break;
				default:
					fprintf(stderr,"Error. Unable to convert to raster type `%s'\n", GDALGetDataTypeName(out_type));
					exit(1);
					break;
			}
		} else if (in_type == GDT_Int32) {
			switch (out_type) {
				case GDT_Byte:
					convert_int32_to_byte(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt16:
					convert_int32_to_uint16(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int16:
					convert_int32_to_int16(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt32:
					convert_int32_to_uint32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int32:
					convert_int32_to_int32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float32:
					convert_int32_to_float32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float64:
					convert_int32_to_float64(in_band, out_band, sizex, sizey);
					break;
				default:
					fprintf(stderr,"Error. Unable to convert to raster type `%s'\n", GDALGetDataTypeName(out_type));
					exit(1);
					break;
			}
		} else if (in_type == GDT_Float32) {
			switch (out_type) {
				case GDT_Byte:
					convert_float32_to_byte(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt16:
					convert_float32_to_uint16(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int16:
					convert_float32_to_int16(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt32:
					convert_float32_to_uint32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int32:
					convert_float32_to_int32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float32:
					convert_float32_to_float32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float64:
					convert_float32_to_float64(in_band, out_band, sizex, sizey);
					break;
				default:
					fprintf(stderr,"Error. Unable to convert to raster type `%s'\n", GDALGetDataTypeName(out_type));
					exit(1);
					break;
			}
		} else if (in_type == GDT_Float64) {
			switch (out_type) {
				case GDT_Byte:
					convert_float64_to_byte(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt16:
					convert_float64_to_uint16(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int16:
					convert_float64_to_int16(in_band, out_band, sizex, sizey);
					break;
				case GDT_UInt32:
					convert_float64_to_uint32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Int32:
					convert_float64_to_int32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float32:
					convert_float64_to_float32(in_band, out_band, sizex, sizey);
					break;
				case GDT_Float64:
					convert_float64_to_float64(in_band, out_band, sizex, sizey);
					break;
				default:
					fprintf(stderr,"Error. Unable to convert to raster type `%s'\n", GDALGetDataTypeName(out_type));
					exit(1);
					break;
			}
		} else {
			fprintf(stderr,"Error. Unable to convert raster of type `%s'\n", GDALGetDataTypeName(in_type));
			exit(1);
		}
	}
	
	// Close the input and output datasets
	GDALClose(in_ds);
	GDALClose(out_ds);
	
	return 0;
}










void convert_byte_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	char *in_data, *in_ptr;
	char *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(char));
	out_data = malloc(sizex*sizey*sizeof(char));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Byte, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			*out_ptr = (char)*in_ptr;
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Byte, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_byte_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	char *in_data, *in_ptr;
	unsigned short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(char));
	out_data = malloc(sizex*sizey*sizeof(unsigned short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Byte, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else {
				*out_ptr = (unsigned short)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_byte_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	char *in_data, *in_ptr;
	short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(char));
	out_data = malloc(sizex*sizey*sizeof(short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Byte, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			*out_ptr = (short)*in_ptr;
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_byte_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	char *in_data, *in_ptr;
	unsigned int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(char));
	out_data = malloc(sizex*sizey*sizeof(unsigned int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Byte, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else if (*in_ptr > UINT_MAX) {
				*out_ptr = UINT_MAX;
			} else {
				*out_ptr = (unsigned int)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_byte_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	char *in_data, *in_ptr;
	int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(char));
	out_data = malloc(sizex*sizey*sizeof(int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Byte, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			*out_ptr = (int)*in_ptr;
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_byte_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	char *in_data, *in_ptr;
	float *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(char));
	out_data = malloc(sizex*sizey*sizeof(float));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Byte, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < FLT_MIN) {
				*out_ptr = FLT_MIN;
			} else if (*in_ptr > FLT_MAX) {
				*out_ptr = FLT_MAX;
			} else {
				*out_ptr = (float)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_byte_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	char *in_data, *in_ptr;
	double *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(char));
	out_data = malloc(sizex*sizey*sizeof(double));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Byte, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < DBL_MIN) {
				*out_ptr = DBL_MIN;
			} else if (*in_ptr > DBL_MAX) {
				*out_ptr = DBL_MAX;
			} else {
				*out_ptr = (double)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float64, 0, 0);
	
	free(in_data);
	free(out_data);
	
}












void convert_uint16_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned short *in_data, *in_ptr;
	char *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned short));
	out_data = malloc(sizex*sizey*sizeof(char));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr > CHAR_MAX) {
				*out_ptr = CHAR_MAX;
			} else {
				*out_ptr = (char)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Byte, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint16_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned short *in_data, *in_ptr;
	unsigned short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned short));
	out_data = malloc(sizex*sizey*sizeof(unsigned short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			*out_ptr = (unsigned short)*in_ptr;
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint16_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned short *in_data, *in_ptr;
	short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned short));
	out_data = malloc(sizex*sizey*sizeof(short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr > SHRT_MAX) {
				*out_ptr = SHRT_MAX;
			} else {
				*out_ptr = (short)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint16_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned short *in_data, *in_ptr;
	unsigned int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned short));
	out_data = malloc(sizex*sizey*sizeof(unsigned int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			*out_ptr = (unsigned int)*in_ptr;
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint16_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned short *in_data, *in_ptr;
	int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned short));
	out_data = malloc(sizex*sizey*sizeof(int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			*out_ptr = (int)*in_ptr;
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint16_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned short *in_data, *in_ptr;
	float *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned short));
	out_data = malloc(sizex*sizey*sizeof(float));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < FLT_MIN) {
				*out_ptr = FLT_MIN;
			} else if (*in_ptr > FLT_MAX) {
				*out_ptr = FLT_MAX;
			} else {
				*out_ptr = (float)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint16_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned short *in_data, *in_ptr;
	double *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned short));
	out_data = malloc(sizex*sizey*sizeof(double));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < DBL_MIN) {
				*out_ptr = DBL_MIN;
			} else if (*in_ptr > DBL_MAX) {
				*out_ptr = DBL_MAX;
			} else {
				*out_ptr = (double)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float64, 0, 0);
	
	free(in_data);
	free(out_data);
	
}















void convert_int16_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	short *in_data, *in_ptr;
	char *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(short));
	out_data = malloc(sizex*sizey*sizeof(char));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < CHAR_MIN) {
				*out_ptr = CHAR_MIN;
			} else if (*in_ptr > CHAR_MAX) {
				*out_ptr = CHAR_MAX;
			} else {
				*out_ptr = (char)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Byte, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int16_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	short *in_data, *in_ptr;
	unsigned short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(short));
	out_data = malloc(sizex*sizey*sizeof(unsigned short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else {
				*out_ptr = (unsigned short)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int16_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	short *in_data, *in_ptr;
	short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(short));
	out_data = malloc(sizex*sizey*sizeof(short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			*out_ptr = (short)*in_ptr;
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int16_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	short *in_data, *in_ptr;
	unsigned int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(short));
	out_data = malloc(sizex*sizey*sizeof(unsigned int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else if (*in_ptr > UINT_MAX) {
				*out_ptr = UINT_MAX;
			} else {
				*out_ptr = (unsigned int)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int16_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	short *in_data, *in_ptr;
	int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(short));
	out_data = malloc(sizex*sizey*sizeof(int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			*out_ptr = (int)*in_ptr;
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int16_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	short *in_data, *in_ptr;
	float *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(short));
	out_data = malloc(sizex*sizey*sizeof(float));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < FLT_MIN) {
				*out_ptr = FLT_MIN;
			} else if (*in_ptr > FLT_MAX) {
				*out_ptr = FLT_MAX;
			} else {
				*out_ptr = (float)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int16_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	short *in_data, *in_ptr;
	double *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(short));
	out_data = malloc(sizex*sizey*sizeof(double));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int16, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < DBL_MIN) {
				*out_ptr = DBL_MIN;
			} else if (*in_ptr > DBL_MAX) {
				*out_ptr = DBL_MAX;
			} else {
				*out_ptr = (double)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float64, 0, 0);
	
	free(in_data);
	free(out_data);
	
}













void convert_uint32_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned int *in_data, *in_ptr;
	char *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned int));
	out_data = malloc(sizex*sizey*sizeof(char));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < CHAR_MIN) {
				*out_ptr = CHAR_MIN;
			} else if (*in_ptr > CHAR_MAX) {
				*out_ptr = CHAR_MAX;
			} else {
				*out_ptr = (char)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Byte, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint32_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned int *in_data, *in_ptr;
	unsigned short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned int));
	out_data = malloc(sizex*sizey*sizeof(unsigned short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else if (*in_ptr > USHRT_MAX) {
				*out_ptr = USHRT_MAX;
			} else {
				*out_ptr = (unsigned short)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint32_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned int *in_data, *in_ptr;
	short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned int));
	out_data = malloc(sizex*sizey*sizeof(short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < SHRT_MIN) {
				*out_ptr = SHRT_MIN;
			} else if (*in_ptr > SHRT_MAX) {
				*out_ptr = SHRT_MAX;
			} else {
				*out_ptr = (short)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint32_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned int *in_data, *in_ptr;
	unsigned int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned int));
	out_data = malloc(sizex*sizey*sizeof(unsigned int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else if (*in_ptr > UINT_MAX) {
				*out_ptr = UINT_MAX;
			} else {
				*out_ptr = (unsigned int)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint32_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned int *in_data, *in_ptr;
	int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned int));
	out_data = malloc(sizex*sizey*sizeof(int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < INT_MIN) {
				*out_ptr = INT_MIN;
			} else if (*in_ptr > INT_MAX) {
				*out_ptr = INT_MAX;
			} else {
				*out_ptr = (int)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint32_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned int *in_data, *in_ptr;
	float *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned int));
	out_data = malloc(sizex*sizey*sizeof(float));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < FLT_MIN) {
				*out_ptr = FLT_MIN;
			} else if (*in_ptr > FLT_MAX) {
				*out_ptr = FLT_MAX;
			} else {
				*out_ptr = (float)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_uint32_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	unsigned int *in_data, *in_ptr;
	double *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(unsigned int));
	out_data = malloc(sizex*sizey*sizeof(double));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_UInt32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < DBL_MIN) {
				*out_ptr = DBL_MIN;
			} else if (*in_ptr > DBL_MAX) {
				*out_ptr = DBL_MAX;
			} else {
				*out_ptr = (double)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float64, 0, 0);
	
	free(in_data);
	free(out_data);
	
}














void convert_int32_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	int *in_data, *in_ptr;
	char *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(int));
	out_data = malloc(sizex*sizey*sizeof(char));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < CHAR_MIN) {
				*out_ptr = CHAR_MIN;
			} else if (*in_ptr > CHAR_MAX) {
				*out_ptr = CHAR_MAX;
			} else {
				*out_ptr = (char)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Byte, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int32_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	int *in_data, *in_ptr;
	unsigned short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(int));
	out_data = malloc(sizex*sizey*sizeof(unsigned short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else if (*in_ptr > USHRT_MAX) {
				*out_ptr = USHRT_MAX;
			} else {
				*out_ptr = (unsigned short)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int32_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	int *in_data, *in_ptr;
	short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(int));
	out_data = malloc(sizex*sizey*sizeof(short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < SHRT_MIN) {
				*out_ptr = SHRT_MIN;
			} else if (*in_ptr > SHRT_MAX) {
				*out_ptr = SHRT_MAX;
			} else {
				*out_ptr = (short)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int32_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	int *in_data, *in_ptr;
	unsigned int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(int));
	out_data = malloc(sizex*sizey*sizeof(unsigned int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else if (*in_ptr > UINT_MAX) {
				*out_ptr = UINT_MAX;
			} else {
				*out_ptr = (unsigned int)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int32_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	int *in_data, *in_ptr;
	int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(int));
	out_data = malloc(sizex*sizey*sizeof(int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < INT_MIN) {
				*out_ptr = INT_MIN;
			} else if (*in_ptr > INT_MAX) {
				*out_ptr = INT_MAX;
			} else {
				*out_ptr = (int)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int32_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	int *in_data, *in_ptr;
	float *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(int));
	out_data = malloc(sizex*sizey*sizeof(float));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < FLT_MIN) {
				*out_ptr = FLT_MIN;
			} else if (*in_ptr > FLT_MAX) {
				*out_ptr = FLT_MAX;
			} else {
				*out_ptr = (float)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_int32_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	int *in_data, *in_ptr;
	double *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(int));
	out_data = malloc(sizex*sizey*sizeof(double));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Int32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < DBL_MIN) {
				*out_ptr = DBL_MIN;
			} else if (*in_ptr > DBL_MAX) {
				*out_ptr = DBL_MAX;
			} else {
				*out_ptr = (double)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float64, 0, 0);
	
	free(in_data);
	free(out_data);
	
}











void convert_float32_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	float *in_data, *in_ptr;
	char *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(float));
	out_data = malloc(sizex*sizey*sizeof(char));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < CHAR_MIN) {
				*out_ptr = CHAR_MIN;
			} else if (*in_ptr > CHAR_MAX) {
				*out_ptr = CHAR_MAX;
			} else {
				*out_ptr = (char)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Byte, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float32_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	float *in_data, *in_ptr;
	unsigned short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(float));
	out_data = malloc(sizex*sizey*sizeof(unsigned short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else if (*in_ptr > USHRT_MAX) {
				*out_ptr = USHRT_MAX;
			} else {
				*out_ptr = (unsigned short)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float32_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	float *in_data, *in_ptr;
	short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(float));
	out_data = malloc(sizex*sizey*sizeof(short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < SHRT_MIN) {
				*out_ptr = SHRT_MIN;
			} else if (*in_ptr > SHRT_MAX) {
				*out_ptr = SHRT_MAX;
			} else {
				*out_ptr = (short)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float32_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	float *in_data, *in_ptr;
	unsigned int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(float));
	out_data = malloc(sizex*sizey*sizeof(unsigned int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else if (*in_ptr > UINT_MAX) {
				*out_ptr = UINT_MAX;
			} else {
				*out_ptr = (unsigned int)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float32_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	float *in_data, *in_ptr;
	int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(float));
	out_data = malloc(sizex*sizey*sizeof(int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < INT_MIN) {
				*out_ptr = INT_MIN;
			} else if (*in_ptr > INT_MAX) {
				*out_ptr = INT_MAX;
			} else {
				*out_ptr = (int)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float32_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	float *in_data, *in_ptr;
	float *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(float));
	out_data = malloc(sizex*sizey*sizeof(float));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < FLT_MIN) {
				*out_ptr = FLT_MIN;
			} else if (*in_ptr > FLT_MAX) {
				*out_ptr = FLT_MAX;
			} else {
				*out_ptr = (float)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float32_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	float *in_data, *in_ptr;
	double *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(float));
	out_data = malloc(sizex*sizey*sizeof(double));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float32, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < DBL_MIN) {
				*out_ptr = DBL_MIN;
			} else if (*in_ptr > DBL_MAX) {
				*out_ptr = DBL_MAX;
			} else {
				*out_ptr = (double)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float64, 0, 0);
	
	free(in_data);
	free(out_data);
	
}














void convert_float64_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	double *in_data, *in_ptr;
	char *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(double));
	out_data = malloc(sizex*sizey*sizeof(char));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float64, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < CHAR_MIN) {
				*out_ptr = CHAR_MIN;
			} else if (*in_ptr > CHAR_MAX) {
				*out_ptr = CHAR_MAX;
			} else {
				*out_ptr = (char)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Byte, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float64_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	double *in_data, *in_ptr;
	unsigned short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(double));
	out_data = malloc(sizex*sizey*sizeof(unsigned short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float64, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else if (*in_ptr > USHRT_MAX) {
				*out_ptr = USHRT_MAX;
			} else {
				*out_ptr = (unsigned short)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float64_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	double *in_data, *in_ptr;
	short *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(double));
	out_data = malloc(sizex*sizey*sizeof(short));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float64, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < SHRT_MIN) {
				*out_ptr = SHRT_MIN;
			} else if (*in_ptr > SHRT_MAX) {
				*out_ptr = SHRT_MAX;
			} else {
				*out_ptr = (short)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int16, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float64_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	double *in_data, *in_ptr;
	unsigned int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(double));
	out_data = malloc(sizex*sizey*sizeof(unsigned int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float64, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < 0) {
				*out_ptr = 0;
			} else if (*in_ptr > UINT_MAX) {
				*out_ptr = UINT_MAX;
			} else {
				*out_ptr = (unsigned int)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_UInt32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float64_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	double *in_data, *in_ptr;
	int *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(double));
	out_data = malloc(sizex*sizey*sizeof(int));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float64, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < INT_MIN) {
				*out_ptr = INT_MIN;
			} else if (*in_ptr > INT_MAX) {
				*out_ptr = INT_MAX;
			} else {
				*out_ptr = (int)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Int32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float64_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	double *in_data, *in_ptr;
	float *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(double));
	out_data = malloc(sizex*sizey*sizeof(float));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float64, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < FLT_MIN) {
				*out_ptr = FLT_MIN;
			} else if (*in_ptr > FLT_MAX) {
				*out_ptr = FLT_MAX;
			} else {
				*out_ptr = (float)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float32, 0, 0);
	
	free(in_data);
	free(out_data);
	
}


void convert_float64_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey) {
	
	double *in_data, *in_ptr;
	double *out_data, *out_ptr;
	int i, j;
	
	in_data = malloc(sizex*sizey*sizeof(double));
	out_data = malloc(sizex*sizey*sizeof(double));
	
	GDALRasterIO(in_band, GF_Read, 0, 0, sizex, sizey, in_data, sizex, sizey, GDT_Float64, 0, 0);
	in_ptr = in_data;
	out_ptr = out_data;
	for (i = 0; i < sizex; i++) {
		for (j = 0; j < sizey; j++) {
			if (*in_ptr < DBL_MIN) {
				*out_ptr = DBL_MIN;
			} else if (*in_ptr > DBL_MAX) {
				*out_ptr = DBL_MAX;
			} else {
				*out_ptr = (double)*in_ptr;
			}
			in_ptr++;
			out_ptr++;
		}
	}
	
	GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, out_data, sizex, sizey, GDT_Float64, 0, 0);
	
	free(in_data);
	free(out_data);
	
}









