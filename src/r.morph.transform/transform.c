/*
 *  transform.c
 *  r.morph.transform
 *
 *  Created by Christian Kaiser on 14.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */



#include <stdio.h>

#include <GDAL/gdal.h>
#include <GDAL/ogr_api.h>

#include "proj.h"
#include "ogr_ext.h"
#include "transform.h"





int coord_transform(char *morphfile, int back_transform) {
	
	morph *grid;
	char coords[1000], space[1000];
	double x, y, proj_x, proj_y;
	
	// Read the input morph file
	grid = read_grid(morphfile);
	if (grid == NULL) {
		fprintf(stderr, "Error. Unable to read input morph file '%s'\n", morphfile);
		exit(1);
	}
	
	while (fgets(coords, 1000, stdin)) {
		sscanf(coords, "%lf%[, \t;]%lf\n", &x, space, &y);
		if (back_transform) {
			project_back_coord(x, y, &proj_x, &proj_y, grid);
		} else {
			project_coord(x, y, &proj_x, &proj_y, grid->x, grid->y, grid->xsize, grid->ysize, grid->georef);
		}
		fprintf(stdout, "%f\t%f\n", proj_x, proj_y);
	}
	
	free_morph(grid);
	
	return 0;
}




int raster_transform(char *infile, char *morphfile, char *outfile, char *format, int back_transform) {
	
	fprintf(stderr, "Error. Raster transform currently not implemented.\n");
	GDALAllRegister();
	return 0;
}




int vector_transform(char *infile, char *input_layer,  char *spatial_filter, char *attr_filter,
					 char *morphfile, 
					 char *outfile, char *attrs, char *format, char *output_layer,
					 int back_transform) {
	
	OGRSFDriverH drvr;
	OGRDataSourceH in_ds, out_ds;
	OGRLayerH in_lyr, out_lyr;
	OGRFeatureDefnH schema;
	OGRFeatureH in_feat, out_feat;
	OGRGeometryH in_geom, out_geom;
	morph *mgrid;
	double xmin, xmax, ymin, ymax;
	int ok;
	
	
	OGRRegisterAll();
	
	// Open the OGR datasource
	in_ds = OGROpen(infile, FALSE, &drvr);
	if (in_ds == NULL) {
		fprintf(stderr, "Error. Unable to open '%s' with OGR driver '%s'\n", infile, OGR_Dr_GetName(drvr));
		exit(1);
	}
	
	// Get the required input layer
	if (input_layer != NULL) {
		in_lyr = OGR_DS_GetLayerByName(in_ds, input_layer);
	} else {
		in_lyr = OGR_DS_GetLayer(in_ds, 0);
	}
	if (in_lyr == NULL) {
		fprintf(stderr, "Error. Unable to open layer for '%s'\n", infile);
		exit(1);
	}
	
	// Get the input layer definition
	schema = OGR_L_GetLayerDefn(in_lyr);
	
	// Apply the spatial filter and the attribute filter if needed.
	if (spatial_filter != NULL) {
		ok = sscanf(spatial_filter, "%lf,%lf,%lf,%lf", &xmin, &ymin, &xmax, &ymax);
		if (ok == 4) {
			OGR_L_SetSpatialFilterRect(in_lyr, xmin, ymin, xmax, ymax);
		}
	}
	if (attr_filter != NULL) {
		OGR_L_SetAttributeFilter(in_lyr, attr_filter);
	}
	
	// Read the input morph file
	mgrid = read_grid(morphfile);
	if (mgrid == NULL) {
		fprintf(stderr, "Error. Unable to read input morph file '%s'\n", morphfile);
		OGR_DS_Destroy(in_ds);
		exit(1);
	}
	
	
	// Create the output datasource
	// If an output format is specified, take this. If this is not the case,
	// check first if the input driver is able to create a new datasource.
	if (format != NULL) {
		drvr = OGRGetDriverByName(format);
		if (drvr == NULL) {
			fprintf(stderr, "Error. Unable to find a driver for format '%s'.\n", format);
			OGR_DS_Destroy(in_ds);
			free_morph(mgrid);
			exit(1);
		}
	} else if (OGR_Dr_TestCapability(drvr, "ODrCCreateDataSource") == FALSE) {
		fprintf(stderr, 
				"Error. Unable to write in format '%s'. Please specify an output format using the --format flag.\n", 
				OGR_Dr_GetName(drvr));
	}
	out_ds = OGR_Dr_CreateDataSource(drvr, outfile, NULL);
	if (out_ds == NULL) {
		fprintf(stderr, "Error. Unable to create output datasource '%s'\n", outfile);
		OGR_DS_Destroy(in_ds);
		free_morph(mgrid);
		exit(1);
	}
	
	// Create the output layer
	out_lyr = OGR_DS_CreateLayer(out_ds, 
								 output_layer, 
								 OGR_L_GetSpatialRef(in_lyr), 
								 OGR_FD_GetGeomType(schema), 
								 NULL);
	if (out_lyr == NULL) {
		fprintf(stderr, "Error. Unable to create output layer '%s'\n", output_layer);
		OGR_DS_Destroy(in_ds);
		OGR_DS_Destroy(out_ds);
		free_morph(mgrid);
		exit(1);
	}
	
	// Make a copy of the OGR feature definition for the new layer
	ogr_layer_copy_schema(in_lyr, &out_lyr, attrs);
	
	
	// Loop through the input features and transform the geometry.
	OGR_L_ResetReading(in_lyr);
	while ((in_feat = OGR_L_GetNextFeature(in_lyr)) != NULL) {
		
		// Create a new feature and copy the attributes.
		out_feat = OGR_F_Create(OGR_L_GetLayerDefn(out_lyr));
		ogr_feat_copy_attributes(in_feat, &out_feat, attrs);
		
		// Project the geometry.
		in_geom = OGR_F_GetGeometryRef(in_feat);
		out_geom = OGR_G_Clone(in_geom);
		project_geom(out_geom, mgrid, back_transform);
		OGR_F_SetGeometry(out_feat, out_geom);
		OGR_G_DestroyGeometry(out_geom);
		
		// Write the new feature to the output layer.
		OGR_L_CreateFeature(out_lyr, out_feat);
		OGR_F_Destroy(out_feat);
		
		OGR_F_Destroy(in_feat);
	}
	
	
	// Free the ressources
	OGR_DS_Destroy(in_ds);
	OGR_DS_Destroy(out_ds);
	free_morph(mgrid);
	
	return 0;
}






