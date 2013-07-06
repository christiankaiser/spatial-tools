/*
 
 r.clusters.stats
 Computes statistics for different clusters. The clusters are defined in a
 raster file where the value corresponds to the cluster ID. The statistics
 are computed for the statistic raster file according to the spatial clusters
 defined in the cluster raster file. The statistics include the sum, number of
 cells, average, minimum and maximum.
 
 Version:	1.0
 Date:		24.11.2009
 Author:	Christian Kaiser, christian.kaiser@unil.ch
 
 Copyright (C) 2009 Christian Kaiser.
 
 r.clusters.stats is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 2, or (at your option) any
 later version.
 
 r.clusters.stats is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with r.clusters.stats; see the file COPYING.  If not, write to the Free
 Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301, USA.
 
 */



#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "clusters_stats.h"
#include "gdal.h"



static char *usage[] = {
	"\nr.clusters.stats -- Computes statistics for different clusters. The \n",
	"   clusters are defined in a raster file where the value corresponds to \n",
	"   the cluster ID. The statistics are computed for the statistic raster \n",
	"   file according to the spatial clusters defined in the cluster raster \n",
	"   file. The statistics include the sum, number of cells, average, minimum\n",
	"   and maximum.\n\n",
	"SYNOPSIS\n",
	"   r.clusters.stats\n",
	"      [--help]\n",
	"      [--nodata 0]\n",
	"      --clusters clusters_raster --values statistics_raster\n",
	"      --output output_text_file\n\n",
	"DESCRIPTION\n",
	"   The following options are available:\n\n",
	"   -h\n",
	"   --help\n",
	"      Shows this usage note.\n\n",
	"   -n\n",
	"   --nodata\n",
	"      Indicates which values in the clusters raster file should be \n",
	"      considered as no data.\n\n",
	"   -c clusters_raster\n",
	"   --clusters clusters_raster\n",
	"      The raster file containing the clusters. It should contain the IDs\n",
	"      of each cluster. The raster file should contain integer values.\n",
	"      Only the first band of the file is considered.\n\n",
	"   -v statistics_raster\n",
	"   --values statistics_raster\n",
	"      The raster for which we should compute the cluster statistics.\n",
	"      Only the first band of the raster is considered.\n",
	"      Both clusters and values raster file must have the same dimensions.\n\n",
	"   -o output_text_file\n",
	"   --output output_text_file\n",
	"      The path to the output text file which will contain the cluter\n",
	"      statistics.\n\n",
	"BUGS\n",
	"   Please send any comments or bug reports to christian@361degres.ch.\n\n",
	"VERSION\n",
	"   1.0.0 (24.11.2009)\n\n",
	"AUTHOR\n"
	"   Christian Kaiser, University of Lausanne <christian@361degres.ch>\n\n",
	"ACKNOWLEDGEMENTS\n",
	"   This work has been supported by the Swiss National Foundation, through the projects \"Urbanization\n",
	"   Regime and Environmental Impact: Analysis and Modelling of Urban Patters, Clustering and\n",
	"   Methamorphoses\" (no 100012-113506, see www.clusterville.org for more information) and \n",
	"   \"GeoKernels\": Kernel-Based Methods for Geo- and Environmental Sciences, phase 2 \n",
	"   (no 200020-121835, see www.geokernels.org for more information).\n\n",
	NULL};





int main (int argc, const char *argv[])
{
	int c;
	int index;
	
	int64_t nodata;					// No data value
	char *clusters_raster;			// Path to the clusters raster file
	char *values_raster;			// Path to the statistics raster file
	char *output_file;				// Path to the output text file.
	
	int ok;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	
	// Initialize variables with default values.
	nodata = 0;
	clusters_raster = NULL;
	values_raster = NULL;
	output_file = NULL;
	
	
	
	// Process command line
	while (1)
	{
		
		static struct option long_options[] =
		{
			{"help",				no_argument,		0,	'h'},
			{"nodata",				required_argument,	0,	'n'},
			{"clusters",			required_argument,	0,	'c'},
			{"values",				required_argument,	0,	'v'},
			{"output",				required_argument,	0,  'o'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long(argc, (char**)argv, "hn:c:v:o:", long_options, NULL);
		
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
				
			case 'n':
				nodata = atoll(optarg);
				break;
				
			case 'c':
				clusters_raster = optarg;
				break;
				
			case 'v':
				values_raster = optarg;
				break;
				
			case 'o':
				output_file = optarg;
				break;
				
			case '?':
				return 1;
				
			default:
				abort();
		}
	}
	
	
	
	
	
	if (clusters_raster == NULL) {
		fprintf(stderr, "Error. You must provide a clusters raster file.\n");
		return 1;
	}

	if (values_raster == NULL) {
		fprintf(stderr, "Error. You must provide a values raster file.\n");
		return 1;
	}
	
	
	
	
	GDALAllRegister();
	ok = clusters_stats(clusters_raster, nodata, values_raster, output_file);
	fprintf(stdout, "r.clusters.stats done.\n");
	
	
	return ok;
}








