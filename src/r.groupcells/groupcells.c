/*
 
 This file is part of r.groupcells
 
 Version:	1.0
 Date:		22.5.2009
 Author:	Christian Kaiser, christian.kaiser@unil.ch
 
 Copyright (C) 2009 Christian Kaiser.
 
 r.groupcells is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 2, or (at your option) any
 later version.
 
 r.groupcells is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with Octave; see the file COPYING.  If not, write to the Free
 Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301, USA.
 
 */



#include "groupcells.h"

#include <GDAL/gdal.h>





int groupcells (char *iraster, 
				char *oraster, 
				char *oformat, 
				int r, 
				char *statistic)
{
	
	GDALDatasetH idataset;				// The input raster dataset.
	GDALDatasetH odataset;				// The output raster dataset.
	int nbands;							// Number of bands of the input raster.
	int bindex;							// Band index.
	GDALRasterBandH iband;				// The input raster band.
	GDALRasterBandH oband;				// The output raster band.
	int ix, iy, ox, oy;					// The size of the input and output raster (in pixels).
	GDALDataType rasterType;			// The pixel data type for the input raster band.
	double *imatrix;					// The content of the input raster band.
	double *omatrix;					// The content of the output raster band.
	double *omatrixPtr;
	GDALDriverH orasterDriver;			// GDAL Driver for output raster.
	double adfTransform[6];				// Affine transformation information.
	int i, j;
	int ixindex, iyindex, kx, ky, index;
	double s;
	
	
	// Print task information
	printf("\n");
	printf("r.groupcells task information:\n");
	printf("   Input raster: %s\n", iraster);
	printf("   Ouput raster: %s\n", oraster);
	printf("   Output format: %s\n", oformat);
	printf("   Number of cells to group together: %i\n", r);
	printf("   Group statistic: %s\n", statistic);
	printf("\n");
	
	
	
	GDALAllRegister();
	
	
	// Open the input raster file.
	idataset = GDALOpen(iraster, GA_ReadOnly);
	if (idataset == NULL)
	{
		fprintf(stderr, "Error. Unable to open input raster '%s'\n", iraster);
		return 1;
	}
	
	
	// Get the number of bands.
	nbands = GDALGetRasterCount(idataset);
	
	
	// Get the size of the input raster.
	ix = GDALGetRasterXSize(idataset);
	iy = GDALGetRasterYSize(idataset);
	
	
	// Compute the output raster size.
	ox = (int)roundtol(ceil((double)ix / (double)r));
	oy = (int)roundtol(ceil((double)iy / (double)r));
	if (ox < 1 || oy < 1)
	{
		fprintf(stderr, "Error. Output raster size invalid.\n");
		GDALClose(idataset);
		return 1;
	}
	
	// Compute the number of input cells missing at the right and at bottom
	int ix_over = ox * r - ix;
	int iy_over = oy * r - iy;
	
	
	
	// Get the raster band type.
	iband = GDALGetRasterBand(idataset, 1);
	rasterType = GDALGetRasterDataType(iband);
	//rasterType = GDT_Float64;
	
	// Create the output raster dataset.
	orasterDriver = GDALGetDriverByName(oformat);
	if (orasterDriver == NULL)
	{
		fprintf(stderr, "Warning. Driver for provided format not found. Using GTiff format.\n");
		orasterDriver = GDALGetDriverByName("GTiff");
		if (orasterDriver == NULL)
		{
			fprintf(stderr, "Error. Unable to get GTiff driver.\n");
			GDALClose(idataset);
			return 1;
		}
	}
	odataset = GDALCreate(orasterDriver, oraster, ox, oy, nbands, rasterType, NULL);
	if (odataset == NULL)
	{
		fprintf(stderr, "Error. Unable to create output dataset.\n");
		GDALClose(idataset);
		return 1;
	}
	
	
	// Set the georeference for the new output raster dataset.
	GDALGetGeoTransform(idataset, adfTransform);
	adfTransform[1] *= r;	// Adjust pixel width.
	adfTransform[5] *= r;	// Adjust pixel height.
	GDALSetGeoTransform(odataset, adfTransform);
	GDALSetProjection(odataset, GDALGetProjectionRef(idataset));
	
	
	// Allocate the memory for the input and output bands.
	imatrix = malloc(ix * iy * sizeof(double));
	if (imatrix == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory to read input raster.\n");
		return 1;
	}
	omatrix = malloc(ox * oy * sizeof(double));
	if (omatrix == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory to create output raster.\n");
		return 1;
	}
	
	
	// Convert one band after another.
	for (bindex = 0; bindex < nbands; bindex++)
	{
		// Get the bands.
		iband = GDALGetRasterBand(idataset, bindex+1);
		oband = GDALGetRasterBand(odataset, bindex+1);
		
		// Read the input band
		GDALRasterIO(iband, GF_Read, 0, 0, ix, iy, imatrix, ix, iy, GDT_Float64, 0, 0);
		
		omatrixPtr = omatrix;
		
		// Compute the output band
		for (j = 0; j < oy; j++)
		{
			for (i = 0; i < ox; i++)
			{
				ixindex = i * r;
				iyindex = j * r;
				
				s = 0.0f;
				
				
				// The output matrix may cover a bigger region than the input matrix.
				// Make sure that we don't read from outside the input matrix...
				int rx = r;
				int ry = r;
				
				if (j == (oy-1))
					ry = r - iy_over;
				
				if (i == (ox-1))
					rx = r -ix_over;
				
					
				for (ky = 0; ky < ry; ky++)
				{
					for (kx = 0; kx < rx; kx++)
					{
						index = (ixindex + kx) + ((iyindex+ky) * ix);
						if (statistic == NULL || strcmp(statistic, "mean") == 0 || strcmp(statistic, "sum") == 0)
						{
							s += imatrix[index];
						}
						else if (strcmp(statistic, "min") == 0)
						{
							if (kx == 0 && ky == 0)
								s = imatrix[index];
							else if (imatrix[index] < s)
								s = imatrix[index];
						}
						else
						{
							if (kx == 0 && ky == 0)
								s = imatrix[index];
							else if (imatrix[index] > s)
								s = imatrix[index];
						}
					}
				}
				
				if (statistic == NULL || strcmp(statistic, "mean") == 0)
					s = s / (double)(r*r);
				
				//s = imatrix[ixindex + (iyindex * ix)];
				//index = i + (j * ox);
				//omatrix[index] = s;
				*omatrixPtr = s;
				omatrixPtr++;
				
			}
		}
		
		
		// Write the output raster band.
		char* omatrix_char;
		short* omatrix_int16;
		unsigned short* omatrix_uint16;
		long* omatrix_int32;
		unsigned long* omatrix_uint32;
		float* omatrix_float32;
		switch (rasterType)
		{
			case GDT_Byte:
				omatrix_char = arrayConvertFloat64ToChar(omatrix, ox*oy);
				GDALRasterIO(oband, GF_Write, 0, 0, ox, oy, omatrix_char, ox, oy, rasterType, 0, 0);
				free(omatrix_char);
				break;
				
			case GDT_Int16:
				omatrix_int16 = arrayConvertFloat64ToInt16(omatrix, ox*oy);
				GDALRasterIO(oband, GF_Write, 0, 0, ox, oy, omatrix_int16, ox, oy, rasterType, 0, 0);
				free(omatrix_int16);
				break;
				
			case GDT_UInt16:
				omatrix_uint16 = arrayConvertFloat64ToUInt16(omatrix, ox*oy);
				GDALRasterIO(oband, GF_Write, 0, 0, ox, oy, omatrix_uint16, ox, oy, rasterType, 0, 0);
				free(omatrix_uint16);
				break;
				
			case GDT_Int32:
				omatrix_int32 = arrayConvertFloat64ToInt32(omatrix, ox*oy);
				GDALRasterIO(oband, GF_Write, 0, 0, ox, oy, omatrix_int32, ox, oy, rasterType, 0, 0);
				free(omatrix_int32);
				break;
				
			case GDT_UInt32:
				omatrix_uint32 = arrayConvertFloat64ToUInt32(omatrix, ox*oy);
				GDALRasterIO(oband, GF_Write, 0, 0, ox, oy, omatrix_uint32, ox, oy, rasterType, 0, 0);
				free(omatrix_uint32);
				break;
				
			case GDT_Float32:
				omatrix_float32 = arrayConvertFloat64ToFloat32(omatrix, ox*oy);
				GDALRasterIO(oband, GF_Write, 0, 0, ox, oy, omatrix_float32, ox, oy, rasterType, 0, 0);
				free(omatrix_float32);
				break;
				
			default:
				GDALRasterIO(oband, GF_Write, 0, 0, ox, oy, omatrix, ox, oy, rasterType, 0, 0);
		}

	}
	
	
	free(imatrix);
	free(omatrix);
	GDALClose(idataset);
	GDALClose(odataset);
	
	return 0;
}








