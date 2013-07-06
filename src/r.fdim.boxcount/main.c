

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "boxcount.h"




static char *usage[] = {
"\nr.fdim.boxcount -- Estimates the fractal dimension using the boxcounting\n",
"                     method.\n\n",
"SYNOPSIS\n",
"   r.fdim.boxcount \n",
"      [--help] [--band raster_band] [--minbox 1] [--maxbox 20] [--vdom raster]\n",
"      [--plot output_plot] --raster input_raster \n\n",
"DESCRIPTION\n",
"   Computes the fractal dimension for a given raster file.\n\n",
"   The following options are available:\n\n",
"   -h\n",
"   --help\n",
"      Shows this information and quits.\n\n",
"   -b raster_band\n",
"   --band raster_band\n",
"      The raster band number for which we should estimate the fractal\n",
"      dimension. Default is 1.\n\n",
"   --minbox integer_value\n",
"      The minimum number of boxes to use for the box counting.\n",
"      Default is 1.\n\n",
"   --maxbox integer_value\n",
"      The maximum number of boxes (in 1 dimension) to use for the box\n",
"      counting. Default is 20.\n\n",
"   -v validity_domain_raster\n",
"   --vdom validity_domain_raster\n",
"      A raster file where 0 values are outside the allowed region.\n",
"      If present, a corrected fractal dimension is also computed.\n\n",
"   -p output_plot\n",
"   --plot output_plot\n",
"      Path to an output SVG file containing the fractal dimension output plot.\n\n",
"   -r input_raster\n",
"   --raster input_raster\n",
"      Raster for which we should estimate the fractal dimension.\n",
"      Values of 0 are considered as no occurence values, all others as\n",
"      occurences.\n\n",
"BUGS\n",
"   Please send any comments or bug reports to christian@361degres.ch.\n\n",
"VERSION\n",
"   1.0.0 (6.6.2009)\n\n",
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
	
	char *input_raster;				// Path to the input raster file.
	int band;						// Input raster band.
	int minbox;						// Minimum number of boxes.
	int maxbox;						// Maximum number of boxes.
	char *output_plot;				// Path to the output SVG plot
	char *vdom;						// Path to the validity domain raster file.
	
	int ok;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	// Initialize variables with default values.
	input_raster = NULL;
	band = 1;
	minbox = 1;
	maxbox = 20;
	output_plot = NULL;
	vdom = NULL;
	
	
	// Process command line
	while (1)
	{
		
		static struct option long_options[] =
		{
			{"help",	no_argument,		0,	'h'},
			{"band",	required_argument,	0,	'b'},
			{"minbox",	required_argument,	0,	'm'},
			{"maxbox",	required_argument,	0,	'x'},
			{"plot",	required_argument,	0,	'p'},
			{"raster",	required_argument,	0,	'r'},
			{"vdom",	required_argument,	0,	'v'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long(argc, (char**)argv, "hb:m:x:p:r:", long_options, NULL);
		
		// Detect the end of the options.
		if (c == -1)
			break;
		
		
		switch (c) {
				
			case 'h':
				index = 0;
				while (usage[index] != NULL) {
					printf(usage[index]);
					index++;
				}
				return 0;
			
			case 'b':
				band = atoi(optarg);
				break;
			
			case 'm':
				minbox = atoi(optarg);
				break;
			
			case 'x':
				maxbox = atoi(optarg);
				break;
				
			case 'v':
				vdom = optarg;
				break;
				
			case 'p':
				output_plot = optarg;
				break;
			
			case 'r':
				input_raster = optarg;
				break;
								
			case '?':
				return 1;
				
			default:
				abort();
		}
	}
	

	
	for (index = optind; index < argc; index++)
	{
		if (input_raster == NULL)
			input_raster = (char*)argv[index];
	}
	
	
	
	if (input_raster == NULL)
	{
		fprintf(stderr, "Error. You must provide at least the following file:\n");
		fprintf(stderr, "   input raster file\n");
		return 1;
	}
	
	
	
	ok = boxcount(input_raster, band, minbox, maxbox, vdom, output_plot);
	
    return ok;
	
}
