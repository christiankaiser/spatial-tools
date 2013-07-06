/*
 *  convert_csv.h
 *  v.from.csv
 *
 *  Created by Christian Kaiser on 20.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


int csv_to_ogr(char *infile, int xidx, int yidx, 
			   int header, char *sep, char *quote, 
			   char *format, char *outfile, char *layer);
