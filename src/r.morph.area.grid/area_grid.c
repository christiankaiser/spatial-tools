/*
 *  area_grid.c
 *  r.morph.area.grid
 *
 *  Created by Christian Kaiser on 13.02.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#include <GDAL/GDAL.h>

#include "area_grid.h"
#include "proj.h"





int area_grid (char *infile, char *outfile, char *format) {

	morph				*mgrid;
	GDALDriverH			out_driver;
	GDALDatasetH		out_ds;
	GDALRasterBandH		out_band;
	double				*rast;
	double				ax, ay, bx, by, cx, cy, dx, dy;
	double				s1, s2;
	int					i, j, idx;
	
	
	GDALAllRegister();
	
	// Read the morph grid file.
	mgrid = read_grid(infile);
	if (mgrid == NULL) {
		fprintf(stderr, "Error. Unable to read input morph file '%s'.\n", infile);
		exit(1);
	}
	
	// Create a new raster file
	out_driver = GDALGetDriverByName(format);
	if (out_driver == NULL) {
		fprintf(stderr, "Warning. Unable to find driver for GDAL format '%s'. Using HFA format instead.\n", format);
		out_driver = GDALGetDriverByName("HFA");
	}
	out_ds = GDALCreate(out_driver, outfile, mgrid->xsize, mgrid->ysize, 1, GDT_Float64, NULL);
	GDALSetGeoTransform(out_ds, mgrid->georef);
	
	// Allocate the memory for holding the raster data.
	rast = calloc(mgrid->xsize * mgrid->ysize, sizeof(double));
	if (rast == NULL) {
		fprintf(stderr, "Error. Not enough memory.\n");
		exit(1);
	}

	// Compute the area for each morph grid cell size
	for (j = 0; j < mgrid->ysize; j++) {
		for (i = 0; i < mgrid->xsize; i++) {
			
			idx = j*mgrid->xsize + i;
			ax = mgrid->x[i][j];
			ay = mgrid->y[i][j];
			bx = mgrid->x[i+1][j];
			by = mgrid->y[i+1][j];
			cx = mgrid->x[i][j+1];
			cy = mgrid->y[i][j+1];
			dx = mgrid->x[i+1][j+1];
			dy = mgrid->y[i+1][j+1];
			
			s1 = 0.5 * ABS((dx-ax)*(by-ay) - (dy-ay)*(bx-ax));
			s2 = 0.5 * ABS((dx-cx)*(by-cy) - (dy-cy)*(bx-cx));
			
			rast[idx] = s1 + s2;
		}
	}
	
	// Write the raster data
	out_band = GDALGetRasterBand(out_ds, 1);
	GDALRasterIO(out_band, GF_Write, 0, 0, mgrid->xsize, mgrid->ysize, rast, 
				 mgrid->xsize, mgrid->ysize, GDT_Float64, 0, 0);
	free(rast);
	GDALClose(out_ds);
	
	free_morph(mgrid);
	
	return 0;
}









