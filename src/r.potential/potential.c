/*
 *  potential.c
 *  r.potential
 *
 *  Created by Christian Kaiser on 23.05.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "potential.h"
#include "gdal.h"

#include <stdlib.h>



int potential(char *iraster, int band, char *oraster, char *oformat, st_variogram *vg)
{
	GDALDatasetH idataset;				// The input raster dataset.
	GDALDatasetH odataset;				// The output raster dataset.
	int nbands;							// Number of bands of the input raster.
	GDALRasterBandH iband;				// The input raster band.
	GDALRasterBandH oband;				// The output raster band.
	int rasterX, rasterY;				// The size of the raster (in pixels).
	double *imatrix;					// The content of the input raster band.
	double *omatrix;					// The content of the output raster band.
	GDALDriverH orasterDriver;			// GDAL Driver for output raster.
	double adfTransform[6];				// Affine transformation information.
	
	
	
	// Print task information
	printf("\n");
	printf("r.potential task information:\n");
	printf("   Input raster: %s\n", iraster);
	printf("   Input raster band: %i\n", band);
	printf("   Ouput raster: %s\n", oraster);
	printf("   Output format: %s\n", oformat);
	printf("   Variogram model type: %s\n", st_variogram_model_name(vg));
	printf("   Variogram sill: %f\n", vg->sill);
	printf("   Variogram range: %f\n", vg->range);
	printf("   Variogram nugget: %f\n", vg->nugget);
	if (vg->model == ST_VGMODEL_POWER)
		printf("   Variogram power: %f\n", vg->power);
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
	if (band > nbands)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. The input raster has only %i bands.\n", nbands);
		return 1;
	}
	
	
	// Get the size of the input raster.
	rasterX = GDALGetRasterXSize(idataset);
	rasterY = GDALGetRasterYSize(idataset);
	
	
	// Get the input raster band.
	iband = GDALGetRasterBand(idataset, band);
	
	
	// Fetch the input raster band content.
	imatrix = (double*) malloc(rasterX * rasterY * sizeof(double));
	if (imatrix == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory to read input raster.\n");
		return 1;
	}
	GDALRasterIO(iband, GF_Read, 0, 0, rasterX, rasterY, imatrix, rasterX, rasterY, GDT_Float64, 0, 0);
	
	
	
	// Allocate the memory for the output raster.
	omatrix = calloc(rasterX*rasterY, sizeof(double));
	if (omatrix == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory for output raster.\n");
		return 1;
	}
	
	
	// Estimate the potential values.
	computePotential(imatrix, omatrix, rasterX, rasterY, vg);
	
	
	
	// Create the output raster driver.
	orasterDriver = GDALGetDriverByName(oformat);
	if (orasterDriver == NULL)
	{
		fprintf(stderr, "Warning. Driver for provided format not found. Using HFA format.\n");
		orasterDriver = GDALGetDriverByName("HFA");
		if (orasterDriver == NULL)
		{
			fprintf(stderr, "Error. Unable to get HFA driver.\n");
			GDALClose(idataset);
			return 1;
		}
	}

	
	// Create the output dataset.
	odataset = GDALCreate(orasterDriver, oraster, rasterX, rasterY, 1, GDT_Float64, NULL);
	
	// Create the georeferencing information in the new file.
	GDALGetGeoTransform(idataset, adfTransform);
	GDALSetGeoTransform(odataset, adfTransform);
	GDALSetProjection(odataset, GDALGetProjectionRef(idataset));
	
	// Create a new band.
	oband = GDALGetRasterBand(odataset, 1);
	
	
	// Write the potential to the output dataset.
	GDALRasterIO(oband, GF_Write, 0, 0, rasterX, rasterY, omatrix, rasterX, rasterY, GDT_Float64, 0, 0);
	
	
	// Close the raster images.
	GDALClose(idataset);
	GDALClose(odataset);
	
	
	// Release the memory.
	free(imatrix);
	free(omatrix);
	
	
	return 0;
	
}









void computePotential(double *imatrix, double *omatrix, int rasterX, int rasterY, st_variogram *vg)
{
	
	int i, j;
	int minx, maxx, miny, maxy;
	double pot;
	double h, w;
	int kx, ky;
	int index;
	double *omatrixPtr;
	int prct, prct_old;				// Percentage done.
	
	
	
	prct = 0;
	prct_old = 0;
	
	
	// Compute the potential for each cell of the output matrix.
	omatrixPtr = omatrix;
	for (j = 0; j < rasterY; j++)
	{
		for (i = 0; i < rasterX; i++)
		{
			pot = 0.0f;
			
			// Compute the extent of the window that we will use for the potential computation.
			minx = MAX(i - 2*vg->range, 0);
			maxx = MIN(i + 2*vg->range + 1, rasterX);
			miny = MAX(j - 2*vg->range, 0);
			maxy = MIN(j + 2*vg->range + 1, rasterY);
			
			// Compute the potential for each cell inside the window.
			for (ky = miny; ky < maxy; ky++)
			{
				for (kx = minx; kx < maxx; kx++)
				{
					// The distance between the two cells (in pixels).
					h = sqrt((kx-i)*(kx-i) + (ky-j)*(ky-j));
					
					// Compute the weight using the variogram.
					w = st_variogram_value(vg, h);
					
					// Compute the potential and sum it up.
					index = kx + (ky * rasterX);
					pot += (1-w) * imatrix[index];
				}
			}
			
			// Write the potential value.
			*omatrixPtr = pot;
			omatrixPtr++;
			
			prct = (int)roundtol(100.0f * ((double)(i+j*rasterX) / (double)(rasterX*rasterY)));
			if (prct != prct_old)
			{
				fprintf(stdout, "%i%% done\n", prct);
				prct_old = prct;
			}
		}
	}
	
	
	
	
	
}






