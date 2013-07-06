/* 
 * interp
 * This program allows projecting coordinates using a morphing grid.
 * It takes a morphing grid file as produced by the cartogram density equalization
 * program, and a simple text file with one pair of coordinates per line,
 * and writes a new text file with the projected coordinates.
 *
 * This program is based on the program written by Mark Newman. It has been
 * adapted by Christian Kaiser for the ScapeToad project. It contains new code
 * mainly for using georeferenced files.
 *
 * See http://www.umich.edu/~mejn/ for further details on the work done by Mark Newman.
 * See http://scapetoad.choros.ch for further details on the ScapeToad project.
 */




#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "proj.h"


#define LINELENGTH 1000

#ifndef MAX
#  define MIN(a,b)      ((a<b) ? a : b)
#  define MAX(a,b)      ((a>b) ? a : b)
#endif




morph *create_morph(int xsize, int ysize)
{
	morph *mg;
	int i;
	
	
	if (xsize <= 0 || ysize <= 0) {
		fprintf(stderr, "[ScapeToadPy] Error. Morphing grid size can not be less than 1x1.\n");
		return NULL;
	}
	
	mg = malloc(sizeof(morph));
	
	mg->xsize = xsize;
	mg->ysize = ysize;
	
	mg->x = malloc((xsize+1)*sizeof(double*));
	mg->y = malloc((xsize+1)*sizeof(double*));
	if (mg->x == NULL || mg->y == NULL) {
		fprintf(stderr, "[ScapeToadPy] Error. Unable to allocate memory for morphing grid.\n");
		return NULL;
	}
	
	for (i = 0; i <= xsize; i++) {
		mg->x[i] = malloc((ysize+1)*sizeof(double));
		mg->y[i] = malloc((ysize+1)*sizeof(double));
		if (mg->x[i] == NULL || mg->y[i] == NULL) {
			fprintf(stderr, "[ScapeToadPy] Error. Unable to allocate memory for morphing grid.\n");
			return NULL;
		}
	}
	
	return mg;
}




void free_morph(morph* mg)
{
	int i;
	
	for (i = 0; i <= mg->xsize; i++) {
		free(mg->x[i]);
		free(mg->y[i]);
	}
	
	free(mg->x);
	free(mg->y);
	
	free(mg);
}






int point_geo_to_pixel (double xgeo, double ygeo, double *pixelx, double *pixely, double georef[6]) {
	
	double topleftx, toplefty, weres, nsres, rot1, rot2;
	
	
	topleftx = georef[0];
	weres = georef[1];
	rot1 = georef[2];
	toplefty = georef[3];
	rot2 = georef[4];
	nsres = georef[5];
	
	if (nsres*weres == 0) {
		return 1;
	}
	
	// Transform the point from geographic coordinates to the grid space.
	// The base formula are the following:
	//		xgeo = topleftx + xpixel*weres + ypixel*rot1
	//		ygeo = toplefty + xpixel*rot2 + ypixel*nsres
	*pixelx = (ygeo*rot1 - toplefty*rot1 - xgeo*nsres + topleftx*nsres) / (rot1*rot2 - nsres*weres);
	*pixely = (ygeo*weres - toplefty*weres - xgeo*rot2 + topleftx*rot2) / (weres*nsres - rot1*rot2);
	
	return 0;
}





int point_pixel_to_geo (double pixelx, double pixely, double *geox, double *geoy, double georef[6]) {
	
	double topleftx, toplefty, weres, nsres, rot1, rot2;
	
	topleftx = georef[0];
	weres = georef[1];
	rot1 = georef[2];
	toplefty = georef[3];
	rot2 = georef[4];
	nsres = georef[5];
	
	*geox = topleftx + pixelx*weres + pixely*rot1;
	*geoy = toplefty + pixelx*rot2 + pixely*nsres;
	
	return 0;
}





int read_grid_points (FILE *stream, 
					  double **gridx, double **gridy, 
					  int xsize, int ysize)
{

	int ix, iy;

	for (iy = 0; iy <= ysize; iy++) {
		for (ix = 0; ix <= xsize; ix++) {
			if (fscanf(stream,"%lf %lf\n", &gridx[ix][iy], &gridy[ix][iy]) < 2) {
				return 1;
			}
		}
	}

	/*int i;
	for (i = 0; i < (xsize+1)*(ysize+1); i++) {
		if (fscanf(stream,"%lf %lf\n", gridx[i], gridy[i]) < 2) {
			return 1;
		}
	}*/

	return 0;
	
}





