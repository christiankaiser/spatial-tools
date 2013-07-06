/*
 *  create_grid.c
 *  r.morph.create.grid
 *
 *  Created by Christian Kaiser on 13.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#include <stdio.h>

#include <GDAL/ogr_api.h>

#include "create_grid.h"
#include "proj.h"



#define LINELENGTH 1000



int create_grid(char *infile, char *outfile, int gridsize, char *format, char *layer) {

	OGRSFDriverH out_driver;
	OGRDataSourceH out_ds;
	OGRLayerH out_layer;
	OGRFieldDefnH field;
	OGRFeatureH feat;
	OGRGeometryH geom;
	morph *mgrid;
	int xstep, ystep, step;
	double topleftx, toplefty, weres, nsres, rot1, rot2;
	int i,j, fid, row, col;
	
	
	OGRRegisterAll();
	
	// Read the morph grid file.
	mgrid = read_grid(infile);
	if (mgrid == NULL) {
		fprintf(stderr, "Error. Unable to read input morph file '%s'.\n", infile);
		exit(1);
	}
	
	// Create the ouptut layer.
	out_driver = OGRGetDriverByName(format);
	if (out_driver == NULL) {
		fprintf(stderr, "Error. Driver for format '%s' not available.\n", format);
		exit(1);
	}
	out_ds = OGR_Dr_CreateDataSource(out_driver, outfile, NULL);
	if (out_ds == NULL) {
		fprintf(stderr, "Error. Unable to create output data source.\n");
		exit(1);
	}
	out_layer = OGR_DS_CreateLayer(out_ds, layer, NULL, wkbLineString, NULL);
	if (out_layer == NULL) {
		fprintf(stderr, "Error. Unable to create output layer.\n");
		exit(1);
	}
	
	// Add the attributes to the new layer.
	field = OGR_Fld_Create("ID", OFTInteger);
	OGR_Fld_SetWidth(field, 11);
	if (OGR_L_CreateField(out_layer, field, TRUE) != OGRERR_NONE) {
		fprintf(stderr, "Error. Creating ID attribute failed.\n");
		exit(1);
	}
	OGR_Fld_Destroy(field);
	
	field = OGR_Fld_Create("ROW", OFTInteger);
	OGR_Fld_SetWidth(field, 6);
	if (OGR_L_CreateField(out_layer, field, TRUE) != OGRERR_NONE) {
		fprintf(stderr, "Error. Creating ROW attribute failed.\n");
		exit(1);
	}
	OGR_Fld_Destroy(field);
	
	field = OGR_Fld_Create("COL", OFTInteger);
	OGR_Fld_SetWidth(field, 6);
	if (OGR_L_CreateField(out_layer, field, TRUE) != OGRERR_NONE) {
		fprintf(stderr, "Error. Creating COL attribute failed.\n");
		exit(1);
	}
	OGR_Fld_Destroy(field);
	
	// Compute the step size.
	xstep = mgrid->xsize / gridsize;
	if (xstep < 1) {
		xstep = 1;
	}
	if (xstep > (mgrid->xsize / 2)) {
		xstep = mgrid->xsize / 2;
	}
	ystep = mgrid->ysize / gridsize;
	if (ystep < 1) {
		ystep = 1;
	}
	if (ystep > (mgrid->ysize / 2)) {
		ystep = mgrid->ysize / 2;
	}
	if (xstep < ystep) {
		step = ystep;
	} else {
		step = xstep;
	}
	
	// Read the georeference
	topleftx = mgrid->georef[0];
	weres = mgrid->georef[1];
	rot1 = mgrid->georef[2];
	toplefty = mgrid->georef[3];
	rot2 = mgrid->georef[4];
	nsres = mgrid->georef[5];
	
	// Write all horizontal lines.
	fid = 1;
	row = 1;
	for (j = 0; j < mgrid->ysize; j += step) {
		// Create a new Feature with the projected coordinates.
		feat = OGR_F_Create(OGR_L_GetLayerDefn(out_layer));
		OGR_F_SetFieldInteger(feat, OGR_F_GetFieldIndex(feat, "ID"), fid);
		OGR_F_SetFieldInteger(feat, OGR_F_GetFieldIndex(feat, "ROW"), row);
		OGR_F_SetFieldInteger(feat, OGR_F_GetFieldIndex(feat, "COL"), 0);

		// Create a new geometry object.
		geom = OGR_G_CreateGeometry(wkbLineString);
		for (i = 0; i < mgrid->xsize; i++) {
			OGR_G_SetPoint_2D(
							  geom, 
							  i, 
							  topleftx + mgrid->x[i][j]*weres + mgrid->y[i][j]*rot1, 
							  toplefty + mgrid->x[i][j]*rot2 + mgrid->y[i][j]*nsres);
		}
		OGR_F_SetGeometry(feat, geom);
		OGR_G_DestroyGeometry(geom);
		
		// Write the feature to the output vector layer.
		if (OGR_L_CreateFeature(out_layer, feat) != OGRERR_NONE) {
			fprintf(stderr, "Error. Unable to create new feature.\n");
			exit(1);
		}
		OGR_F_Destroy(feat);
		
		row++;
		fid++;
	}
	
	// Write all vertical lines.
	col = 1;
	for (i = 0; i < mgrid->xsize; i += step) {
		// Create a new Feature with the projected coordinates.
		feat = OGR_F_Create(OGR_L_GetLayerDefn(out_layer));
		OGR_F_SetFieldInteger(feat, OGR_F_GetFieldIndex(feat, "ID"), fid);
		OGR_F_SetFieldInteger(feat, OGR_F_GetFieldIndex(feat, "ROW"), 0);
		OGR_F_SetFieldInteger(feat, OGR_F_GetFieldIndex(feat, "COL"), col);
		
		// Create a new geometry object.
		geom = OGR_G_CreateGeometry(wkbLineString);
		for (j = 0; j < mgrid->ysize; j++) {
			OGR_G_SetPoint_2D(
							  geom, 
							  j, 
							  topleftx + mgrid->x[i][j]*weres + mgrid->y[i][j]*rot1, 
							  toplefty + mgrid->x[i][j]*rot2 + mgrid->y[i][j]*nsres);
		}
		OGR_F_SetGeometry(feat, geom);
		OGR_G_DestroyGeometry(geom);
		
		// Write the feature to the output vector layer.
		if (OGR_L_CreateFeature(out_layer, feat) != OGRERR_NONE) {
			fprintf(stderr, "Error. Unable to create new feature.\n");
			exit(1);
		}
		OGR_F_Destroy(feat);
		
		col++;
		fid++;
	}

	// Close the datasources and free the memory.
	OGR_DS_Destroy(out_ds);
	free_morph(mgrid);
	
	return 0;
}






