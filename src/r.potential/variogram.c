/*
 *  variogram.c
 *  r.potential
 *
 *  Created by Christian Kaiser on 23.05.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "variogram.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>


double st_variogram_value (st_variogram *variogram, double h)
{
	if (variogram->model == ST_VGMODEL_EXPONENTIAL)
		return st_variogram_value_exponential(variogram, h);
	else if (variogram->model == ST_VGMODEL_SPHERICAL)
		return st_variogram_value_spherical(variogram, h);
	else if (variogram->model == ST_VGMODEL_GAUSSIAN)
		return st_variogram_value_gaussian(variogram, h);
	else if (variogram->model == ST_VGMODEL_POWER)
		return st_variogram_value_power(variogram, h);
	else
	{
		fprintf(stderr, "Error. Model for provided variogram unknown.\n");
		return NAN;
	}
}




double st_variogram_value_exponential (st_variogram *variogram, double h)
{
	double y;
	
	if (variogram->model != ST_VGMODEL_EXPONENTIAL)
	{
		fprintf(stderr, "Error. Provided variogram model is not exponential.\n");
		return NAN;
	}
	
	if (variogram->range == 0)
	{
		fprintf(stderr, "Error. The range of an exponential variogram cannot be 0.\n");
		return NAN;
	}

	y = (variogram->sill - variogram->nugget) * (1 - exp(-h*3 / variogram->range)) + variogram->nugget;
	return y;
}




double st_variogram_value_spherical (st_variogram *variogram, double h)
{
	double y;
	
	if (variogram->model != ST_VGMODEL_SPHERICAL)
	{
		fprintf(stderr, "Error. Provided variogram model is not spherical.\n");
		return NAN;
	}
	
	if (variogram->range == 0)
	{
		fprintf(stderr, "Error. The range of a spherical variogram cannot be 0.\n");
		return NAN;
	}
	
	if (h >= variogram->range)
		y = variogram->sill;
	else
		y = (variogram->sill - variogram->nugget) * 
			(((3*h) / (2*variogram->range)) - ((h*h*h)/(2*pow(variogram->range, 3.0f)))) + 
			 variogram->nugget;
	
	return y;
}



double st_variogram_value_gaussian (st_variogram *variogram, double h)
{
	double y;
	
	if (variogram->model != ST_VGMODEL_GAUSSIAN)
	{
		fprintf(stderr, "Error. Provided variogram model is not exponential.\n");
		return NAN;
	}
	
	if (variogram->range == 0)
	{
		fprintf(stderr, "Error. The range of an exponential variogram cannot be 0.\n");
		return NAN;
	}
	
	y = (variogram->sill - variogram->nugget) * (1 - exp(-h*h*3 / (variogram->range*variogram->range))) + variogram->nugget;
	return y;
}



double st_variogram_value_power (st_variogram *variogram, double h)
{
	double y;
	y = -(variogram->sill - variogram->nugget) * (pow((h+1),(-variogram->power))) + variogram->sill;
	return y;
}






const char *st_variogram_model_name (st_variogram *vg)
{
	if (vg->model == ST_VGMODEL_EXPONENTIAL)
		return "exponential";
	
	if (vg->model == ST_VGMODEL_SPHERICAL)
		return "spherical";

	if (vg->model == ST_VGMODEL_GAUSSIAN)
		return "gaussian";
	
	if (vg->model == ST_VGMODEL_POWER)
		return "power";
	
	return "unknown";
}