int read_grid_georef (FILE *stream, double *geoTransform)
{
	char line[LINELENGTH];
	char *l;
	
	l = fgets(line, LINELENGTH, stream);	// top left x
	geoTransform[0] = atof(&line[9]);
	
	l = fgets(line, LINELENGTH, stream);	// w-e resolution
	geoTransform[1] = atof(&line[6]);
	
	l = fgets(line, LINELENGTH, stream);	// rotation 1
	geoTransform[2] = atof(&line[5]);
	
	l = fgets(line, LINELENGTH, stream);	// top left y
	geoTransform[3] = atof(&line[9]);
	
	l = fgets(line, LINELENGTH, stream);	// rotation 2
	geoTransform[4] = atof(&line[5]);
	
	l = fgets(line, LINELENGTH, stream);	// n-s resolution
	geoTransform[5] = atof(&line[6]);
	
	return 0;
}





morph* read_grid (char *file)
{
	FILE *fp;
	char line[10240];
	int xsize, ysize;
	morph *mg;
	char *l;
	int ok;
	
	
	// Open the morphing grid file
	fp = fopen(file, "r");
	if (fp == NULL) {
		fprintf(stderr,"[ScapeToadPy] Uunable to open morphing grid file `%s'\n", file);
		return NULL;
	}
	
	
	// Read the morphing grid size.
	l = fgets(line, 10240, fp);			// File version information
	l = fgets(line, 10240, fp);			// xsize
	xsize = atoi(&line[6]);
	l = fgets(line, 10240, fp);			// ysize
	ysize = atoi(&line[6]);
	if (xsize <= 0 || ysize <= 0) {
		fprintf(stderr, "Error. Morphing grid size invalid.\n");
		return NULL;
	}
	
	// Create the morphing grid structure.
	mg = create_morph(xsize, ysize);
	if (mg == NULL) {
		return NULL;
	}

	// Read the georeference
	ok = read_grid_georef(fp, &mg->georef[0]);
	if (ok != 0) {
		fprintf(stderr, "Error. Unable to read the morphing grid georeference.\n");
		return NULL;
	}
	
	// Read in the grid of points
	ok = read_grid_points(fp, mg->x, mg->y, mg->xsize, mg->ysize);
	if (ok != 0) {
		fprintf(stderr, "Error. Unable to read the morphing grid points.\n");
		return NULL;
	}
	
	// Close the grid file
	fclose(fp);
	
	return mg;
}











int project_coord (double xgeo, double ygeo, 
				   double *xout, double *yout,
				   double **gridx, double **gridy, int xsize, int ysize,
				   double geoTransform[6])
{
	
	int ix,iy;
	double xin,yin;			// Grid coordinates
	double dx, dy, x, y;
	double topleftx, toplefty, weres, nsres, rot1, rot2;	// Georeference
	int ok;
	
	
	topleftx = geoTransform[0];
	weres = geoTransform[1];
	rot1 = geoTransform[2];
	toplefty = geoTransform[3];
	rot2 = geoTransform[4];
	nsres = geoTransform[5];
	
	

	// Transform the point from geographic coordinates to the grid space.
	ok = point_geo_to_pixel(xgeo, ygeo, &xin, &yin, geoTransform);
	if (ok != 0) {
		return 1;
	}
		
		
		
	// Check if we are outside bounds
	if ((xin < 0.0) || (xin >= xsize) || (yin < 0.0) || (yin >= ysize)) {
		*xout = xgeo;
		*yout = ygeo;
	} else {
			
		// Bilinear interpolation
		ix = xin;
		dx = xin - ix;
		iy = yin;
		dy = yin - iy;
		x = (1-dx)*(1-dy)*gridx[ix][iy] + dx*(1-dy)*gridx[ix+1][iy] + (1-dx)*dy*gridx[ix][iy+1] + dx*dy*gridx[ix+1][iy+1];
		y = (1-dx)*(1-dy)*gridy[ix][iy] + dx*(1-dy)*gridy[ix+1][iy] + (1-dx)*dy*gridy[ix][iy+1] + dx*dy*gridy[ix+1][iy+1];
		
		// Transform back to geographic coordinates
		ok = point_pixel_to_geo(x, y, xout, yout, geoTransform);
		if (ok != 0) {
			return 1;
		}
		
	}
	
	
	return 0;
}






