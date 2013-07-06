/*
 *  downscale.c
 *  r.downscale
 *
 *  Created by Christian Kaiser on 24.05.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "downscale.h"
#include "gdal.h"


#include <stdio.h>
#include <stdlib.h>


int downscale (char *agg_stats, 
			   char *agg_raster, 
			   char *prob_raster, 
			   char *prior_raster, 
			   char *vdom_raster,
			   char *oraster, 
			   char *oformat)
{
	
	int *agg_sum;				// Aggregated statistics.
	int minindex, maxindex;		// Maximum and minimum indices for aggregated statistics.
	int i;
	int rasterX, rasterY;		// The size of the raster files.
	int *agg_data;				// The content of the aggregate raster.
	double *prob_data;			// The content of the probability raster.
	int *prior_data;			// The content of the prior raster.
	int *vdom, *vdom_ptr;		// The content of the validity domain raster.
	int *out_data;				// Output data matrix.
	
	GDALDriverH odriver;		// The output GDAL driver.
	GDALDatasetH idataset;		// An input dataset.
	GDALDatasetH odataset;		// The output dataset.
	GDALRasterBandH oband;		// The output raster band.
	double adfTransform[6];		// Affine transformation information.
	
	
	
	
	
	// Print task information
	printf("\n");
	printf("r.downscale task information:\n");
	printf("   Aggregated statistics file: %s\n", agg_stats);
	printf("   Aggregate raster file: %s\n", agg_raster);
	printf("   Probability raster file: %s\n", prob_raster);
	printf("   Prior raster file: %s\n", prior_raster);
	printf("   Validity domain raster file: %s\n", vdom_raster);
	printf("   Output raster file: %s\n", oraster);
	printf("   Output raster format: %s\n", oformat);
	printf("\n");

	
	
	
	
	// Read aggregated statistics file.
	if (readAggregatedStats(agg_stats, &agg_sum, &minindex, &maxindex) != 0)
	{
		fprintf(stderr, "Error while reading aggregated statistics file.\n");
		return 1;
	}
	
	
	// Register all raster drivers.
	GDALAllRegister();
	
	// Read the aggregate raster.
	if (readAggregateRaster(agg_raster, &agg_data, &rasterX, &rasterY) != 0)
	{
		fprintf(stderr, "Error while reading aggregate raster file.\n");
		return 1;
	}
	
	
	
	// Read the probability raster.
	if (readProbabilityRaster(prob_raster, &prob_data) != 0)
	{
		fprintf(stderr, "Error while reading probability raster file.\n");
		return 1;
	}
	
	
	// Read the prior raster if there is one.
	if (prior_raster != NULL)
	{
		if (readPriorRaster(prior_raster, &prior_data) != 0)
		{
			fprintf(stderr, "Error while reading prior raster file.\n");
			return 1;
		}
	}
	
	
	// Read the validity domain raster if there is one.
	// If we don't have a validity domain raster, just create the array
	// and initialize with 1 (validity domain = whole region)
	if (vdom_raster != NULL)
	{
		if (readValidityDomainRaster(vdom_raster, &vdom) != 0)
		{
			fprintf(stderr, "Error while reading validity domain raster file.\n");
			return 1;
		}
	}
	else
	{
		vdom = malloc(rasterX * rasterY * sizeof(int));
		vdom_ptr = vdom;
		for (i = 0; i < (rasterX * rasterY); i++)
		{
			*vdom_ptr = 1;
			vdom_ptr++;
		}
	}
	
	
	
	
	
	// Allocate the memory for the output raster.
	out_data = calloc(rasterX*rasterY, sizeof(int));
	
	
	
	// Downscale data.
	downscaleData(agg_sum, minindex, maxindex, agg_data, prob_data, prior_data, vdom, out_data, rasterX, rasterY);
	
	
	
	// Write the downscaled data into the output raster file.
	
	odriver = GDALGetDriverByName(oformat);
	if (odriver == NULL)
	{
		fprintf(stderr, "Warning. Driver for provided format not found. Using HFA format.\n");
		odriver = GDALGetDriverByName("HFA");
		if (odriver == NULL)
		{
			fprintf(stderr, "Error. Unable to get HFA driver.\n");
			GDALClose(odataset);
			return 1;
		}
	}
	
	odataset = GDALCreate(odriver, oraster, rasterX, rasterY, 1, GDT_Int32, NULL);
	
	// Create the georeferencing information in the new file.
	idataset = GDALOpen(agg_raster, GA_ReadOnly);
	GDALGetGeoTransform(idataset, adfTransform);
	GDALSetGeoTransform(odataset, adfTransform);
	GDALSetProjection(odataset, GDALGetProjectionRef(idataset));
	GDALClose(idataset);
	
	// Get the input raster band.
	oband = GDALGetRasterBand(odataset, 1);
	
	// Write the potential to the output dataset.
	GDALRasterIO(oband, GF_Write, 0, 0, rasterX, rasterY, out_data, rasterX, rasterY, GDT_Int32, 0, 0);
	
	GDALClose(odataset);
	
	
	// Free the memory.
	free(agg_sum);
	free(agg_data);
	free(prob_data);
	if (prior_data != NULL)
		free(prior_data);
	free(out_data);
	free(vdom);
	
	
	return 0;
}











int readAggregatedStats (char* agg_stats, int** agg_sum, int *minindex, int *maxindex)
{
	
	FILE *fp;
	char line[1024];			// The line read from the file.
	char snumb[1024];			// For copying the values.
	char *linePtr, *snumbPtr;
	int lcontrol;
	int index;
	
	int *agg_sum_ptr;
	
	
	
	// Get the number of lines and find the minimum index.
	fp = fopen(agg_stats, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Error. Unable to open aggregated statistics file '%s'.\n", agg_stats);
		return 1;
	}
	
	// First line contains header.
	fgets(line, 1024, fp);
	
	// Read line by line.
	*minindex = -1;
	*maxindex = -1;
	while (fgets(line, 1024, fp))
	{
		// Read the feature id (must be an integer).
		linePtr = line;
		snumbPtr = snumb;
		lcontrol = 0;
		
		// While linePtr is not a number...
		while ((*linePtr < 48 || *linePtr > 57) && lcontrol < 1024)
		{
			linePtr++;
			lcontrol++;
		}
		
		// While linePtr is a number...
		while (*linePtr >= 48 && *linePtr <= 57 && lcontrol < 1024)
		{
			*snumbPtr = *linePtr;
			linePtr++;
			snumbPtr++;
			lcontrol++;
		}
		
		// Terminate the number.
		*snumbPtr = 0;
		
		// Get the number.
		index = atoi(snumb);
		
		// Copy to the minimum value if needed
		if (*minindex < 0 || *minindex > index)
			*minindex = index;
		
		// Copy to the maximum value if needed
		if (*maxindex < 0 || *maxindex < index)
			*maxindex = index;
		
	}
	
	fclose(fp);
	
	
	
	// Allocate the memory for the statistics value.
	// For efficient memory access, the array index is based on the feature id.
	agg_sum_ptr = calloc(*maxindex - *minindex + 1, sizeof(int));
	if (agg_sum_ptr == NULL)
	{
		fprintf(stderr, "Error. Unable to allocate memory for aggregated statistics (%i bytes).\n", 
				(*maxindex - *minindex + 1)*sizeof(int));
		return 1;
	}
	
	
	
	
	
	// Read the statistical value.
	
	fp = fopen(agg_stats, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Error. Unable to open aggregated statistics file '%s'.\n", agg_stats);
		return 1;
	}
	
	// Header line.
	fgets(line, 1024, fp);
	
	// Read line by line.
	while (fgets(line, 1024, fp))
	{
		linePtr = line;
		snumbPtr = snumb;
		lcontrol = 0;
		
		// While linePtr is not a number...
		while ((*linePtr < 48 || *linePtr > 57) && lcontrol < 1024)
		{
			linePtr++;
			lcontrol++;
		}
		
		// Find the index (feature id)
		// While linePtr is number...
		while (*linePtr >= 48 && *linePtr <= 57 && lcontrol < 1024)
		{
			*snumbPtr = *linePtr;
			snumbPtr++;
			linePtr++;
			lcontrol++;
		}
		
		*snumbPtr = 0;
		index = atoi(snumb);
		if (index < *minindex || index > *maxindex)
		{
			fprintf(stderr, "Error while reading aggregated statistics.\n");
			return 1;
		}
		snumbPtr = snumb;
		
		// While linePtr is not a number...
		// (finding start of stat value)
		while ((*linePtr < 48 || *linePtr > 57) && lcontrol < 1024)
		{
			linePtr++;
			lcontrol++;
		}
		
		// Read the number...
		while (*linePtr >= 48 && *linePtr <= 57 && lcontrol < 1024)
		{
			*snumbPtr = *linePtr;
			snumbPtr++;
			linePtr++;
			lcontrol++;
		}
		
		// Terminate the number.
		*snumbPtr = 0;
		
		// Get the number.
		agg_sum_ptr[index-*minindex] = atoi(snumb);
		
		// Copy to the minimum value if needed
		if (*minindex < 0 || *minindex > index)
			*minindex = index;
		
		// Copy to the maximum value if needed
		if (*maxindex < 0 || *maxindex < index)
			*maxindex = index;
		
	}
	
	fclose(fp);
	
	
	*agg_sum = agg_sum_ptr;

	return 0;
}










int readAggregateRaster(char *agg_raster, int** agg_data, int *rasterX, int *rasterY)
{
	
	GDALDatasetH idataset;
	GDALRasterBandH iband;				// The input raster band.
	size_t agg_data_size;
	
	
	// Open the input raster file.
	idataset = GDALOpen(agg_raster, GA_ReadOnly);
	if (idataset == NULL)
	{
		fprintf(stderr, "Error. Unable to open input raster '%s'\n", agg_raster);
		return 1;
	}
	
	// Get the size of the input raster.
	*rasterX = GDALGetRasterXSize(idataset);
	*rasterY = GDALGetRasterYSize(idataset);
	
	// Get the input raster band.
	iband = GDALGetRasterBand(idataset, 1);
	
	// Fetch the input raster band content.
	agg_data_size = *rasterX * *rasterY * sizeof(int);
	*agg_data = (int*) malloc(agg_data_size);
	if (*agg_data == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory to read aggregate raster.\n");
		return 1;
	}
	GDALRasterIO(iband, GF_Read, 0, 0, *rasterX, *rasterY, *agg_data, *rasterX, *rasterY, GDT_Int32, 0, 0);
	
	GDALClose(idataset);
	
	return 0;
}








int readProbabilityRaster(char *prob_raster, double** prob_data)
{
	
	GDALDatasetH idataset;
	GDALRasterBandH iband;				// The input raster band.
	int x, y;
	
	// Open the input raster file.
	idataset = GDALOpen(prob_raster, GA_ReadOnly);
	if (idataset == NULL)
	{
		fprintf(stderr, "Error. Unable to open probability raster '%s'\n", prob_raster);
		return 1;
	}
	
	// Get the size of the input raster.
	x = GDALGetRasterXSize(idataset);
	y = GDALGetRasterYSize(idataset);
	
	// Get the input raster band.
	iband = GDALGetRasterBand(idataset, 1);
	
	// Fetch the input raster band content.
	*prob_data = (double*) malloc(x * y * sizeof(double));
	if (*prob_data == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory to read probability raster.\n");
		return 1;
	}
	GDALRasterIO(iband, GF_Read, 0, 0, x, y, *prob_data, x, y, GDT_Float64, 0, 0);
	
	GDALClose(idataset);
	
	return 0;
}






int readPriorRaster(char *prior_raster, int** prior_data)
{
	
	GDALDatasetH idataset;
	GDALRasterBandH iband;				// The input raster band.
	int x, y;
	
	// Open the input raster file.
	idataset = GDALOpen(prior_raster, GA_ReadOnly);
	if (idataset == NULL)
	{
		fprintf(stderr, "Error. Unable to open prior raster '%s'\n", prior_raster);
		return 1;
	}
	
	// Get the size of the input raster.
	x = GDALGetRasterXSize(idataset);
	y = GDALGetRasterYSize(idataset);
	
	// Get the input raster band.
	iband = GDALGetRasterBand(idataset, 1);
	
	// Fetch the input raster band content.
	*prior_data = (int*) malloc(x * y * sizeof(int));
	if (*prior_data == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory to read prior raster.\n");
		return 1;
	}
	GDALRasterIO(iband, GF_Read, 0, 0, x, y, *prior_data, x, y, GDT_Int32, 0, 0);
	
	GDALClose(idataset);
	
	return 0;
}







int readValidityDomainRaster(char *vdom_raster, int** vdom)
{
	GDALDatasetH idataset;
	GDALRasterBandH iband;				// The input raster band.
	int x, y;
	
	// Open the input raster file.
	idataset = GDALOpen(vdom_raster, GA_ReadOnly);
	if (idataset == NULL)
	{
		fprintf(stderr, "Error. Unable to open validity domain raster '%s'\n", vdom_raster);
		return 1;
	}
	
	// Get the size of the input raster.
	x = GDALGetRasterXSize(idataset);
	y = GDALGetRasterYSize(idataset);
	
	// Get the input raster band.
	iband = GDALGetRasterBand(idataset, 1);
	
	// Fetch the input raster band content.
	*vdom = (int*) malloc(x * y * sizeof(int));
	if (*vdom == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory to read validity domain raster.\n");
		return 1;
	}
	GDALRasterIO(iband, GF_Read, 0, 0, x, y, *vdom, x, y, GDT_Int32, 0, 0);
	
	GDALClose(idataset);
	
	return 0;
}









void downscaleData(int *agg_sum, int minindex, int maxindex, 
				   int *agg_data, double *prob_data, int *prior_data, int *vdom, int *out_data, 
				   int rasterX, int rasterY)
{

	double *prob_agg;		// Aggregated probability data.
	int *prior_agg;			// Aggregated prior data.
	int i, index;
	int *agg_data_ptr;
	double *prob_data_ptr;
	int *prior_data_ptr;
	int *vdom_ptr;
	int *out_data_ptr;
	int diff_to_distribute;
	
	int prct, prct_old;				// Percentage done.
	
	
	
	
	
	// Compute the sums of the probability and prior data for each aggregated feature.
	
	prob_agg = calloc(maxindex - minindex + 1, sizeof(double));
	prior_agg = calloc(maxindex - minindex + 1, sizeof(int));
	
	agg_data_ptr = agg_data;
	prob_data_ptr = prob_data;
	vdom_ptr = vdom;
	for (i = 0; i < (rasterX*rasterY); i++)
	{
		if (*agg_data_ptr >= minindex && *agg_data_ptr <= maxindex)
		{
			index = *agg_data_ptr - minindex;
			prob_agg[index] += *prob_data_ptr * (double)*vdom_ptr;
		}
		agg_data_ptr++;
		prob_data_ptr++;
		vdom_ptr++;
	}
	

	
	if (prior_data != NULL)
	{
		agg_data_ptr = agg_data;
		prior_data_ptr = prior_data;
		vdom_ptr = vdom;
		for (i = 0; i < (rasterX*rasterY); i++)
		{
			if (*agg_data_ptr >= minindex && *agg_data_ptr <= maxindex)
			{
				index = *agg_data_ptr - minindex;
				prior_agg[index] += *prior_data_ptr * *vdom_ptr;
			}
			agg_data_ptr++;
			prior_data_ptr++;
			vdom_ptr++;
		}
	}
	
	
	
	
	// Copy prior distribution to output distribution, if necessary.
	prior_data_ptr = prior_data;
	out_data_ptr = out_data;
	for (i = 0; i < (rasterX*rasterY); i++)
	{
		*out_data_ptr = *prior_data_ptr;
		out_data_ptr++;
		prior_data_ptr++;
	}
	
	
	
	
	prct = 0;
	prct_old = 0;
	
	
	// For each aggregated statistic, distribute the difference randomly according 
	// to the probability raster.
	srandomdev();
	for (i = minindex; i <= maxindex; i++)
	{
		index = i - minindex;
		
		diff_to_distribute = agg_sum[index] - prior_agg[index];
		
		if (diff_to_distribute != 0)
		{
			printf("Treating feature ID %i. Old value: %i. New value: %i. Difference: %i\n", i, prior_agg[index], agg_sum[index], diff_to_distribute);
			estimateDistribution(i, agg_data,
								 diff_to_distribute, 
								 prob_data, prob_agg[index], 
								 vdom,
								 out_data,
								 rasterX, rasterY);
		}
		
		
		prct = (int)roundtol(100.0f * ((double)(i-minindex) / (double)(maxindex-minindex+1)));
		if (prct != prct_old)
		{
			fprintf(stdout, "%i%% done\n", prct);
			prct_old = prct;
		}
		
	}
	
	
	
	// Free the allocated memory.
	free(prob_agg);
	if (prior_data != NULL)
		free(prior_agg);
	
	
}






void estimateDistribution(int feature_id, int *feature_dist,
						  int diff_to_distribute, 
						  double *prob_dist, double prob_sum, 
						  int *vdom,
						  int *out_dist,
						  int size_x, int size_y)
{
	
	int still_to_distribute;			// Sum left to distribute;
	double rand_value;
	int i, j;
	
	// Distribute the values randomly until there is nothing left to distribute.
	still_to_distribute = diff_to_distribute;
	while (still_to_distribute != 0)
	{
		
		// Get a random value between 0 and prob_sum.
		rand_value = prob_sum * ((double)random() / (double)RAND_MAX);
		
		findIndexForCumulatedValue(rand_value, prob_dist, vdom, feature_id, feature_dist, size_x, size_y, &i, &j);
		
		if (i >= 0 && i < size_x && j >= 0 && j < size_y)
		{
			if (still_to_distribute < 0)
			{
				out_dist[i + (j*size_x)]--;
				still_to_distribute++;
			}
			else
			{
				out_dist[i + (j*size_x)]++;
				still_to_distribute--;
			}
		}
		
	}
	
	
}



void findIndexForCumulatedValue(double cum_val, double *prob_dist, int *vdom,
								int feature_id, int *feature_dist, 
								int size_x, int size_y, 
								int *x, int *y)
{
	
	double current_sum;
	int i, j, last_i, last_j;
	double *prob_dist_ptr;
	int *feature_dist_ptr;
	int *vdom_ptr;
	
	
	current_sum = 0.0f;
	prob_dist_ptr = prob_dist;
	feature_dist_ptr = feature_dist;
	vdom_ptr = vdom;
	
	for (j = 0; j < size_y; j++)
	{
		for (i = 0; i < size_x; i++)
		{
			if (*feature_dist_ptr == feature_id)
			{
				current_sum += *prob_dist_ptr * *vdom_ptr;
				last_i = i;
				last_j = j;
				
				if (current_sum > cum_val)
				{
					*x = i;
					*y = j;
					return;
				}
			}
			
			feature_dist_ptr++;
			prob_dist_ptr++;
			vdom_ptr++;
		}
	}
		   
	
	*x = last_i;
	*y = last_j;
	
	return;	   
}





