/*
 
 This file is part of r.percolation
 
 r.percolation
 Computes the spatial clusters in a raster map, based on percolation.
 
 Version:	1.0
 Date:		18.5.2009
 Author:	Christian Kaiser, christian.kaiser@unil.ch
 
 Copyright (C) 2009 Christian Kaiser.
 
 r.percolation is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 3, or (at your option) any
 later version.
 
 r.percolation is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with Octave; see the file COPYING.  If not, write to the Free
 Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301, USA.
 
 */



#include "percolation.h"

#include "gdal.h"




int percolation (char* iraster, 
				 char* oraster, 
				 char *rasterStats,
				 char* oformat,
				 char* ostats, 
				 int band, 
				 int extneigh, 
				 double bias, 
				 double cluval)
{
	
	GDALDatasetH idataset;				// The input raster dataset.
	GDALDatasetH odataset;				// The output raster dataset.
	int inbands;						// Number of bands of the input raster.
	GDALRasterBandH iband;				// The input raster band.
	GDALRasterBandH oband;				// The output raster band.
	int rasterX, rasterY;				// The size of the raster band (in pixels).
	GDALDataType rasterType;			// The pixel data type for the input raster band.
	double *imatrix;					// The content of the input raster band.
	unsigned int *omatrix, *omatrixPtr;		// The content of the output raster band.
	GDALDriverH orasterDriver;			// GDAL Driver for output raster.
	double adfTransform[6];				// Affine transformation information.
	ClusterStatistics clustats;			// Structure for the cluster statistics.
	FILE *fpStats;						// File pointer for cluster statistics.
	unsigned int i;
	double mean;
	double *rstats, *rstatsPtr;
	
	
	
	// Print task information
	printf("\n");
	printf("r.percolate task information:\n");
	printf("   Raster band: %i\n", band);
	printf("   Extended neighborhood: %i\n", extneigh);
	printf("   Bias value: %f\n", bias);
	printf("   Cluster value: %f\n", cluval);
	printf("   Input raster: %s\n", iraster);
	printf("   Ouput raster: %s\n", oraster);
	printf("   Output format: %s\n", oformat);
	printf("   Ouput statistics: %s\n", ostats);
	printf("\n");
	
	
	GDALAllRegister();
	
	// Open the input raster file.
	idataset = GDALOpen(iraster, GA_ReadOnly);
	if (idataset == NULL)
	{
		fprintf(stderr, "Error. Unable to open input raster '%s'\n", iraster);
		return 1;
	}
	
	// Check the number of bands.
	inbands = GDALGetRasterCount(idataset);
	if (band > inbands)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. The input raster has only %i bands.\n", inbands);
		return 1;
	}
	
	// Get the input raster band.
	iband = GDALGetRasterBand(idataset, band);
	rasterX = GDALGetRasterBandXSize(iband);
	rasterY = GDALGetRasterBandYSize(iband);
	rasterType = GDALGetRasterDataType(iband);
	
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
	omatrix = calloc(rasterX*rasterY, sizeof(unsigned int));
	if (omatrix == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory for output raster.\n");
		return 1;
	}
	
		
	
	// Do the percolation job.
	findClusters(imatrix, omatrix, rasterX, rasterY, extneigh, bias, cluval);
	
	
	// Compute the cluster statistics.
	clustats = clusterStatistics(imatrix, omatrix, rasterX, rasterY);
	
	
	// Remove clusters with sum smaller than cluval.
	if (cluval > 0.0f)
		removeSmallClusters(omatrix, rasterX, rasterY, clustats, cluval);
	
	
	
	
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
	
	
	
	
	// Write the output raster content
	
	// Create the dataset. If the statistic is the cluster number, it will be a UInt32,
	// and a Float64 otherwise.
	if (strcmp(rasterStats, "mean") == 0 ||
		strcmp(rasterStats, "sum") == 0 ||
		strcmp(rasterStats, "min") == 0 ||
		strcmp(rasterStats, "max") == 0)
	{
		odataset = GDALCreate(orasterDriver, oraster, rasterX, rasterY, 1, GDT_Float64, NULL);
		
		// Allocate the memory for holding the statistic values.
		rstats = calloc(rasterX*rasterY, sizeof(double));
	}
	else
	{
		odataset = GDALCreate(orasterDriver, oraster, rasterX, rasterY, 1, GDT_UInt32, NULL);
	}
	
	// Create the georeferencing information in the new file.
	GDALGetGeoTransform(idataset, adfTransform);
	GDALSetGeoTransform(odataset, adfTransform);
	GDALSetProjection(odataset, GDALGetProjectionRef(idataset));
	
	// Create a new band.
	oband = GDALGetRasterBand(odataset, 1);
	
	
	// Write the statistic into the output raster file.
	rstatsPtr = rstats;
	omatrixPtr = omatrix;
	if (strcmp(rasterStats, "mean") == 0)
	{
		for (i = 0; i < (rasterX * rasterY); i++)
		{
			if (*omatrixPtr > 0)
				*rstatsPtr = clustats.sum[*omatrixPtr-1] / (double)clustats.ncells[*omatrixPtr-1];
			else
				*rstatsPtr = 0.0f;
			rstatsPtr++;
			omatrixPtr++;
		}
		GDALRasterIO(oband, GF_Write, 0, 0, rasterX, rasterY, rstats, rasterX, rasterY, GDT_Float64, 0, 0);
		free(rstats);
	}
	else if (strcmp(rasterStats, "sum") == 0)
	{
		for (i = 0; i < (rasterX * rasterY); i++)
		{
			if (*omatrixPtr > 0)
				*rstatsPtr = clustats.sum[*omatrixPtr-1];
			else
				*rstatsPtr = 0.0f;
			rstatsPtr++;
			omatrixPtr++;
		}
		GDALRasterIO(oband, GF_Write, 0, 0, rasterX, rasterY, rstats, rasterX, rasterY, GDT_Float64, 0, 0);
		free(rstats);
	}
	else if (strcmp(rasterStats, "min") == 0)
	{
		for (i = 0; i < (rasterX * rasterY); i++)
		{
			if (*omatrixPtr > 0)
				*rstatsPtr = clustats.min[*omatrixPtr-1];
			else
				*rstatsPtr = 0.0f;
			rstatsPtr++;
			omatrixPtr++;
		}
		GDALRasterIO(oband, GF_Write, 0, 0, rasterX, rasterY, rstats, rasterX, rasterY, GDT_Float64, 0, 0);
		free(rstats);
	}
	else if (strcmp(rasterStats, "max") == 0)
	{
		for (i = 0; i < (rasterX * rasterY); i++)
		{
			if (*omatrixPtr > 0)
				*rstatsPtr = clustats.max[*omatrixPtr-1];
			else
				*rstatsPtr = 0.0f;
			rstatsPtr++;
			omatrixPtr++;
		}
		GDALRasterIO(oband, GF_Write, 0, 0, rasterX, rasterY, rstats, rasterX, rasterY, GDT_Float64, 0, 0);
		free(rstats);
	}
	else
	{
		GDALRasterIO(oband, GF_Write, 0, 0, rasterX, rasterY, omatrix, rasterX, rasterY, GDT_UInt32, 0, 0);
	}
	
	
	
	// Close the raster images.
	GDALClose(idataset);
	GDALClose(odataset);
	
	
	// Free the memory.
	free(imatrix);
	free(omatrix);

	
	
	// Write the statistics to output file if needed.
	if (ostats != NULL)
	{
		fpStats = fopen(ostats, "w");
		if (fpStats == NULL)
		{
			fprintf(stderr, "Error. Unable to create output statistics file: %s\n", ostats);
			return 1;
		}
		
		fprintf(fpStats, "cluster\tncells\tmin\tmax\tsum\tmean\n");
		for (i = 0; i < clustats.nclusters; i++)
		{
			// Write out only clusters with sum > cluval.
			if (clustats.sum[i] > cluval)
			{
				if (clustats.ncells[i] > 0)
					mean = clustats.sum[i] / (double)clustats.ncells[i];
				else
					mean = 0.0f;
				
				fprintf(fpStats, "%i\t%i\t%f\t%f\t%f\t%f\n", (i+1), clustats.ncells[i],
						clustats.min[i], clustats.max[i], clustats.sum[i], mean);
			}
		}
		
		fclose(fpStats);
	}
	
	freeClusterStatistics(clustats);
	
	
	return 0;
}








