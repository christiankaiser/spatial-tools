/*
 *  pycnophylactic.c
 *  r.pycnophylactic
 *
 *  Created by Christian Kaiser on 10.12.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#include <stdio.h>


#include "pycnophylactic.h"
#include "gdal.h"


int pycnophylactic(char *frast, char *vrast, char *orast, int density, int maxIterations) {

	GDALDriverH hDriver;					// The GDAL driver.
	GDALDatasetH fds, vds, ods;				// The raster datasets.
	GDALRasterBandH fband, vband, oband;	// The raster bands.
	int sizex, sizey;						// The size of the raster images (in pixels).
	int *fmatrix;							// The feature matrix
	double *vmatrix, *omatrix;				// The value and output matrices
	double *smatrix;						// The smoothness adjustment matrix
	int nregions;							// The number of regions.
	double *pop, *pop_new;					// Total population per region.
	int itercnt;							// For counting the iterations of the pycnophylactic algorithm.
	int *pixelsPerRegion;					// The number of pixels for each region.
	
	
	// Provide some information about the task.
	fprintf(stdout, "Pycnophylactic interpolation started.\n");
	fprintf(stdout, "   Feature raster:               '%s'\n", frast);
	fprintf(stdout, "   Value raster:                 '%s'\n", vrast);
	fprintf(stdout, "   Output raster:                '%s'\n", orast);
	if (density == 1) {
		fprintf(stdout, "   Computing density:            Yes\n", orast);
	} else {
		fprintf(stdout, "   Computing density:            No\n", orast);
	}
	fprintf(stdout, "   Maximum number of iterations: %i\n", maxIterations);
	fprintf(stdout, "\n");
	
	
	GDALAllRegister();

	
	// If maxIterations is not set (0), define it using a default value
	if (maxIterations <= 0) {
		maxIterations = 1;
	}
	
	// Open the input raster files.
	fds = GDALOpen(frast, GA_ReadOnly);
	if (fds == NULL) {
		fprintf(stderr, "Error. Unable to open feature raster '%s'\n", frast);
		return 1;
	}
	vds = GDALOpen(vrast, GA_ReadOnly);
	if (vds == NULL) {
		fprintf(stderr, "Error. Unable to open value raster file '%s'\n", vrast);
		return 1;
	}
	
	// Get the input raster bands.
	fband = GDALGetRasterBand(fds, 1);
	sizex = GDALGetRasterBandXSize(fband);
	sizey = GDALGetRasterBandYSize(fband);
	vband = GDALGetRasterBand(vds, 1);
	
	// Read the input raster bands.
	fmatrix = (int*)malloc(sizex * sizey * sizeof(int));
	vmatrix = (double*)malloc(sizex * sizey * sizeof(double));
	if (fmatrix == NULL) {
		GDALClose(fds);
		GDALClose(vds);
		fprintf(stderr, "Error. Not enough memory to read input raster files.\n");
		return 1;
	}
	GDALRasterIO(fband, GF_Read, 0, 0, sizex, sizey, fmatrix, sizex, sizey, GDT_Int32, 0, 0);
	GDALRasterIO(vband, GF_Read, 0, 0, sizex, sizey, vmatrix, sizex, sizey, GDT_Float64, 0, 0);
	
	// Allocate the memory for the output raster.
	omatrix = calloc(sizex * sizey, sizeof(double));
	if (omatrix == NULL) {
		GDALClose(fds);
		GDALClose(vds);
		fprintf(stderr, "Error. Not enough memory for output raster.\n");
		return 1;
	}
	
	// Allocate the memory for the smoothness matrix.
	smatrix = calloc(sizex * sizey, sizeof(double));
	if (smatrix == NULL) {
		GDALClose(fds);
		GDALClose(vds);
		fprintf(stderr, "Error. Not enough memory for smoothness matrix.\n");
		return 1;
	}
	
	// Find the number of regions.
	nregions = findNumberOfRegions(fmatrix, sizex, sizey);
	
	// Compute the number of pixels for each region.
	pixelsPerRegion = calloc(nregions, sizeof(int));
	numberOfPixelsPerRegion(pixelsPerRegion, fmatrix, sizex, sizey);
	
	// Compute the density if needed.
	if (density == 1) {
		computeDensity(fmatrix, vmatrix, pixelsPerRegion, sizex, sizey);
	}
	
	// Compute the population totals for each region.
	pop = calloc(nregions, sizeof(double));
	pop_new = calloc(nregions, sizeof(double));
	computePopulationTotals(pop, fmatrix, vmatrix, sizex, sizey);
	
	
	
	for (itercnt = 0; itercnt < maxIterations; itercnt++) {
		
		if ((itercnt+1) % 100 == 0) {
			fprintf(stdout, "Computing pycnophylactic interpolation: iteration %i\n", itercnt+1);
		}
		
		// For each lattice point, compute the adjustment for smoothness
		computeAdjustmentForSmoothness(smatrix, vmatrix, fmatrix, sizex, sizey);
		// exportMatrixDouble(smatrix, sizex, sizey, "/Temp/smatrix.txt");
		
		// Compute a decrementing factor so that the average adjustment for each region is zero.
		applyDecrementingFactorPerRegion(smatrix, fmatrix, sizex, sizey, pixelsPerRegion, nregions);
	
		// Apply the adjustment for smoothness to the density values.
		matrixSum(vmatrix, smatrix, sizex, sizey, 0);
		
		// Compute the population totals for each region again.
		computePopulationTotals(pop_new, fmatrix, vmatrix, sizex, sizey);
		
		// Correct the population totals to respect the pycnophylactic constraint.
		enforcePycnophylacticConstraint(pop, pop_new, fmatrix, vmatrix, sizex, sizey, pixelsPerRegion, nregions);
		
	}
	
	
	// Writing out the resulting dataset.
	hDriver = GDALGetDatasetDriver(vds);
	ods = GDALCreateCopy(hDriver, orast, vds, FALSE, NULL, NULL, NULL);
	oband = GDALGetRasterBand(ods, 1);
	GDALRasterIO(oband, GF_Write, 0, 0, sizex, sizey, vmatrix, sizex, sizey, GDT_Float64, 0, 0);
	
	
	// Close the GDAL datasets.
	GDALClose(ods);
	GDALClose(fds);
	GDALClose(vds);
	
	fprintf(stdout, "\nPycnophylactic interpolation terminated.\n\n");
	
	return 0;
}




void exportMatrixDouble(double *matrix, int sizex, int sizey, char *fpath) {
	
	FILE *fp;
	int i,j;
	double *mPtr;
	
	fp = fopen(fpath, "w");
	mPtr = matrix;
	
	for (j = 0; j < sizey; j++) {
		for (i = 0; i < sizex; i++) {
			fprintf(fp, "%f", *mPtr);
			mPtr++;
			if (i < (sizex-1)) {
				fprintf(fp, " ");
			} else {
				fprintf(fp, "\n");
			}
		}
	}
	
	fclose(fp);
	
}



int findNumberOfRegions(int* fmatrix, int sizex, int sizey) {
	
	int *fptr;
	int fidmax = 0;
	int i;
	
	fptr = fmatrix;
	for (i=0; i < sizex*sizey; i++) {
		if (*fptr > fidmax) {
			fidmax = *fptr;
		}
		fptr++;
	}
	
	return fidmax;
}




void computeDensity(int *fmatrix, double *vmatrix, int *pixelsPerRegion, int sizex, int sizey) {
	
	int i;
	int *fptr;
	double *vptr;
	double v, p;
	int f;
	
	fptr = fmatrix;
	vptr = vmatrix;
	for (i=0; i < sizex*sizey; i++) {
		f = *fptr;
		p = (double)pixelsPerRegion[f];
		if (f > 0 && p > 0) {
			v = *vptr;
			*vptr /= p;
		}
		vptr++;
		fptr++;
	}
	
}



void computePopulationTotals(double *pop, int* fmatrix, double *vmatrix, int sizex, int sizey) {
	
	int *fptr;
	double *vptr;
	int i;
	
	fptr = fmatrix;
	vptr = vmatrix;
	for (i=0; i < sizex*sizey; i++) {
		if (*fptr > 0) {
			pop[*fptr] += *vmatrix;
		}
		fptr++;
		vptr++;
	}
	
}



void numberOfPixelsPerRegion(int *pixelsPerRegion, int *fmatrix, int sizex, int sizey) {
	
	int i;
	int *fptr;
	
	fptr = fmatrix;
	for (i=0; i < sizex*sizey; i++) {
		pixelsPerRegion[*fptr]++;
		fptr++;
	}
	
}


void computeAdjustmentForSmoothness(double *smatrix, double *vmatrix, int *fmatrix, int sizex, int sizey) {
	
	int i, j;
	int idx, idx2;
	double delta;
	double left, right, top, bottom, center;
	int fid;
	
	for (j = 0; j < sizey; j++) {
		for (i = 0; i < sizex; i++) {
			idx = j*sizex + i;
			center = vmatrix[idx];
			fid = fmatrix[idx];
			if (fid > 0) {
				// Make individual sum to check for boundary conditions at the same time.
				// Currently, only Dirichlet boundary condition with value 0 is implemented.
				left = right = top = bottom = 0.0;
				if (j < sizey-1) {
					idx2 = idx + sizex;
					bottom = vmatrix[idx2];
				}
				if (j > 0) {
					idx2 = idx - sizex;
					top = vmatrix[idx2];
				}
				if (i < sizex-1) {
					idx2 = idx + 1;
					right = vmatrix[idx2];
				}
				if (i > 0) {
					idx2 = idx - 1;
					left = vmatrix[idx2];
				}
				delta = 0.25 * (top + bottom + left + right) - center;
				// Underrelax
				smatrix[idx] = 0.25 * delta;
			}
		}
	}
	
}





void applyDecrementingFactorPerRegion(double *smatrix, int *fmatrix, int sizex, int sizey, int *pixelsPerRegion, int nregions) {
	
	double *adjsum;
	int i;
	double *sptr;
	int *fptr;
	
	// Compute the adjustment totals for every region.
	sptr = smatrix;
	fptr = fmatrix;
	adjsum = calloc(nregions, sizeof(double));
	for (i=0; i < sizex*sizey; i++) {
		adjsum[*fptr] += *sptr;
		sptr++;
		fptr++;
	}
	
	for (i=1; i < nregions; i++) {
		if (pixelsPerRegion[i] > 0) {
			adjsum[i] /= (double)pixelsPerRegion[i];
		} else {
			adjsum[i] = 0;
		}
	}
	
	// Correct the smoothing adjustment.
	sptr = smatrix;
	fptr = fmatrix;
	for (i=0; i < sizex*sizey; i++) {
		if (*fptr > 0) {
			*sptr -= adjsum[*fptr];
		}
		sptr++;
		fptr++;
	}
	
	free(adjsum);
	
}




void matrixSum(double *res, double *factor, int sizex, int sizey, double minvalue) {
	
	double *rptr, *fptr;
	int i;
	
	rptr = res;
	fptr = factor;
	for (i=0; i < sizex*sizey; i++) {
		if ((*rptr + *fptr) > minvalue) {
			*rptr += *fptr;
		} else {
			*rptr = minvalue;
		}
		rptr++;
		fptr++;
	}
	
}




void enforcePycnophylacticConstraint(double *pop, double *pop_new, int *fmatrix, double *vmatrix, int sizex, int sizey, int *pixelsPerRegion, int nregions) {
	
	int *fptr;
	double *vptr;
	int i;
	int *pixelsPerRegionAlreadyTreated;
	int remainingPixels;
	double correctedPop;
	
	
	// Allocate the memory for the pixels per region already treated.
	pixelsPerRegionAlreadyTreated = calloc(nregions, sizeof(int));
	
	// Compute the population difference
	for (i=1; i < nregions; i++) {
		correctedPop = pop[i] - pop_new[i];
		pop_new[i] = correctedPop;
	}
	
	fptr = fmatrix;
	vptr = vmatrix;
	for (i=0; i < sizex*sizey; i++) {
		if (*fptr > 0 && pop_new[*fptr] != 0) {
			remainingPixels = pixelsPerRegion[*fptr] - pixelsPerRegionAlreadyTreated[*fptr];
			if (remainingPixels > 0) {
				correctedPop = *vptr + (pop_new[*fptr] / (double)remainingPixels);
			} else {
				correctedPop = 0;
			}
			if (*vptr < correctedPop) {
				correctedPop = *vptr;
			}
			*vptr = correctedPop;
			pop_new[*fptr] -= correctedPop;
			pixelsPerRegionAlreadyTreated[*fptr]++;
		}
		fptr++;
		vptr++;
	}
	
	free(pixelsPerRegionAlreadyTreated);
	
}










