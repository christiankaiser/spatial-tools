/*
 *  pts_to_rast.c
 *  v.pts.to.rast
 *
 *  Created by Christian Kaiser on 11.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */



#include <math.h>
#include <stdlib.h>
#include <omp.h>

#include <GDAL/gdal.h>
#include <GDAL/ogr_api.h>

#include "pts_to_rast.h"



int pts_to_rast(char *input_file, char *layer, char *attr, char *stat,
				char *output_file, double resolution, 
				double xmin, double xmax, double ymin, double ymax, 
				int nullValue,
				char *format, char *raster_type,
				char* filter)
{
	
	OGRDataSourceH		in_ds;
	OGRLayerH			in_lyr;
	OGRFeatureH			in_feat;
	OGRGeometryH		in_geom;
	
	GDALDriverH			out_driver;
	GDALDatasetH		out_ds;
	GDALRasterBandH		out_band;
	GDALDataType		out_type;
	
	int					sizex, sizey;			// The size of the output raster.
	double				georef[6];				// The georeference for the output raster.
	int					*dataInt, *dataCnt;		// The output raster data matrix, and a count matrix.
	double				*dataDbl;				// The double version of the output raster data matrix.
	
	double				x, y;					// Real-world coordinates
	int					i, j;					// Pixel coordinates
	int					fidx;					// Field index
	
	int					cnt;
	
	
	
	GDALAllRegister();
	OGRRegisterAll();
	
	// Open the input vector file (OGR-compatible)
	in_ds = OGROpen(input_file, FALSE, NULL);
	if (in_ds == NULL) {
		fprintf(stderr, "Error. Unable to open the input vector datasource.\n");
		exit(1);
	}
	
	// Get the named layer if we have one, and the first layer otherwise.
	if (layer != NULL) {
		in_lyr = OGR_DS_GetLayerByName(in_ds, layer);
	} else {
		in_lyr = OGR_DS_GetLayer(in_ds, 0);
	}
	if (in_lyr == NULL) {
		fprintf(stderr, "Error. Unable to read the input vector layer.\n");
		exit(1);
	}
	
	// Apply an attribute filter if required.
	if (filter != NULL) {
		OGR_L_SetAttributeFilter(in_lyr, filter);
	}
	
	// If the spatial extent is not given using the xmin, xmax, ymin and ymax,
	// we compute here the spatial extent of the vector layer.
	if (xmin >= xmax || ymin >= ymax) {
		// Loop through all the features.
		in_feat = OGR_L_GetNextFeature(in_lyr);
		in_geom = OGR_F_GetGeometryRef(in_feat);
		xmin = xmax = OGR_G_GetX(in_geom, 0);
		ymin = ymax = OGR_G_GetY(in_geom, 0);
		OGR_F_Destroy(in_feat);
		while ((in_feat = OGR_L_GetNextFeature(in_lyr)) != NULL) {
			in_geom = OGR_F_GetGeometryRef(in_feat);
			if (OGR_G_GetX(in_geom, 0) < xmin) {
				xmin = OGR_G_GetX(in_geom, 0);
			}
			if (OGR_G_GetX(in_geom, 0) > xmax) {
				xmax = OGR_G_GetX(in_geom, 0);
			}
			if (OGR_G_GetY(in_geom, 0) < ymin) {
				ymin = OGR_G_GetY(in_geom, 0);
			}
			if (OGR_G_GetY(in_geom, 0) > ymax) {
				ymax = OGR_G_GetY(in_geom, 0);
			}
			OGR_F_Destroy(in_feat);
		}
	} else {
		// Apply a spatial filter on the input layer.
		OGR_L_SetSpatialFilterRect(in_lyr, xmin, ymin, xmax, ymax);
	}
	
	OGR_L_ResetReading(in_lyr);
	
	// Compute the number of pixels in x and y
	sizex = lround(ceil((xmax - xmin) / resolution));
	sizey = lround(ceil((ymax - ymin) / resolution));
	
	// Find the raster type for the output raster file.
	if (strcmp(raster_type, "Float32")) {
		out_type = GDT_Float32;
	} else if (strcmp(raster_type, "Float64")) {
		out_type = GDT_Float64;
	} else {
		out_type = GDT_Int32;
	}
	
	// Now we can create the output raster file.
	out_driver = GDALGetDriverByName(format);
	if (out_driver == NULL) {
		fprintf(stderr, "Warning. Unable to find driver for GDAL format '%s'. Using HFA format instead.\n", format);
		out_driver = GDALGetDriverByName("HFA");
	}
	out_ds = GDALCreate(out_driver, output_file, sizex, sizey, 1, out_type, NULL);
	georef[0] = xmin;
	georef[1] = resolution;
	georef[2] = 0;
	georef[3] = ymax;
	georef[4] = 0;
	georef[5] = -1.0f*resolution;
	GDALSetGeoTransform(out_ds, georef);
	
	// Allocate the memory for holding the raster data.
	if (out_type == GDT_Int32) {
		dataInt = calloc(sizex*sizey, sizeof(int));
	} else {
		dataDbl = calloc(sizex*sizey, sizeof(double));
	}
	if (dataInt == NULL && dataDbl == NULL) {
		fprintf(stderr, "Error. Not enough memory.\n");
		exit(1);
	}
	if (strcmp(stat, "mean") == 0) {
		// We also need a count matrix.
		dataCnt = calloc(sizex*sizey, sizeof(int));
		if (dataCnt	== NULL) {
			fprintf(stderr, "Error. Not enough memory.\n");
			exit(1);
		}
	}
	
	
	// Number of threads = number of processors (or cores)
#if defined (_OPENMP)
	omp_set_num_threads(omp_get_num_procs());
#endif

	// Looping through all features and update the raster data accordingly.
	cnt = 0;
	while ((in_feat = OGR_L_GetNextFeature(in_lyr)) != NULL) {
		in_geom = OGR_F_GetGeometryRef(in_feat);
		x = OGR_G_GetX(in_geom, 0);
		y = OGR_G_GetY(in_geom, 0);
		i = lround(floor((x - xmin) / resolution));
		j = sizey - 1 - lround(floor((y - ymin) / resolution));
		//printf("%f / %f >> %i / %i\n", x, y, i, j);
		if (i >= 0 && i < sizex && j >= 0 && j < sizey) {
			if (attr == NULL) {
				if (out_type == GDT_Int32) {
					dataInt[j*sizex + i] += 1;
				} else {
					dataDbl[j*sizex + i] += 1;
				}
			} else {
				fidx = OGR_F_GetFieldIndex(in_feat, attr);
				if (out_type == GDT_Int32) {
					dataInt[j*sizex + i] += OGR_F_GetFieldAsInteger(in_feat, fidx);
				} else {
					dataDbl[j*sizex + i] += OGR_F_GetFieldAsDouble(in_feat, fidx);
				}
				if (strcmp(stat, "mean") == 0) {
					dataCnt[j*sizex + i] += 1;
				}
			}
			cnt++;
		}
		OGR_F_Destroy(in_feat);
	}
	
	
	if (strcmp(stat, "mean") == 0) {
		#pragma omp parallel for default(shared) private(i, j)
		for (j = 0; j < sizey; j++) {
			for (i = 0; i < sizex; i++) {
				if (dataCnt[j*sizex + i] > 0) {
					if (out_type == GDT_Int32) {
						dataInt[j*sizex + i] /= dataCnt[j*sizex + i];
					} else {
						dataDbl[j*sizex + i] /= dataCnt[j*sizex + i];
					}
				}
			}
		}
		free(dataCnt);
	}
	
	
	fprintf(stdout, "   %d features considered for creating the raster\n", cnt);
	
	
	// Close the vector file.
	OGR_DS_Destroy(in_ds);
	
	// Write the data out to the output raster file.
	out_band = GDALGetRasterBand(out_ds, 1);
	if (out_type == GDT_Int32) {
		GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, dataInt, sizex, sizey, GDT_Int32, 0, 0);
		free(dataInt);
	} else {
		GDALRasterIO(out_band, GF_Write, 0, 0, sizex, sizey, dataDbl, sizex, sizey, GDT_Float64, 0, 0);
		free(dataDbl);
	}
	GDALClose(out_ds);
	
	return 0;
}





