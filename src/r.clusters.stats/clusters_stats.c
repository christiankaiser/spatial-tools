/*
 *  clusters_stats.c
 *  r.clusters.stats
 *
 *  Created by Christian on 24.11.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "clusters_stats.h"
#include "raster.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>


int clusters_stats (char *clusters_raster, long long nodata, char *values_raster, char *output_file) {

	long *clusters, *clustersPtr;				// The cluster data array.
	double *values, *valuesPtr;					// The values data array.
	int rasterX, rasterY;						// The size of the raster files.
	int raster2X, raster2Y;
	int ok;
	long nclusters;								// The number of clusters (maximum ID).
	int i, j;
	double *sum, *min, *max, *minPtr, *maxPtr;	// Statistics arrays.
	long *n;
	long index;
	double v;
	FILE *fp;
	
	
	
	// Read the clusters raster file.
	ok = raster_band_read_long(clusters_raster, 1, &clusters, &rasterX, &rasterY);
	if (ok != 0)
	{
		fprintf(stderr, "ERROR. Unable to read clusters raster file.\n");
		return 1;
	}
	
	// Read the values raster file.
	ok = raster_band_read_double(values_raster, 1, &values, &raster2X, &raster2Y);
	if (ok != 0)
	{
		fprintf(stderr, "ERROR. Unable to read values raster file.\n");
		free(clusters);
		return 1;
	}
	if (rasterX != raster2X || rasterY != raster2Y) {
		fprintf(stderr, "ERROR. Both clusters and values raster files must have the same size.\n");
		free(clusters);
		free(values);
		return 1;
	}
	
	
	// Find the maximum ID in the clusters raster file.
	nclusters = 0;
	clustersPtr = clusters;
	for (j = 0; j < rasterY; j++) {
		for (i = 0; i < rasterX; i++) {
			if (*clustersPtr > nclusters) {
				nclusters = *clustersPtr;
			}
			clustersPtr++;
		}
	}
	

	// Allocate the memory for the statistics.
	sum = calloc(nclusters, sizeof(double));
	n = calloc(nclusters, sizeof(long));
	min = calloc(nclusters, sizeof(double));
	max = calloc(nclusters, sizeof(double));
	minPtr = min;
	maxPtr = max;
	for (i = 0; i < nclusters; i++) {
		*minPtr = DBL_MAX;
		*maxPtr = -DBL_MAX;
		minPtr++;
		maxPtr++;
	}

	
	clustersPtr = clusters;
	valuesPtr = values;
	for (j = 0; j < rasterY; j++) {
		for (i = 0; i < rasterX; i++) {
			index = *clustersPtr + 1;
			if (index >= 0 && index < nclusters)
			{
				v = *valuesPtr;
				if (index != nodata) {
					sum[index] += v;
					n[index]++;
					if (v < min[index]) min[index] = v;
					if (v > max[index]) max[index] = v;
				}
			}
			
			clustersPtr++;
			valuesPtr++;
		}
	}
	
	
	free(clusters);
	free(values);
	
	
	
	// Write out the results.
	if (output_file != NULL) {
		fp = fopen(output_file, "w");
		if (fp == NULL) {
			fprintf(stderr, "ERROR. Unable to open output file.");
			free(sum);
			free(n);
			free(min);
			free(max);
			return 1;
		}
	}
	else {
		fp = stdout;
	}

	
	
	
	fprintf(fp, "Cluster\tNumber of cells\tSum\tAverage\tMinimum\tMaximum\n");
	
	
	for (i = 0; i < nclusters; i++) {
		fprintf(fp, "%i\t%i\t%f\t%f\t%f\t%f\n", i, (int)n[i], sum[i], ((double)sum[i]/(double)n[i]), min[i], max[i]);
	}
	
	
	
	
	free(sum);
	free(n);
	free(min);
	free(max);	
	
	if (output_file != NULL) {
		fclose(fp);
	}
	
	
	
	
	return 0;
	
	
}




