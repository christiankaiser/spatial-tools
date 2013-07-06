/*
 *  boxcount.c
 *  r.fdim.boxcount
 *
 *  Created by Christian Kaiser on 06.06.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "boxcount.h"

#include <gdal/gdal.h>
#include <stdlib.h>
#include <math.h>



int boxcount (char *input_raster, int band, int minbox, int maxbox, char *vdom, char *output_plot)
{

	int rasterX, rasterY;		// The size of the raster files.
	double *input_data;			// The content of the input raster.
	short has_vdom;				// Is a validity domain defined?
	int *vdom_data;				// The content of the vdom raster.
	GDALDatasetH idataset;		// The input dataset.
	GDALRasterBandH iband;		// The input band.
	
	
	vdom_data = NULL;
	
	
	// Print task information
	printf("\n");
	printf("r.fdim.boxcount task information:\n");
	printf("   Input raster file: %s\n", input_raster);
	printf("   Raster band: %i\n", band);
	printf("   Minimum box number: %i\n", minbox);
	printf("   Maximum box number: %i\n", maxbox);
	printf("   Validity domain raster: %s\n", vdom);
	printf("   Output plot file path: %s\n", output_plot);
	printf("\n");
	
	
	// Register all raster drivers.
	GDALAllRegister();
	
	
	// Open the input raster file.
	idataset = GDALOpen(input_raster, GA_ReadOnly);
	if (idataset == NULL)
	{
		fprintf(stderr, "Error. Unable to open input raster '%s'\n", input_raster);
		return 1;
	}
	
	
	// Get the size of the input raster.
	rasterX = GDALGetRasterXSize(idataset);
	rasterY = GDALGetRasterYSize(idataset);
	
	
	// Get the input raster band.
	iband = GDALGetRasterBand(idataset, band);
	if (iband == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Input band %i not available.\n", band);
		return 1;
	}
	
	
	// Fetch the input raster band content.
	input_data = (double*)malloc(rasterX * rasterY * sizeof(double));
	if (input_data == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory to read input raster.\n");
		return 1;
	}
	GDALRasterIO(iband, GF_Read, 0, 0, rasterX, rasterY, input_data, rasterX, rasterY, GDT_Float64, 0, 0);
	
	
	
	
	// Get the validity domain.
	if (vdom != NULL)
	{
		has_vdom = 1;
		vdom_data = (int*)malloc(rasterX * rasterY * sizeof(int));
		if (vdom_data == NULL)
		{
			fprintf(stderr, "Error. Not enough memory to read validity domain raster.\n");
			return 1;
		}
		
		// Open the validity domain raster file.
		idataset = GDALOpen(vdom, GA_ReadOnly);
		if (idataset == NULL)
		{
			fprintf(stderr, "Error. Unable to open validity domain raster '%s'\n", input_raster);
			return 1;
		}
		
		// Get the input raster band.
		iband = GDALGetRasterBand(idataset, 1);

		GDALRasterIO(iband, GF_Read, 0, 0, rasterX, rasterY, vdom_data, rasterX, rasterY, GDT_Int32, 0, 0);
	}
	else
	{
		has_vdom = 0;
	}
	
	
	
	
	computeBoxcount(input_data, rasterX, rasterY, vdom_data, minbox, maxbox, output_plot);
	
	
	
	
	free(input_data);
	if (has_vdom)
		free(vdom_data);
	
	return 0;
}








void computeBoxcount(double *input_data, int rasterX, int rasterY, 
					 int *vdom_data, int minbox, int maxbox, char *plot_file)
{
	
	st_boxcount *ncells, *ncells_ptr;
	st_boxcount *ncells_vdom, *ncells_vdom_ptr;
	
	int i;
	double *random_data;				// Array for the random image inside vdom.
	int nonzero_values;
	double fdim, fdim_vdom;
	
	
	// Allocate the memory for holding the number of cells for each box size to compute.
	ncells = (st_boxcount*)calloc(maxbox - minbox + 1, sizeof(st_boxcount));
	ncells_ptr = ncells;
	
	if (vdom_data != NULL)
	{
		ncells_vdom = (st_boxcount*)calloc(maxbox - minbox + 1, sizeof(st_boxcount));
		ncells_vdom_ptr = ncells_vdom;
	}
	
	
	fprintf(stdout, "FRACTAL DIMENSION ESTIMATION FOR INPUT RASTER\n");
	fprintf(stdout, "---------------------------------------------\n");
	
	// Compute the number of cells for each box size.
	fprintf(stdout, "Fractal dimension estimation using boxcounting\n\n");
	fprintf(stdout, "Box size [in pixels]\tNumber of filled boxes\n");
	for (i = minbox; i <= maxbox; i++)
	{
		*ncells_ptr = numberOfCellsForNumberOfBoxes(i, input_data, rasterX, rasterY);
		fprintf(stdout, "%f\t%i\n", ncells_ptr->boxsize, ncells_ptr->nboxes);
		ncells_ptr++;
	}
	fprintf(stdout, "\n");
	
	
	// Estimate the fractal dimension using a linear regression.
	fdim = estimateFDim(ncells, maxbox-minbox+1);
	
	
	// If there is a validity domain, compute the fractal dimension for the
	// random map of the validity domain.
	if (vdom_data != NULL)
	{
		random_data = calloc(rasterX * rasterY, sizeof(double));
		
		// Count the number of non-zero values in the input raster.
		nonzero_values = countNonZeroValues(input_data, rasterX, rasterY);
		
		// Create the random raster inside the vdom.
		randomRasterInsideVDom(nonzero_values, random_data, vdom_data, rasterX, rasterY);
		
		// Compute the fractal dimension for the validity domain.
		fprintf(stdout, "FRACTAL DIMENSION ESTIMATION FOR VALIDITY DOMAIN RASTER\n");
		fprintf(stdout, "-------------------------------------------------------\n");
		
		fprintf(stdout, "Fractal dimension estimation for validity domain using boxcounting\n\n");
		fprintf(stdout, "Box size [in pixels]\tNumber of filled boxes\n");
		for (i = minbox; i <= maxbox; i++)
		{
			*ncells_vdom_ptr = numberOfCellsForNumberOfBoxes(i, random_data, rasterX, rasterY);
			fprintf(stdout, "%f\t%i\n", ncells_vdom_ptr->boxsize, ncells_vdom_ptr->nboxes);
			ncells_vdom_ptr++;
		}
		fprintf(stdout, "\n");
		
		fdim_vdom = estimateFDim(ncells_vdom, maxbox-minbox+1);
		
		fprintf(stdout, "Validty domain corrected fractal dimension: %f\n\n", (fdim * 2.0f / fdim_vdom));
		
		free(random_data);
	}
	
	
	
	// Draw the SVG plot if necessary.
	if (plot_file != NULL)
	{
		if (vdom_data == NULL)
			plot_fdim(plot_file, ncells, minbox, maxbox, NULL, fdim, 0.0f);
		else
			plot_fdim(plot_file, ncells, minbox, maxbox, ncells_vdom, fdim, fdim_vdom);
	}
	
	
	
	if (vdom_data != NULL)
		free(ncells_vdom);
	
	free(ncells);
	return;
}








st_boxcount numberOfCellsForNumberOfBoxes (int nboxes, double *input_data, int rasterX, int rasterY)
{
	st_boxcount bc;
	double boxsize;
	int i, j, ix, iy;
	int minx, maxx, miny, maxy;
	int ncells;
	int index;
	
	bc.boxsize = 0.0f;
	bc.nboxes = -1;
	
	// Compute the box size in pixels.
	boxsize = MAX((double)rasterX / (double)nboxes, (double)rasterY / (double)nboxes);
	if (boxsize < 2)
	{
		fprintf(stderr, "Box size too small for counting the number of cells.\n");
		return bc;
	}
	
	
	ncells = 0;
	for (j = 0; j < nboxes; j++)
	{
		miny = (int)roundtol((double)j * boxsize);
		maxy = (int)roundtol((double)(j+1) * boxsize);
		if (maxy >= rasterY) maxy = rasterY - 1;
		for (i = 0; i < nboxes; i++)
		{
			minx = (int)roundtol((double)i * boxsize);
			maxx = (int)roundtol((double)(i+1) * boxsize);
			if (maxx >= rasterX) maxx = rasterX - 1;
			for (iy = miny; iy <= maxy; iy++)
			{
				for (ix = minx; ix <= maxx; ix++)
				{
					index = ix + (iy * rasterX);
					if (input_data[index] > 0.0f)
					{
						ncells++;
						ix = maxx + 1;
						iy = maxy + 1;
					}
				}
			}
		}
	}
	
	bc.boxsize = boxsize;
	bc.nboxes = ncells;
	return bc;
	
}






double estimateFDim (st_boxcount *cnts, int ncnts)
{

	int i;
	double Sx, Sy, Sxx, Sxy;
	
	// Computing the data summary.
	Sx = 0.0;
	Sy = 0.0;
	Sxx = 0.0;
	Sxy = 0.0;
	for (i = 0; i < ncnts; i++)
	{
		Sx += log10(cnts[i].boxsize);
		Sy += log10(cnts[i].nboxes);
		Sxx += log10(cnts[i].boxsize) * log10(cnts[i].boxsize);
		Sxy += log10(cnts[i].boxsize) * log10(cnts[i].nboxes);
	}
	
	// Estimating slope:
	double beta = (((double)ncnts * Sxy) - (Sx * Sy)) / (((double)ncnts * Sxx) - (Sx * Sx));
	fprintf(stdout, "Estimated fractal dimension: %f\n\n", -beta);
	
	// Estimating intercept:
	double alpha = (Sy - (beta * Sx)) / (double)ncnts;
	fprintf(stdout, "Linear regression values:\n");
	fprintf(stdout, "   Slope: %f\n", beta);
	fprintf(stdout, "   Intercept: %f\n", alpha);
	fprintf(stdout, "\n");
	
	return -beta;
}







int countNonZeroValues (double *raster, int sizex, int sizey)
{
	int i;
	double *raster_ptr;
	int nonzero_values;
	
	
	raster_ptr = raster;
	nonzero_values = 0;
	for (i = 0; i < sizex * sizey; i++)
	{
		if (*raster_ptr != 0.0f)
			nonzero_values++;
	
		raster_ptr++;
	}
	
	
	return nonzero_values;
}






void randomRasterInsideVDom(int random_cells, double *random_data, int *vdom_data, int rasterX, int rasterY)
{
	int ncells, found_cells;
	int index;
	
	
	ncells = rasterX * rasterY;
	found_cells = 0;
	
	
	srandomdev();
	while (found_cells < random_cells)
	{
		// Get a random value between 0 and ncells.
		index = roundtol(floor((double)ncells * ((double)random() / (double)RAND_MAX)));
		
		if (vdom_data[index] > 0 && random_data[index] == 0.0f)
		{
			random_data[index] = 1.0f;
			found_cells++;
		}

	}
	
}






void plot_fdim (char *plot_file, 
				st_boxcount* ncells, int minbox, int maxbox, 
				st_boxcount *ncells_vdom, 
				double fdim, double fdim_vdom)
{
}







