
#include "variogram.h"
#include "potential.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>





static char *usage[] = {
"\nr.potential -- computes a potential surface based on an input raster and\n",
"                 a spatial variogram.\n\n",
"SYNOPSIS\n",
"   r.potential \n",
"      [-m model] [-r range] [-s sill] [-n nugget] [-p power] [-f format]\n",
"      [-b band] input_raster output_raster\n\n",
"DESCRIPTION\n",
"   The following options are available:\n\n",
"   -m model\n",
"      The model type to use for the variogram. Possible options are:\n",
"      exp (exponential), spher (spherical), gauss (gaussian), power\n\n",
"   -r range\n",
"      The range is measured in number of pixels (a range of 5 means\n",
"      5 pixels, with a resolution of 100 meters per pixel, this is 500 meters.\n\n",
"   -s sill\n",
"      The sill should be 1 for the computation of the potential.\n",
"      (The potential is 1 - y, where y is the variogram value)\n\n",
"   -n nugget\n",
"      You may want to include some noise. E.g. if you have a nugget of 0.2,\n",
"      the potential at distance 0 is weightet with 0.8 instead of 1.\n\n",
"   -p power\n",
"      For power model only, where it replaces the nugget. Default is 2.\n\n",
"   -f format\n",
"      Format for the output raster file. Default is HFA as it\n",
"      supports all needed data types.\n",
"      The following formats are supported:\n",
"      GTiff    : GeoTIFF\n",
"      HFA      : Erdas Imagine (.img)\n",
"      AAIGrid  : Arc/Info ASCII Grid\n",
"      PNG      : Portable Network Graphics\n",
"      JPEG     : JPEG image\n",
"      GIF      : Graphics Interchange Format\n",
"      PCIDSK   : PCIDSK Database File\n",
"      PCRaster : PCRaster Raster File\n",
"      GMT      : GMT NetCDF Grid Format\n",
"      JPEG2000 : JPEG-2000\n",
"      RST      : Idrisi Raster A.1\n",
"      ENVI     : ENVI .hdr Labelled\n",
"\n",
"   -b band\n",
"      The band to be considered for the potential estimation. Default is 1.\n\n",
"   input_raster\n\n",
"   output_raster\n",
"      The output raster is a 64-bits floating point raster.\n\n",
"   \n\n",
"BUGS\n",
"   Please send any comments or bug reports to christian@361degres.ch.\n\n",
"VERSION\n",
"   1.0.0 (24.5.2009)\n\n",
"AUTHOR\n"
"   Christian Kaiser, University of Lausanne <christian@361degres.ch>\n\n",
"ACKNOWLEDGEMENTS\n",
"   This work has been supported by the Swiss National Foundation, through the projects \"Urbanization\n",
"   Regime and Environmental Impact: Analysis and Modelling of Urban Patters, Clustering and\n",
"   Methamorphoses\" (no 100012-113506, see www.clusterville.org for more information) and \n",
"   \"GeoKernels\": Kernel-Based Methods for Geo- and Environmental Sciences, phase 2 \n",
"   (no 200020-121835, see www.geokernels.org for more information).\n\n",
NULL};







int main (int argc, const char * argv[])
{
	int c;
	int index;
	char *iraster;					// Input raster.
	char *oraster;					// Output raster.
	char *model;					// Variogram model to use
	double range;
	double sill;
	double nugget;
	double power;
	int band;
	char *oformat;					// Output raster format.
	char defaultFormat[] = "HFA";	// Default format is Imagine
	char defaultModel[] = "exp";	// Exponential variogram model is default
	st_variogram vg;
	int ok;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	// Initialize variables with default values.
	iraster = NULL;
	oraster = NULL;
	range = 1.0f;
	sill = 1.0f;
	nugget = 0.0f;
	power = 2.0f;
	band = 1;
	oformat = defaultFormat;
	model = defaultModel;
	
	
	// Process command line
	while ((c = getopt(argc, (char**)argv, "hm:r:s:n:f:b:p:")) != -1) {
		switch (c) {
				
			case 'h':
				index = 0;
				while (usage[index] != NULL) {
					printf(usage[index]);
					index++;
				}
				return 0;
			
			case 'm':
				model = optarg;
				break;
				
			case 'r':
				range = atof(optarg);
				break;
			
			case 's':
				sill = atof(optarg);
				break;
			
			case 'n':
				nugget = atof(optarg);
				break;
			
			case 'p':
				power = atof(optarg);
				break;
				
			case 'f':
				oformat = optarg;
				break;
			
			case 'b':
				band = (int)atol(optarg);
				break;
				
			case '?':
				if (optopt == 'b' || optopt == 'c') {
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				} else {
					fprintf(stderr, "Unknown option -%c.\n", optopt);
				}
				return 1;
				
			default:
				abort();
		}
	}
	
	for (index = optind; index < argc; index++)
	{
		if (iraster == NULL)
			iraster = (char*)argv[index];
		else
			oraster = (char*)argv[index];
	}
	
	if (iraster == NULL && oraster == NULL)
	{
		fprintf(stderr, "You must provide an input and output raster.\n");
		return 1;
	}
	
	
	vg.model = ST_VGMODEL_EXPONENTIAL;
	if (strcmp(model, "spher") == 0)
		vg.model = ST_VGMODEL_SPHERICAL;
	else if (strcmp(model, "gauss") == 0)
		vg.model = ST_VGMODEL_GAUSSIAN;
	else if (strcmp(model, "power") == 0)
		vg.model = ST_VGMODEL_POWER;
	
	vg.range = range;
	vg.sill = sill;
	vg.nugget = nugget;
	vg.power = power;

	ok = potential(iraster, band, oraster, oformat, &vg);
	
    return ok;
}
