




/**
 * Reads a raster band as a double array.
 * Returns 0 in case of success, a non-zero value in case of an error.
 */
int raster_band_read_double (char *raster, int band, double **data, int *rasterX, int *rasterY);


/**
 * Reads a raster band as a long array.
 * Returns 0 in case of success, a non-zero value in case of an error.
 */
int raster_band_read_long (char *raster, int band, long **data, int *rasterX, int *rasterY);