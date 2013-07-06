/*
 *  ogr_ext.c
 *  r.morph.transform
 *
 *  Created by Christian Kaiser on 14.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#include <string.h>

#include "ogr_ext.h"




int ogr_layer_copy_schema(OGRLayerH src_lyr, OGRLayerH *dst_lyr, char *attrs) {
	
	OGRFeatureDefnH schema;
	OGRFieldDefnH src_fdef, dst_fdef;
	int ncols, i, j, nattrs, copy_field;
	char attrs_copy[10000];
	char *attr_list[50], *a;
	
	// Extract the list of attributes if there is one.
	if (attrs != NULL) {
		strncpy(attrs_copy, attrs, 10000);
		attrs_copy[9999] = 0;
		nattrs = 0;
		a = strtok(attrs_copy, ",");
		if (a == NULL) {
			return 0;
		}
		attr_list[nattrs] = a;
		nattrs++;
		while (a != NULL) {
			a = strtok(NULL, ",");
			attr_list[nattrs] = a;
			nattrs++;
		}
	}
	
	schema = OGR_L_GetLayerDefn(src_lyr);
	ncols = OGR_FD_GetFieldCount(schema);
	
	for (i = 0; i < ncols; i++) {
		src_fdef = OGR_FD_GetFieldDefn(schema, i);
		
		// Check if we should create the field.
		if (attrs != NULL) {
			copy_field = FALSE;
			for (j = 0; j < nattrs; j++) {
				if (strcmp(attr_list[j], OGR_Fld_GetNameRef(src_fdef)) == 0) {
					copy_field = TRUE;
				}
			}
		} else {
			copy_field = TRUE;
		}
		
		if (copy_field) {
			dst_fdef = ogr_fld_clone(src_fdef);
			if (OGR_L_CreateField(*dst_lyr, dst_fdef, TRUE) != OGRERR_NONE) {
				fprintf(stderr, 
						"Warning. Unable to create attribute '%s'. Try to proceed anyway.\n", 
						OGR_Fld_GetNameRef(dst_fdef));
			}
			OGR_Fld_Destroy(dst_fdef);
		}
	}
	
	return 0;
}





int ogr_feat_copy_attributes(OGRFeatureH src_feat, OGRFeatureH *dst_feat, char *attrs) {
	
	OGRFeatureDefnH src_feat_def, dst_feat_def;
	OGRFieldDefnH src_fld_def;
	int i, j, nattrs, copy_field;
	char attrs_copy[10000];
	char *attr_list[50], *a;
	
	// Extract the list of attributes if there is one.
	if (attrs != NULL) {
		strncpy(attrs_copy, attrs, 10000);
		attrs_copy[9999] = 0;
		nattrs = 0;
		a = strtok(attrs_copy, ",");
		if (a == NULL) {
			return 0;
		}
		attr_list[nattrs] = a;
		nattrs++;
		while (a != NULL) {
			a = strtok(NULL, ",");
			attr_list[nattrs] = a;
			nattrs++;
		}
	}
	
	src_feat_def = OGR_F_GetDefnRef(src_feat);
	dst_feat_def = OGR_F_GetDefnRef(*dst_feat);
	
	for (i = 0; i < OGR_FD_GetFieldCount(src_feat_def); i++) {
		
		src_fld_def = OGR_FD_GetFieldDefn(src_feat_def, i);
		
		// Check if we should copy the field.
		if (attrs != NULL) {
			copy_field = FALSE;
			for (j = 0; j < nattrs; j++) {
				if (strcmp(attr_list[j], OGR_Fld_GetNameRef(src_fld_def)) == 0) {
					copy_field = TRUE;
				}
			}
		} else {
			copy_field = TRUE;
		}
		
		if (copy_field) {
			OGR_F_SetFieldString(*dst_feat, i, OGR_F_GetFieldAsString(src_feat, i));
		}
	}
	
	return 0;
}





OGRFieldDefnH ogr_fld_clone(OGRFieldDefnH hField) {
	
	OGRFieldDefnH dst_def;
	
	dst_def = OGR_Fld_Create(OGR_Fld_GetNameRef(hField), OGR_Fld_GetType(hField));
	OGR_Fld_Set(dst_def, 
				OGR_Fld_GetNameRef(hField), 
				OGR_Fld_GetType(hField), 
				OGR_Fld_GetWidth(hField), 
				OGR_Fld_GetPrecision(hField), 
				OGR_Fld_GetJustify(hField));
	
	return dst_def;
}