char* arrayConvertFloat64ToChar(double *data, int len)
{
	char *arr = malloc(len * sizeof(char));
	char *arrPtr = arr;
	double *dataPtr = data;
	
	int i;
	for (i = 0; i < len; i++)
	{
		*arrPtr = (char)roundtol(*dataPtr);
		arrPtr++;
		dataPtr++;
	}
	
	return arr;
}



short* arrayConvertFloat64ToInt16(double *data, int len)
{
	short *arr = malloc(len * sizeof(short));
	short *arrPtr = arr;
	double *dataPtr = data;
	
	int i;
	for (i = 0; i < len; i++)
	{
		*arrPtr = (short)roundtol(*dataPtr);
		arrPtr++;
		dataPtr++;
	}
	
	return arr;
}



unsigned short* arrayConvertFloat64ToUInt16(double *data, int len)
{
	unsigned short *arr = malloc(len * sizeof(unsigned short));
	unsigned short *arrPtr = arr;
	double *dataPtr = data;
	
	int i;
	for (i = 0; i < len; i++)
	{
		*arrPtr = (unsigned short)roundtol(*dataPtr);
		arrPtr++;
		dataPtr++;
	}
	
	return arr;
}



long* arrayConvertFloat64ToInt32(double *data, int len)
{
	long *arr = malloc(len * sizeof(long));
	long *arrPtr = arr;
	double *dataPtr = data;
	
	int i;
	for (i = 0; i < len; i++)
	{
		*arrPtr = (long)roundtol(*dataPtr);
		arrPtr++;
		dataPtr++;
	}
	
	return arr;
}



unsigned long* arrayConvertFloat64ToUInt32(double *data, int len)
{
	unsigned long *arr = malloc(len * sizeof(unsigned long));
	unsigned long *arrPtr = arr;
	double *dataPtr = data;
	
	int i;
	for (i = 0; i < len; i++)
	{
		*arrPtr = (unsigned long)roundtol(*dataPtr);
		arrPtr++;
		dataPtr++;
	}
	
	return arr;
}



float* arrayConvertFloat64ToFloat32(double *data, int len)
{
	float *arr = malloc(len * sizeof(float));
	float *arrPtr = arr;
	double *dataPtr = data;
	
	int i;
	for (i = 0; i < len; i++)
	{
		*arrPtr = (float)*dataPtr;
		arrPtr++;
		dataPtr++;
	}
	
	return arr;
}




