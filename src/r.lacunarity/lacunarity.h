/*
 *  lacunarity.h
 *  r.lacunarity
 *
 *  Created by Christian on 29.06.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */





int lacunarity (char *input_raster, int band, 
				int binary, long binaryThreshold, int f3d,
				int gbox_min, int gbox_max, int gbox_step);


int spatial_lacunarity (char *input_raster, int band, 
						int binary, long binaryThreshold, int f3d,
						int gbox, int mwin, 
						char *output_file, char *format);



/**
 * Computes the lacunarity index inside a given window, for a given
 * gliding box size.
 */
double lacunarity_in_window (long *data, int rasterX, int rasterY, 
							 int f3d,
							 int gbox, 
							 int mwinX, int mwinY, int mwinW, int mwinH);