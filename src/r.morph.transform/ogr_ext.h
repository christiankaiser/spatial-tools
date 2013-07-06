/* OGR extensions */

#include <GDAL/ogr_api.h>


/*
 * Copies the field definition from one layer to another.
 * If attrs is not NULL, only the attriubutes in the comma-separated list are copied.
 */
int ogr_layer_copy_schema(OGRLayerH src_lyr, OGRLayerH *dst_lyr, char *attrs);


/*
 * Copies the attribute values from one feature to another.
 * If attrs is not NULL, only the attriubutes in the comma-separated list are copied.
 */
int ogr_feat_copy_attributes(OGRFeatureH src_feat, OGRFeatureH *dst_feat, char *attrs);


/*
 * Make a copy of the field definition.
 */
OGRFieldDefnH ogr_fld_clone(OGRFieldDefnH hField);