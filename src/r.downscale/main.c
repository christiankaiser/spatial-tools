


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "downscale.h"




static char *usage[] = {
"\nr.downscale -- \n",
"                 \n\n",
"SYNOPSIS\n",
"   r.downscale \n",
"      [-h] [-f format] [-p prior_raster] [-v validity_domain_raster]\n",
"      aggregated_stats.txt aggregate_raster probability_raster output_raster\n\n",
"DESCRIPTION\n",
"   Note that all raster files must cover the same region and have the same \n",
"   resolution.\n\n",
"   The following options are available:\n\n",
"   -h\n",
"      Shows this information and quits.\n\n",
"   -p prior_raster\n",
"      Raster which contains the prior distribution. This must be an integer\n",
"      raster dataset.\n\n",
"   -v validity_domain_raster\n",
"      Raster which contains 0 and 1 values. Regions of value 0 are excluded\n",
"      from the downscaling process.\n\n",
"   -f format\n",
"      Format for the output raster file. Default is HFA.\n",
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
"   aggregated_stats.txt\n",
"      Text file containing the aggregated statistics. It is a tab-separated\n",
"      file with 2 columns. The first column contains the feature id, the\n",
"      second the aggregated statistical value (a sum, which must be an \n",
"      integer). The first line should contain a header, it is ignored.\n\n",
"   aggregate_raster\n",
"      A raster file containing the spatial locations of the features. It is\n",
"      simply a raster containing the feature id's per pixel.\n",
"      Feature id's must be integer values.\n\n",
"   probability_raster\n",
"      Raster which indicates the spatial probability distribution. This \n",
"      raster may contain double values.\n\n",
"   output_raster\n",
"      The output raster containing the downscaled statistics. It is an integer\n",
"      raster dataset.\n\n",
"BUGS\n",
"   Please send any comments or bug reports to christian@361degres.ch.\n\n",
"VERSION\n",
"   1.1.1 (3.6.2009)\n\n",
"AUTHOR\n"
"   Christian Kaiser, University of Lausanne <christian@361degres.ch>\n\n",
"ACKNOWLEDGEMENTS\n",
"   This work has been supported by the Swiss National Foundation, through the\n",
"   projects \"Urbanization Regime and Environmental Impact: Analysis and \n",
"   Modelling of Urban Patters, Clustering and Methamorphoses\" (no \n",
"   100012-113506, see www.clusterville.org for more information) and \n",
"   \"GeoKernels\": Kernel-Based Methods for Geo- and Environmental Sciences,\n",
"   phase 2 (no 200020-121835, see www.geokernels.org for more information).\n\n",
NULL};








int main (int argc, const char * argv[])
{
	int c;
	int index;
	
	char *agg_stats;				// Path to the aggregated statistics text file.
	char *agg_raster;				// Aggregate raster.
	char *prob_raster;				// Probability raster.
	char *prior_raster;				// Prior distribution raster.
	char *vdom_raster;				// Validity domain raster.
	char *oraster;					// Output raster.
	char *oformat;					// Output raster format.
	char defaultFormat[] = "HFA";	// Default format is Imagine
	int ok;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	// Initialize variables with default values.
	oformat = defaultFormat;
	agg_stats = NULL;
	agg_raster = NULL;
	prob_raster = NULL;
	prior_raster = NULL;
	vdom_raster = NULL;
	oraster = NULL;
	
	
	// Process command line
	while ((c = getopt(argc, (char**)argv, "hf:p:v:")) != -1) {
		switch (c) {
				
			case 'h':
				index = 0;
				while (usage[index] != NULL) {
					printf(usage[index]);
					index++;
				}
				return 0;
				
			case 'p':
				prior_raster = optarg;
				break;
				
			case 'v':
				vdom_raster = optarg;
				break;
				
			case 'f':
				oformat = optarg;
				break;
				
			case '?':
				if (optopt == 'p' || optopt == 'f') {
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
		
		if (agg_stats == NULL)
			agg_stats = (char*)argv[index];
		else if (agg_raster == NULL)
			agg_raster = (char*)argv[index];
		else if (prob_raster == NULL)
			prob_raster = (char*)argv[index];
		else
			oraster = (char*)argv[index];
	}
	
	if (agg_stats == NULL || agg_raster == NULL || prob_raster == NULL || oraster == NULL)
	{
		fprintf(stderr, "Error. You must provide at least the following files:\n");
		fprintf(stderr, "   aggregated statistics text file\n");
		fprintf(stderr, "   aggregate raster file\n");
		fprintf(stderr, "   probability raster file\n");
		fprintf(stderr, "   output raster file path\n");
		return 1;
	}
	
	
	
	ok = downscale(agg_stats, agg_raster, prob_raster, prior_raster, vdom_raster, oraster, oformat);

    return ok;
}
