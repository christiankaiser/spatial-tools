

#include "raster.h"

#include <gdal/gdal.h>
#include <stdlib.h>




int raster_band_read_double (char *raster, int band, double **data, int *rasterX, int *rasterY)
{
	GDALDatasetH idataset;
	GDALRasterBandH iband;				// The input raster band.


	// Open the input raster file.
	idataset = GDALOpen(raster, GA_ReadOnly);
	if (idataset == NULL)
	{
		fprintf(stderr, "Error. Unable to open raster '%s'\n", raster);
		return 1;
	}

	// Get the size of the input raster.
	*rasterX = GDALGetRasterXSize(idataset);
	*rasterY = GDALGetRasterYSize(idataset);

	// Get the input raster band.
	iband = GDALGetRasterBand(idataset, band);

	// Fetch the input raster band content.
	*data = (double*) malloc(*rasterX * *rasterY * sizeof(double));
	if (*data == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory to read raster '%s'.\n", raster);
		return 1;
	}
	GDALRasterIO(iband, GF_Read, 0, 0, *rasterX, *rasterY, *data, *rasterX, *rasterY, GDT_Float64, 0, 0);

	GDALClose(idataset);

	return 0;
}




int raster_band_read_long (char *raster, int band, long **data, int *rasterX, int *rasterY)
{
	GDALDatasetH idataset;
	GDALRasterBandH iband;				// The input raster band.


	// Open the input raster file.
	idataset = GDALOpen(raster, GA_ReadOnly);
	if (idataset == NULL)
	{
		fprintf(stderr, "Error. Unable to open raster '%s'\n", raster);
		return 1;
	}

	// Get the size of the input raster.
	*rasterX = GDALGetRasterXSize(idataset);
	*rasterY = GDALGetRasterYSize(idataset);

	// Get the input raster band.
	iband = GDALGetRasterBand(idataset, band);

	// Fetch the input raster band content.
	*data = (long*) malloc(*rasterX * *rasterY * sizeof(long));
	if (*data == NULL)
	{
		GDALClose(idataset);
		fprintf(stderr, "Error. Not enough memory to read raster '%s'.\n", raster);
		return 1;
	}
	GDALRasterIO(iband, GF_Read, 0, 0, *rasterX, *rasterY, *data, *rasterX, *rasterY, GDT_Int32, 0, 0);

	GDALClose(idataset);

	return 0;
}
