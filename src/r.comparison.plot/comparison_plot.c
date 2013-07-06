/*
 *  comparison_plot.c
 *  r.comparison.plot
 *
 *  Created by Christian on 22.10.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "comparison_plot.h"
#include "raster.h"



#include <stdio.h>



int raster_comparison_plot (char *raster1, int band1, double null1, 
							char *raster2, int band2, double null2, 
							char *output_file)
{
	
	FILE *out;								// Output file point.
	GDALDatasetH dataset1, dataset2;		// The GDAL datasets for the input raster files.
	int r1x, r1y, r2x, r2y;					// The size of the two input rasters.
	double georef1[6], georef2[6];			// The georeference of the two input rasters.
	double bbox1[4], bbox2[4];				// The bounding boxes of the two input rasters.
	double *data1, *data2;					// The input data arrays.
	double *data1Ptr, *data2Ptr;			// Pointers to the input data arrays.
	int ok, i, j;
	int sameExtent;							// 1 if the two input rasters have the same extent and georeference.
	

	
	
	
	// Read the two input raster files.
	
	ok = raster_band_read_double(raster1, band1, &data1, &r1x, &r1y);
	if (ok != 0)
	{
		fprintf(stderr, "ERROR. Unable to read input raster 1 file.\n");
		return 1;
	}
	
	ok = raster_band_read_double(raster2, band2, &data2, &r2x, &r2y);
	if (ok != 0)
	{
		fprintf(stderr, "ERROR. Unable to read input raster 2 file.\n");
		return 1;
	}
	
	
	
	// Get the georeference and the bounding boxes of the input rasters.
	dataset1 = GDALOpen(raster1, GA_ReadOnly);
	GDALGetGeoTransform(dataset1, georef1);
	raster_bbox(dataset1, bbox1);
	GDALClose(dataset1);
	
	dataset2 = GDALOpen(raster2, GA_ReadOnly);
	GDALGetGeoTransform(dataset2, georef2);
	raster_bbox(dataset2, bbox2);
	GDALClose(dataset2);
	
	
	// Check whether both input raster files have the same extent and georeference or not.
	sameExtent = 1;
	for (i = 0; i < 6; i++)
	{
		if (georef1[i] != georef2[i])
		{
			sameExtent = 0;
			i = 6;
		}
	}
	for (i = 0; i < 4; i++)
	{
		if (bbox1[i] != bbox2[i])
		{
			sameExtent = 0;
			i = 4;
		}
	}
	
	
	
	// Open the output file.
	
	if (output_file == NULL)
		out = stdout;
	else
		out = fopen(output_file, "w");
	
	if (out == NULL)
	{
		fprintf(stderr, "Error. Unable to create output file.");
		return 1;
	}
	
	
	
	
	if (sameExtent)
	{
		fprintf(out, "raster1\traster2\n");
		data1Ptr = data1;
		data2Ptr = data2;
		for (j = 0; j < r1y; j++)
		{
			for (i = 0; i < r1x; i++)
			{
				if (*data1Ptr != null1 || *data2Ptr != null2)
					fprintf(out, "%f\t%f\n", *data1Ptr, *data2Ptr);
				data1Ptr++;
				data2Ptr++;
			}
		}
	}
	else
	{
		fprintf(stderr, "Error. The two input raster files must have the same extent and resolution.");
		return 1;
	}
	
	
	
	
	
	// Close the output file.
	if (output_file != NULL) fclose(out);
	
	
	return 0;
}