int project_back_coord (double xembedded, double yembedded, double *xorig, double *yorig, morph *grid) {
	
	double i, j, i1, i2, j1, j2;
	int cellx, celly;
	int ok;
	double ax, ay, bx, by, cx, cy, dx, dy, px, py;
	
	
	// Transform the point from geographic coordinates to the grid space.
	ok = point_geo_to_pixel(xembedded, yembedded, &px, &py, grid->georef);
	if (ok != 0) {
		return 1;
	}
	
	// Get the grid cell in which the provided points lies.
	// If the point lies outside the morphing grid, we return the input coordinates.
	ok = grid_cell_for_coordinate(px, py, grid, &cellx, &celly);
	if (ok != 0) {
		*xorig = xembedded;
		*yorig = yembedded;
		return 0;
	}
	
	// Compute the i and j for the bilinear interpolation.
	ax = grid->x[cellx][celly];
	ay = grid->y[cellx][celly];
	bx = grid->x[cellx+1][celly];
	by = grid->y[cellx+1][celly];
	cx = grid->x[cellx+1][celly+1];
	cy = grid->y[cellx+1][celly+1];
	dx = grid->x[cellx][celly+1];
	dy = grid->y[cellx][celly+1];
	i1 = (ay*bx - ax*by - ay*cx + ax*cy + ay*dx - ax*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py - (ay*bx*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py - sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py,2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) + (ax*by*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py - sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) *  (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) + (ay*cx*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py - sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay *px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) - (ax*cy*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py - sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) - (by*dx*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py - sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) + (cy*dx*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py - sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) + (bx*dy*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py - sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) - (cx*dy*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py - sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy))) / (-ay*cx + by*cx + ax*cy - bx*cy + ay*dx - by*dx - ax*dy + bx*dy);
	j1 = (2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py - sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy));
	i2 = (ay*bx - ax*by - ay*cx + ax*cy + ay*dx - ax*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py - (ay*bx*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py + sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) + (ax*by*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py + sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) + (ay*cx*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py + sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) - (ax*cy*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py + sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) - (by*dx*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py + sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) + (cy*dx*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py + sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) + (bx*dy*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py + sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy)) - (cx*dy*(2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py + sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2)))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy))) / (-ay*cx + by*cx + ax*cy - bx*cy + ay*dx - by*dx - ax*dy + bx*dy);
	j2 = (2*ay*bx - 2*ax*by - ay*cx + ax*cy + by*dx - bx*dy - ay*px + by*px - cy*px + dy*px + ax*py - bx*py + cx*py - dx*py + sqrt(-4*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy) * (ay*bx - ax*by - ay*px + by*px + ax*py - bx*py) + pow(-2*ay*bx + 2*ax*by + ay*cx - ax*cy - by*dx + bx*dy + ay*px - by*px + cy*px - dy*px - ax*py + bx*py - cx*py + dx*py, 2))) / (2*(ay*bx - ax*by - ay*cx + ax*cy + by*dx - cy*dx - bx*dy + cx*dy));
	
	if (i1 >= 0.0 && i1 <= 1.0) {
		i = i1;
	} else if (i2 >= 0.0 && i2 <= 1.0) {
		i = i2;
	} else {
		fprintf(stderr, "[ScapeToadPy] Error. Cannot make back projection, value i invalid.\n");
	}
	
	if (j1 >= 0.0 && j1 <= 1.0) {
		j = j1;
	} else if (j2 >= 0.0 && j2 <= 1.0) {
		j = j2;
	} else {
		fprintf(stderr, "[ScapeToadPy] Error. Cannot make back projection, value j invalid.\n");
	}
	
	
	px = (double)cellx + i;
	py = (double)celly + j;
	point_pixel_to_geo(px, py, xorig, yorig, grid->georef);
	
	return 0;
}





