/*
 *  transform.h
 *  r.morph.transform
 *
 *  Created by Christian Kaiser on 14.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */



/*
 * Allows transforming coordinates read from standard input.
 * Writes the result back to standard out.
 */
int coord_transform(char *morphfile, int back_transform);


/*
 * Allows transform of a GDAL compatible raster image.
 */
int raster_transform(char *infile, char *morphfile, char *outfile, char *format, int back_transform);


/*
 * Allows transform of a OGR compatbile vector layer.
 */
int vector_transform(char *infile, char *input_layer, char *spatial_filter, char *attr_filter,
					 char *morphfile, 
					 char *outfile, char *attrs, char *format, char *output_layer, 
					 int back_transform);
