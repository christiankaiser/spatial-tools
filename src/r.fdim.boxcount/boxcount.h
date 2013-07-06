/*
 *  boxcount.h
 *  r.fdim.boxcount
 *
 *  Created by Christian Kaiser on 06.06.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


#if !defined(ST_BOXCOUNT_DEF)
#define ST_BOXCOUNT_DEF 1

typedef struct {
	double boxsize;
	int nboxes;
} st_boxcount;

#endif






int boxcount (char *input_raster, int band, int minbox, int maxbox, char *vdom, char *output_plot);




void computeBoxcount(double *input_data, int rasterX, int rasterY, 
					 int *vdom_data, int minbox, int maxbox, char *plot_file);



st_boxcount numberOfCellsForNumberOfBoxes (int nboxes, double *input_data, int rasterX, int rasterY);

double estimateFDim (st_boxcount *cnts, int ncnts);


int countNonZeroValues (double *raster, int sizex, int sizey);


void randomRasterInsideVDom(int random_cells, double *random_data, int *vdom_data, int rasterX, int rasterY);



// Creates the SVG plot for the fractal dimension.
void plot_fdim (char *plot_file, 
				st_boxcount* ncells, int minbox, int maxbox, 
				st_boxcount *ncells_vdom, 
				double fdim, double fdim_vdom);







