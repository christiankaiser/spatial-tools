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

#include <GDAL/ogr_api.h>





/*
 * Structure for storing a morphing grid.
 */
typedef struct {
	int xsize;
	int ysize;
	double georef[6];
	double **x;
	double **y;
} morph;




/*
 * Allocates the memory for the morphing grid.
 * Use free_morph to free the memory again.
 */
morph *create_morph(int xsize, int ysize);

/*
 * Frees the memory for a morphing grid.
 */
void free_morph(morph* mg);





/*
 * point_geo_to_pixel
 * Translates a geographic point to a pixel coordinate using
 * the provided georeference.
 * Returns 0 in case of success, 1 otherwise.
 */
int point_geo_to_pixel (double xgeo, double ygeo, double *pixelx, double *pixely, double georef[6]);




/*
 * point_pixel_to_geo
 * Translates a pixel coordinate to a geographic coordinate using
 * the provided georeference.
 * Returns 0 in case of success, 1 otherwise.
 */
int point_pixel_to_geo (double pixelx, double pixely, double *geox, double *geoy, double georef[6]);






/*
 * read_grid_points
 * Function to read in the morphing grid points.
 * Uses column-major, so the loops are an odd way around.
 * Returns 0 if successful; 1 if the file ended early.
 */
int read_grid_points (FILE *stream, 
					  double **gridx, double **gridy, 
					  int xsize, int ysize);




/*
 * read_grid_georef
 * Reads the georeference from a morphing grid file.
 * @param stream			The pointer to the open file.
 * @param geoTransform		An array of 6 doubles for storing the georeference.
 * Returns 0 if successful; 1 if the file ended early.
 */
int read_grid_georef(FILE *stream, double *geoTransform);




/*
 * read_grid
 * Reads a whole grid file and allocates the memory for holding the grid points.
 * Returns a pointer to morphing grid sturcture, or NULL on failure.
 */
//morph* read_grid (char *file, double **gridx, double **gridy, int *xsize, int *ysize, double *georef);
morph* read_grid (char *file);



/* project_coord
 * Projects a single coordinate using the provided morphing grid.
 * @param xgeo, ygeo	Coordinates to project
 * @param *xout, *yout	Projected coordinates
 * Returns 0 if successful, 1 if there was a problem.
 */
int project_coord (double xgeo, double ygeo, 
				   double *xout, double *yout,
				   double **gridx, double **gridy, int xsize, int ysize,
				   double geoTransform[6]);




/* project_back_coord
 * Projects a single coordinate from the embedded space to the original space
 * using the provided morphing grid.
 * @param xembedded, yembedded		Coordinates in the embedded space
 * @param xorig, yorig				Coordinates in the original space
 * Returns 0 if successful, 1 if a problem occured.
 */
int project_back_coord (double xembedded, double yembedded, double *xorig, double *yorig, morph *grid);




/* 
 * scale_coord
 * Scales a coordinate point given by x/y by the given scale and using the provided centre.
 * Returns 0 if successful, 1 if there was a problem.
 */
int scale_coord (double x, double y, double scale, double centrex, double centrey, double *xscaled, double *yscaled);




/*
 * project_wkt
 * Projects a WKT geometry using the provided morphping grid.
 */
int project_wkt (char *wktin, char *wktout, int maxlen_wkt, 
				 double **gridx, double **gridy, int xsize, int ysize,
				 double geoTransform[6], 
				 double scale, double centrex, double centrey);

				 


/*
 * project_geom
 * Projects a OGR geometry using the provided morphing grid.
 */
int project_geom(OGRGeometryH geom, morph *grid, int back_transform);





/*
 * grid_cell_for_coordinate
 * Finds the grid cell in which the provided coordinate lies.
 * It provides the values i and j; in this case the coordinate lies inside the polygon
 * defined by the points i,j / i+1,j / i+1,j+1 / i,j+1
 * It return 0 in case of success, 1 in case of an error.
 */
int grid_cell_for_coordinate (double coordx, double coordy, morph *mg, int *cellx, int *celly);





/*
 * point_in_polygon
 * Tests if point ptx,pty is inside the polygon defined by the points xpoly,ypoly.
 * The polygon has npts points, the polygon should be closed (last point same as first).
 * Returns 1 if the point is inside the polygon, 0 if it is not, and -1 if an error occured.
 */
int point_in_polygon(double ptx, double pty, double *xpoly, double *ypoly, int npts);







