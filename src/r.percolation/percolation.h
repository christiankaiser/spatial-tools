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





// A structure containing the cluster statistics.
typedef struct {
	unsigned int nclusters;		// The number of clusters.
	unsigned int *ncells;			// The number of cells for each cluster.
	double *min;					// The minimum value.
	double *max;					// The maximum value.
	double *sum;					// The sum of the cluster.
} ClusterStatistics;





// Performs the percolation job.
// Returns 0 in case of success, and an error code otherwise.
int percolation (char* iraster, 
				 char* oraster, 
				 char* rasterStats,
				 char *oformat,
				 char* ostats, 
				 int band, 
				 int extneigh, 
				 double bias, 
				 double cluval);




void findClusters (double *imatrix, 
				   unsigned int *omatrix, 
				   int sizeX,
				   int sizeY,
				   int extneigh, 
				   double bias, 
				   double cluval);



void addNeighborsToCluster (double *imatrix, 
							unsigned int *omatrix,
							int pixelX,
							int pixelY,
							int sizeX,
							int sizeY,
							int extneigh,
							double bias,
							unsigned int cluster);




void removeSmallClusters(unsigned int *omatrix, int rasterX, int rasterY, ClusterStatistics cstats, double cluval);


// Computes the cluster statistics based on the clusters present in the omatrix.
ClusterStatistics clusterStatistics (double *imatrix, 
									 unsigned int *omatrix, 
									 int rasterX, 
									 int rasterY);




// Allocates the memory for the ClusterStatistics structure
// for the provided number of clusters.
ClusterStatistics allocateClusterStatistics (unsigned int nclusters);


// Frees the memory for the provided ClusterStatistics structure.
void freeClusterStatistics (ClusterStatistics cstats);