int scale_coord (double x, double y, double scale, double centrex, double centrey, double *xscaled, double *yscaled) {
	
	*xscaled = ((x - centrex) * scale) + centrex;
	*yscaled = ((y - centrey) * scale) + centrey;
	
	return 0;
	
}






int project_wkt (char *wktin, char *wktout, int maxlen_wkt, 
				 double **gridx, double **gridy, int xsize, int ysize,
				 double geoTransform[6], 
				 double scale, double centrex, double centrey)
{
	
	int charcnt, i;
	char *inptr, *outptr;
	char incoordx[LINELENGTH], incoordy[LINELENGTH], outcoord[LINELENGTH];
	char *incoordxptr, *incoordyptr, *outcoordptr;
	double xin, yin, xout, yout, xscaled, yscaled;
	
	
	// Go to the beginning of the coordinates.
	inptr = wktin;
	outptr = wktout;
	charcnt = 0;
	while ((*inptr < 45 || *inptr > 57) && *inptr > 0 && charcnt < maxlen_wkt) {
		*outptr = *inptr;
		outptr++;
		inptr++;
		charcnt++;
	}
	
	// We are now at the beginning of the first coordinate.
	// Go through the whole rest of the string.
	while (*inptr > 0 && charcnt < maxlen_wkt) {
		
		// Get the x coordinate
		i = 0;
		incoordxptr = incoordx;
		while (*inptr >= 45 && *inptr <= 57 && *inptr > 0 && charcnt < maxlen_wkt && i < LINELENGTH) {
			*incoordxptr = *inptr;
			incoordxptr++;
			inptr++;
			i++;
		}
		*incoordxptr = 0;
		
		// Go to the beginning of the y coordinate.
		while ((*inptr < 45 || *inptr > 57) && *inptr > 0 && charcnt < maxlen_wkt) {
			inptr++;
		}
		
		// Get the y coordinate
		i = 0;
		incoordyptr = incoordy;
		while (*inptr >= 45 && *inptr <= 57 && *inptr > 0 && charcnt < maxlen_wkt && i < LINELENGTH) {
			*incoordyptr = *inptr;
			incoordyptr++;
			inptr++;
			i++;
		}
		*incoordyptr = 0;
		
		// Convert the coordinate into double values, and project them 
		// using the provided morphing grid.
		xin = atof(incoordx);
		yin = atof(incoordy);
		
		project_coord(xin, yin, &xout, &yout, gridx, gridy, xsize, ysize, geoTransform);
		scale_coord(xout, yout, scale, centrex, centrey, &xscaled, &yscaled);
		
		// Convert the coordinates back to a string
		sprintf(outcoord, "%0.14f %0.14f", xscaled, yscaled);
		
		// Copy the projected coordinate to the WKT string.
		outcoordptr = outcoord;
		while (*outcoordptr > 0 && charcnt < maxlen_wkt) {
			*outptr = *outcoordptr;
			outptr++;
			charcnt++;
			outcoordptr++;
		}
		
		// Go to the beginning of the next coordinate, or the end of the string.
		while ((*inptr < 45 || *inptr > 57) && *inptr > 0 && charcnt < maxlen_wkt) {
			*outptr = *inptr;
			outptr++;
			inptr++;
			charcnt++;
		}
		
	}
	
	*outptr = 0;
	
	return 0;
}






int project_geom(OGRGeometryH geom, morph *grid, int back_transform) {
	
	int i;
	double x, y, z, x_proj, y_proj;
	
	// If the geometry is not directly projectable, call recursively
	// for each composing geometry.
	if (OGR_G_GetGeometryCount(geom) > 0) {
		for (i = 0; i < OGR_G_GetGeometryCount(geom); i++) {
			project_geom(OGR_G_GetGeometryRef(geom, i), grid, back_transform);
		}
		return 0;
	}
	
	// Project every point separately.
	for (i = 0; i < OGR_G_GetPointCount(geom); i++) {
		OGR_G_GetPoint(geom, i, &x, &y, &z);
		if (back_transform) {
			project_back_coord(x, y, &x_proj, &y_proj, grid);
		} else {
			project_coord(x, y, &x_proj, &y_proj, grid->x, grid->y, grid->xsize, grid->ysize, grid->georef);
		}
		OGR_G_SetPoint(geom, i, x_proj, y_proj, z);
	}
	
	return 0;
}





