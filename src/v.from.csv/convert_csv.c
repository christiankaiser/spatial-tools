/*
 *  convert_csv.c
 *  v.from.csv
 *
 *  Created by Christian Kaiser on 20.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#include <stdlib.h>
#include <string.h>

#include <GDAL/ogr_api.h>

#include "csvlib.h"
#include "convert_csv.h"



int csv_to_ogr(char *infile, int xidx, int yidx, 
			   int header, char *sep, char *quote, 
			   char *format, char *outfile, char *layer)
{
	
	int i;
	CSVHandle csv;
	CSVRecord *rec;
	int npts, nfields, width;
	OGRSFDriverH driver;
	OGRDataSourceH out_ds;
	OGRLayerH out_lyr;
	OGRFieldDefnH field_def;
	OGRFeatureH feat;
	OGRGeometryH geom;
	char fname[255];
	double x, y;
	
	
	
	OGRRegisterAll();
	
	csv = CSVOpen(infile, "r", sep[0], header);
	npts = CSVGetRecordCount(csv);
	nfields = CSVGetFieldCount(csv);
	
	// Get the OGR driver
	driver = OGRGetDriverByName(format);
    if (driver == NULL) {
        fprintf(stderr, "Error. '%s' driver not available.\n", format);
        exit(1);
    }
	
	// Create the output data source.
	out_ds = OGR_Dr_CreateDataSource(driver, outfile, NULL);
    if (out_ds == NULL) {
        fprintf(stderr, "Error. Creation of output file failed.\n");
        exit(1);
    }
	
	// Create the output layer.
	out_lyr = OGR_DS_CreateLayer(out_ds, layer, NULL, wkbPoint, NULL);
	if (out_lyr == NULL) {
	    fprintf(stderr, "Error. Unable to create output layer.\n");
        exit(1);
    }
	
	// Create the attributes in the output layer.
	for (i = 0; i < nfields; i++) {
		CSVGetFieldInfo(csv, i, fname, &width);
		field_def = OGR_Fld_Create(fname, OFTString);
		OGR_Fld_SetWidth(field_def, width);
		if (OGR_L_CreateField(out_lyr, field_def, TRUE) != OGRERR_NONE) {
			fprintf(stderr, "Warning. Creating field '%s' failed.\n", fname);
		}
		OGR_Fld_Destroy(field_def);
	}
	
	// Read all the points and transform them into an OGR feature.
	CSVReset(csv);
	while ((rec = CSVGetNextRecord(csv)) != NULL) {
		if (rec->fields[xidx][0] == 0 || rec->fields[yidx][0] == 0) {
			continue;
		}
		feat = OGR_F_Create(OGR_L_GetLayerDefn(out_lyr));
		for (i = 0; i < nfields; i++) {
			OGR_F_SetFieldString(feat, i, rec->fields[i]);
		}
		x = atof(rec->fields[xidx]);
		y = atof(rec->fields[yidx]);
		geom = OGR_G_CreateGeometry(wkbPoint);
        OGR_G_SetPoint_2D(geom, 0, x, y);
		OGR_F_SetGeometry(feat, geom); 
        OGR_G_DestroyGeometry(geom);
		CSVDestroyRecord(rec);
		
		// Create the feature in the output file.
		if (OGR_L_CreateFeature(out_lyr, feat) != OGRERR_NONE) {
			fprintf(stderr, "Error. Failed to create feature in output layer.\n");
        }
        OGR_F_Destroy(feat);
	}
	
	// Close the files.
	OGR_DS_Destroy(out_ds);
	CSVClose(csv);
	
	return 0;
}