void findClusters (double *imatrix, 
				   unsigned int *omatrix, 
				   int sizeX,
				   int sizeY,
				   int extneigh, 
				   double bias, 
				   double cluval)
{
	
	unsigned int nclusters;				// The number of clusters.
	int i, j;							// Index variables for x and y coordinates.
	int index;							// The index for the data array.
	
	
	nclusters = 1;
	for (j = 0; j < sizeY; j++)
	{
		for (i = 0; i < sizeX; i++)
		{
			index = i + (j * sizeX);
			if (imatrix[index] > bias && omatrix[index] == 0)
			{
				omatrix[index] = nclusters;
				addNeighborsToCluster(imatrix, omatrix, i, j, sizeX, sizeY, extneigh, bias, nclusters);
				nclusters++;
			}
		}
	}
	
}






void addNeighborsToCluster (double *imatrix, 
							unsigned int *omatrix,
							int pixelX,
							int pixelY,
							int sizeX,
							int sizeY,
							int extneigh,
							double bias,
							unsigned int cluster)
{
	int index;
	
	// Upper neighbor.
	if (pixelY > 0)
	{
		index = pixelX + ((pixelY-1) * sizeX);
		if (imatrix[index] > bias && omatrix[index] == 0)
		{
			omatrix[index] = cluster;
			addNeighborsToCluster(imatrix, omatrix, pixelX, (pixelY-1), sizeX, sizeY, extneigh, bias, cluster);
		}
	}
	
	// Lower neighbor.
	if (pixelY < (sizeY - 1))
	{
		index = pixelX + ((pixelY+1) * sizeX);
		if (imatrix[index] > bias && omatrix[index] == 0)
		{
			omatrix[index] = cluster;
			addNeighborsToCluster(imatrix, omatrix, pixelX, (pixelY+1), sizeX, sizeY, extneigh, bias, cluster);
		}
	}

	// Left neighbor.
	if (pixelX > 0)
	{
		index = (pixelX-1) + (pixelY * sizeX);
		if (imatrix[index] > bias && omatrix[index] == 0)
		{
			omatrix[index] = cluster;
			addNeighborsToCluster(imatrix, omatrix, (pixelX-1), pixelY, sizeX, sizeY, extneigh, bias, cluster);
		}
	}

	// Right neighbor.
	if (pixelX < (sizeX - 1))
	{
		index = (pixelX+1) + (pixelY * sizeX);
		if (imatrix[index] > bias && omatrix[index] == 0)
		{
			omatrix[index] = cluster;
			addNeighborsToCluster(imatrix, omatrix, (pixelX+1), pixelY, sizeX, sizeY, extneigh, bias, cluster);
		}
	}

	
	
	if (extneigh)
	{
		// Upper left neighbor.
		if (pixelY > 0 && pixelX > 0)
		{
			index = (pixelX-1) + ((pixelY-1) * sizeX);
			if (imatrix[index] > bias && omatrix[index] == 0)
			{
				omatrix[index] = cluster;
				addNeighborsToCluster(imatrix, omatrix, (pixelX-1), (pixelY-1), sizeX, sizeY, extneigh, bias, cluster);
			}
		}
		
		// Upper right neighbor.
		if (pixelY > 0 && pixelX < (sizeX - 1))
		{
			index = (pixelX+1) + ((pixelY-1) * sizeX);
			if (imatrix[index] > bias && omatrix[index] == 0)
			{
				omatrix[index] = cluster;
				addNeighborsToCluster(imatrix, omatrix, (pixelX+1), (pixelY-1), sizeX, sizeY, extneigh, bias, cluster);
			}
		}
		
		// Lower left neighbor.
		if (pixelY < (sizeY - 1) && pixelX > 0)
		{
			index = (pixelX-1) + ((pixelY+1) * sizeX);
			if (imatrix[index] > bias && omatrix[index] == 0)
			{
				omatrix[index] = cluster;
				addNeighborsToCluster(imatrix, omatrix, (pixelX-1), (pixelY+1), sizeX, sizeY, extneigh, bias, cluster);
			}
		}
		
		// Lower right neighbor.
		if (pixelY < (sizeY - 1) && pixelX < (sizeX - 1))
		{
			index = (pixelX+1) + ((pixelY+1) * sizeX);
			if (imatrix[index] > bias && omatrix[index] == 0)
			{
				omatrix[index] = cluster;
				addNeighborsToCluster(imatrix, omatrix, (pixelX+1), (pixelY+1), sizeX, sizeY, extneigh, bias, cluster);
			}
		}
		
	}
}