int grid_cell_for_coordinate (double coordx, double coordy, morph *mg, int *cellx, int *celly) {
	
	int i, j;
	double xpoly[5], ypoly[5];
	double xmin, xmax, ymin, ymax;
	
	// We need to find the 4 coordinates of the cell.
	for (j = 0; j < mg->ysize; j++) {
		for (i = 0; i < mg->xsize; i++) {
			
			xmin = MIN(mg->x[i][j], mg->x[i][j+1]);
			xmax = MAX(mg->x[i+1][j], mg->x[i+1][j+1]);
			ymin = MIN(mg->y[i][j], mg->y[i+1][j]);
			ymax = MAX(mg->y[i][j+1], mg->y[i+1][j+1]);
			
			//fprintf(stderr, "%f %f / %f %f\n", xmin, ymin, xmax, ymax);

			
			if (xmin <= coordx && xmax >= coordx &&
				ymin <= coordy && ymax >= coordy)
			{
				xpoly[0]=mg->x[i][j]; xpoly[1]=mg->x[i][j+1]; xpoly[2]=mg->x[i+1][j+1]; 
				xpoly[3]=mg->x[i+1][j]; xpoly[4]=mg->x[i][j];
				
				ypoly[0]=mg->y[i][j]; ypoly[1]=mg->y[i][j+1]; ypoly[2]=mg->y[i+1][j+1]; 
				ypoly[3]=mg->y[i+1][j]; ypoly[4]=mg->y[i][j];
				
				if (point_in_polygon(coordx, coordy, xpoly, ypoly, 5) == 1) {
					*cellx = i;
					*celly = j;
					return 0;
				}
			}
			
		}
	}
	
	// We were not able to find the coordinate.
	return 1;
}






int point_in_polygon(double ptx, double pty, double *xpoly, double *ypoly, int npts)
{
	
	int leftCrossings, rightCrossings, upperCrossings, lowerCrossings;
	int i;
	double crossing;
	
	
	
	// Count the number of intersections of the x-axis through the point
	// with the polygon. Distinguish between left and right crossings.
	// Loop through all points for finding these crossings.
	leftCrossings = 0;
	rightCrossings = 0;
	for (i = 0; i <= (npts-2); i++)
	{
		// Crossing or not?
		if ((ypoly[i] < pty && ypoly[i+1] >= pty) || (ypoly[i] > pty && ypoly[i+1] <= pty)) {
			// Crossing of the x-axis through the point.
			// Compute the intersection point.
			// Add 1 to left or right crossing.
			crossing = xpoly[i] + ((pty - ypoly[i]) * (xpoly[i+1] - xpoly[i]) / (ypoly[i+1] - ypoly[i]));
			
			// Right crossing or left crossing?
			if (crossing > ptx) {
				rightCrossings++;
			} else {
				leftCrossings++;
			}
		}
	}
	
	// If the number of right crossings and left crossing are
	// not an odd number, the point is outside.
	// Continue the check otherwise.
	if (rightCrossings % 2 == 0 || leftCrossings % 2 == 0) {
		return 0;
	}
	
	
	// Count the number of intersections of the y-axis through the point
	// with the polygon. Distinguish between left and right crossings.
	// Loop through all points for finding these crossings.
	upperCrossings = 0;
	lowerCrossings = 0;
	for (i = 0; i <= (npts-2); i++)
	{
		// Crossing or not?
		if ((xpoly[i] < ptx && xpoly[i+1] > ptx) || (xpoly[i] > ptx && xpoly[i+1] < ptx)) {
			// Crossing of the y-axis through the point.
			// Compute the intersection point.
			// Add 1 to left or right crossing.
			crossing = ypoly[i] + ((ptx - xpoly[i]) * (ypoly[i+1] - ypoly[i+1]) / (xpoly[i+1] - xpoly[i]));
			// Right crossing or left crossing?
			if (crossing > pty) {
				upperCrossings++;
			} else {
				lowerCrossings++;
			}
		}
	}
	
	// If the number of upper crossings and lower crossing are
	// not an odd number, the point is outside.
	if (upperCrossings % 2 == 0 || lowerCrossings % 2 == 0) {
		return 0;
	} else {
		return 1;
	}
	
	
	return -1;
}







