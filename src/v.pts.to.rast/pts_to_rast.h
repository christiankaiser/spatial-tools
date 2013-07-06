/*
 *  pts_to_rast.h
 *  v.pts.to.rast
 *
 *  Created by Christian Kaiser on 11.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


int pts_to_rast(char *input_file, char *layer, char *attr, char *stat,
				char *output_file, double resolution, 
				double xmin, double xmax, double ymin, double ymax, 
				int nullValue,
				char *format, char *raster_type,
				char *filter);