void removeSmallClusters(unsigned int *omatrix, int rasterX, int rasterY, ClusterStatistics cstats, double cluval)
{
	unsigned int cindex;
	unsigned int *optr;
	int i;
	
	// Check each cluster
	for (cindex = 0; cindex < cstats.nclusters; cindex++)
	{
		if (cstats.sum[cindex] < cluval)
		{
			// Remove this cluster from the omatrix.
			optr = omatrix;
			for (i = 0; i < (rasterX * rasterY); i++)
			{
				if (*optr == (cindex+1))
					*optr = 0;
				optr++;
			}
		}
	}
}



ClusterStatistics clusterStatistics (double *imatrix, 
									 unsigned int *omatrix, 
									 int rasterX, 
									 int rasterY)
{
	
	int i;
	unsigned int *optr;
	double *iptr;
	unsigned int nclusters;
	ClusterStatistics cstats;
	unsigned int cindex;
	
	// Find the number of clusters (maximum value in the imatrix array).
	optr = omatrix;
	nclusters = 0;
	for (i = 0; i < (rasterX * rasterY); i++)
	{
		if (nclusters < *optr)
			nclusters = *optr;
		optr++;
	}
	
	// Allocate the structure for the statistics.
	cstats = allocateClusterStatistics(nclusters);
	
	// Compute the statistics.
	optr = omatrix;
	iptr = imatrix;
	for (i = 0; i < (rasterX * rasterY); i++)
	{
		if (*optr > 0)
		{
			// The cluster index for the current cell.
			cindex = *optr - 1;
			
			// If the number of cells is 0, we must initialize the min, max and sum values.
			// Otherwise, we update the statistics.
			if (cstats.ncells[cindex] == 0)
			{
				cstats.min[cindex] = *iptr;
				cstats.max[cindex] = *iptr;
				cstats.sum[cindex] = *iptr;
			}
			else
			{
				if (*iptr < cstats.min[cindex])
					cstats.min[cindex] = *iptr;
				if (*iptr > cstats.max[cindex])
					cstats.max[cindex] = *iptr;
				cstats.sum[cindex] += *iptr;
			}
			
			// Increase the number of cells.
			cstats.ncells[cindex]++;
		}
		optr++;
		iptr++;
	}
	
	return cstats;
}





ClusterStatistics allocateClusterStatistics (unsigned int nclusters)
{
	ClusterStatistics cstats;
	
	cstats.nclusters = nclusters;
	cstats.ncells = calloc(nclusters, sizeof(unsigned int));
	cstats.min = malloc(nclusters * sizeof(double));
	cstats.max = malloc(nclusters * sizeof(double));
	cstats.sum = malloc(nclusters * sizeof(double));
	
	if (cstats.ncells == NULL || cstats.min == NULL || cstats.max == NULL || cstats.sum == NULL)
	{
		fprintf(stderr, "Error. Unable to allocate memory for cluster statistics.\n");
		cstats.nclusters = 0;
		return cstats;
	}
	
	return cstats;
}



void freeClusterStatistics (ClusterStatistics cstats)
{
	free(cstats.ncells);
	free(cstats.min);
	free(cstats.max);
	free(cstats.sum);
}


