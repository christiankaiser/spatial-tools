/*
 *  potential.h
 *  r.potential
 *
 *  Created by Christian Kaiser on 23.05.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


#include "variogram.h"



#define MAX(a,b) ((b>a)?(b):(a))
#define MIN(a,b) ((b<a)?(b):(a))





int potential(char *iraster, int band, char *oraster, char *oformat, st_variogram *vg);

void computePotential(double *imatrix, double *omatrix, int rasterX, int rasterY, st_variogram *vg);
