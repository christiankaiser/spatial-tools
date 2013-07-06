/*
 
 v.group.distance 
 
 Version:	1.0.0
 Date:		15.1.2011
 Author:	Christian Kaiser, chri.kais@gmail.com
 
 Copyright (C) 2011 Christian Kaiser.
 
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "group_dist.h"


static char *usage[] = {
	"\n",
	"v.group.distance\n",
	"   PURPOSE\n",
	"      Allows computing the distances between groups of points.\n",
	"      Each point is labelled with a group ID, and the mean distance\n",
	"      each group is computed. All possible combinations between the points\n",
	"      of two groups are considered for computing the mean distance.\n",
	"      Currently, computation of Euclidean and Mahalanobis distances are\n",
	"      implemented. The result is written to standard output.\n",
	"   SYNOPSIS\n",
	"      v.group.distance [--help]\n",
	"         --input INPUT_DATASOURCE [--layer INPUT_LAYER]\n",
	"         --group_attr GROUP_ATTRIBUTE\n",
	"         [--manhattan] [--median]\n",
	"   DESCRIPTION\n",
	"      The following options are available:\n",
	"         --help         Shows this usage note.\n",
	"         --input        Path to a OGR compatible input datasource.\n",
	"         --layer        A datasource might have more than one layer.\n",
	"                        In this case, this options allows to specify \n",
	"                        the input layer name.\n",	
	"         --group_attr   The name of the attribute containing the group ID.\n",
	"                        The attribute has to be an integer.\n",
	"         --manhattan    Computes the Manhattan distance instead of\n",
	"                        Euclidean distance.\n",
	"         --median       Computes the median distance instead of mean \n",
	"                        distance between the points.\n",
	"   BUGS\n",
	"      Please send any comments or bug reports to chri.kais@gmail.com.\n",
	"   VERSION\n",
	"      1.0.1 (18.01.2011)\n",
	"   AUTHOR\n"
	"      Christian Kaiser, National Centre for Geocomputation,\n",
	"      National University of Ireland Maynooth\n",
	"      <chri.kais@gmail.com>\n",
	"\n",
	NULL};


void printUsage();


int main (int argc, const char *argv[]) {
	
	int c;
	int ok;
	
	char *infile, *layer, *group_attr;
	int manhattan, median;
	
	extern int optind;
	extern int optopt;
	extern char * optarg;
	
	
	if (argc < 4) {
		printUsage();
		return(0);
	}
	
	
	// Provide default values.
	infile = layer = group_attr = NULL;
	median = manhattan = 0;
	
	
	// Process command line
	while (1) {
		static struct option long_options[] =
		{
			{"help",			no_argument,		0,	'h'},
			{"input",			required_argument,	0,	'i'},
			{"layer",			required_argument,	0,	'l'},
			{"group_attr",		required_argument,  0,  'g'},
			{"median",          no_argument,		0,	'm'},
			{"manhattan",		no_argument,		0,  't'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long(argc, (char**)argv, "hi:l:g:", long_options, NULL);
		if (c == -1) {
			break;
		}
		
		switch (c) {
			case 'h':
				printUsage();
				return 0;
			case 'i':
				infile = optarg;
				break;
			case 'l':
				layer = optarg;
				break;
			case 'g':
				group_attr = optarg;
				break;
			case 'm':
				median = 1;
				break;
			case 't':
				manhattan = 1;
				break;
			case '?':
				return 1;
			default:
				printUsage();
				abort();
		}
	}
	
	
	// Input value checking.
	if (infile == NULL) {
		fprintf(stderr, "Error. You must provide a input file.\n");
		exit(1);
	}
	if (group_attr == NULL) {
		fprintf(stderr, "Error. Specify an group attribute.\n");
		exit(1);
	}
	
	// Let's go!
	ok = group_dist(infile, layer, group_attr, median, manhattan);
	
	return ok;
}




void printUsage () {
	int index = 0;
	while (usage[index] != NULL) {
		printf(usage[index]);
		index++;
	}
}



