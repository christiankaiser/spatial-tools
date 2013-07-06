/*
 *  r_change_type.h
 *  r.change.type
 *
 *  Created by Christian Kaiser on 11.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#include <GDAL/gdal.h>



int r_change_type(char *infile, char *outfile, char *new_type);


void convert_byte_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_byte_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_byte_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_byte_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_byte_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_byte_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_byte_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);

void convert_uint16_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint16_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint16_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint16_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint16_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint16_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint16_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);

void convert_int16_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int16_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int16_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int16_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int16_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int16_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int16_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);

void convert_uint32_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint32_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint32_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint32_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint32_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint32_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_uint32_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);

void convert_int32_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int32_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int32_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int32_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int32_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int32_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_int32_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);

void convert_float32_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float32_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float32_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float32_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float32_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float32_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float32_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);

void convert_float64_to_byte(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float64_to_uint16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float64_to_int16(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float64_to_uint32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float64_to_int32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float64_to_float32(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);
void convert_float64_to_float64(GDALRasterBandH in_band, GDALRasterBandH out_band, int sizex, int sizey);

