/*
 *  downscale.h
 *  r.downscale
 *
 *  Created by Christian Kaiser on 24.05.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */




int downscale (char *agg_stats, 
			   char *agg_raster, 
			   char *prob_raster, 
			   char *prior_raster, 
			   char *vdom_raster,
			   char *oraster, 
			   char *oformat);





/**
 * Reads the aggregated statistics text file. The file containes 2 tab-separated
 * columns; the first contains the feature id, and the second the aggregated
 * statistitic (a sum).
 * @param agg_stats		path to the file
 * @param *agg_sum		pointer to a integer array which will be allocated by
 *						this function. The user is responsible to release it
 *						by calling free(). Contains the aggregated statistics.
 * @param *minindex		pointer to an integer containing the minimum feature id.
 * @param *maxindex		pointer to an integer containing the maximum feature id.
 * @return				an error code: 0 if success, a value different from 0 in
 *						case of an error.
 */
int readAggregatedStats (char* agg_stats, int** agg_sum, int *minindex, int *maxindex);



int readAggregateRaster(char *agg_raster, int** agg_data, int *rasterX, int *rasterY);


int readProbabilityRaster(char *prob_raster, double** prob_data);

int readPriorRaster(char *prior_raster, int** prior_data);

int readValidityDomainRaster(char *vdom_raster, int** vdom);




void downscaleData(int *agg_sum, int minindex, int maxindex, 
				   int *agg_data, double *prob_data, int *prior_data, int *vdom, int *out_data, 
				   int rasterX, int rasterY);




/**
 * Estimates the posterior distribution for a given feature.
 */
void estimateDistribution(int feature_id, int *feature_dist,
						  int diff_to_distribute, 
						  double *prob_dist, double prob_sum, 
						  int *vdom,
						  int *out_dist,
						  int size_x, int size_y);




void findIndexForCumulatedValue(double cum_val, double *prob_dist, int *vdom,
								int feature_id, int *feature_dist, 
								int size_x, int size_y, 
								int *i, int *j);

