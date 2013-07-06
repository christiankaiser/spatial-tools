/*
 *  group_dist.c
 *  v.group.distance
 *
 *  Created by Christian Kaiser on 15.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#include <math.h>
#include <stdlib.h>
#include <omp.h>

#include <GDAL/ogr_api.h>

#include "group_dist.h"


int group_dist(char *infile, char *layer, char *group_attr, int median, int manhattan) {

	OGRDataSourceH ds;
	OGRLayerH lyr;
	OGRFeatureH feat;
	OGRGeometryH geom;
	int i, j;				// Iterator variables
	int npts;				// Number of points
	int *gid, *sgid;		// Array of group IDs
	int fidx;				// Field index
	int ngrps;				// Number of groups
	group *grps;			// Array with all the groups and points
	int grpidx;				// Group index
	int ngrppts;			// Number of group points
	
	
	OGRRegisterAll();
	
	// Open the input file.
	ds = OGROpen(infile, FALSE, NULL);
	if (ds == NULL) {
		fprintf(stderr, "Error. Unable to open '%s'\n", infile);
		exit(1);
	}
	if (layer == NULL) {
		lyr = OGR_DS_GetLayer(ds, 0);
	} else {
		lyr = OGR_DS_GetLayerByName(ds, layer);
	}
	if (lyr == NULL) {
		fprintf(stderr, "Error. Unable to get layer for '%s'\n", infile);
		OGR_DS_Destroy(ds);
		exit(1);
	}
	OGR_L_ResetReading(lyr);
	
	// Check the geometry type of the input layer, and count the number of points.
	npts = 0;
    while ((feat = OGR_L_GetNextFeature(lyr)) != NULL) {
		geom = OGR_F_GetGeometryRef(feat);
		if (geom != NULL && wkbFlatten(OGR_G_GetGeometryType(geom)) == wkbPoint) {
			npts++;
		}
		OGR_F_Destroy(feat);
	}
	if (npts == 0) {
		fprintf(stderr, "Error. Input file is not a point layer.\n");
		OGR_DS_Destroy(ds);
		exit(1);
	}
	OGR_L_ResetReading(lyr);
	
	// Collect the group id's of the features.
	gid = malloc(npts * sizeof(int));
	sgid = malloc(npts * sizeof(int));
	if (gid == NULL || sgid == NULL) {
		fprintf(stderr, "Error. Not enough memory available.\n");
		OGR_DS_Destroy(ds);
		exit(1);
	}
	i = 0;
	while ((feat = OGR_L_GetNextFeature(lyr)) != NULL) {
		// Get the field index for the group id attribute.
		fidx = OGR_F_GetFieldIndex(feat, group_attr);
		if (fidx < 0) {
			fprintf(stderr, "Error. Unable to find attribute '%s'\n", group_attr);
			OGR_DS_Destroy(ds);
			OGR_F_Destroy(feat);
			exit(1);
		}
		gid[i] = OGR_F_GetFieldAsInteger(feat, fidx);
		sgid[i] = gid[i];
		i++;
		OGR_F_Destroy(feat);
	}
	OGR_L_ResetReading(lyr);
	
	// Find out the number of group ids.
	qsort(sgid, npts, sizeof(int), comp_int_asc);
	ngrps = 1;
	for (i = 1; i < npts; i++) {
		if (sgid[i-1] != sgid[i]) {
			ngrps++;
		}
	}
	
	
	// Collect information about the points in each group.
	
	// Allocate first the memory for holding all these group points.
	grps = malloc(ngrps * sizeof(group));
	if (grps == NULL) {
		fprintf(stderr, "Error. Not enough memory available.\n");
		free(gid);
		free(sgid);
		OGR_DS_Destroy(ds);
		exit(1);
	}
	
	grpidx = 0;
	ngrppts = 1;
	for (i = 1; i < npts; i++) {
		if (sgid[i-1] != sgid[i]) {
			grps[grpidx] = make_group(ngrppts, sgid[i-1]);
			ngrppts = 1;
			grpidx++;
		}
		ngrppts++;
	}
	grps[grpidx] = make_group(ngrppts, sgid[i-1]);
	
	// We don't need the sorted group id's anymore.
	free(sgid);
	
	// We are now looping through all the features to extract the coordinates
	// and associate them with the different groups.
	i = 0;
	while ((feat = OGR_L_GetNextFeature(lyr)) != NULL) {
		geom = OGR_F_GetGeometryRef(feat);
		if (geom != NULL && wkbFlatten(OGR_G_GetGeometryType(geom)) == wkbPoint) {
			// Find the group with the ID of this feature.
			grpidx = find_group_with_id(grps, gid[i], ngrps);
			if (grpidx >= 0) {
				*grps[grpidx].xptr = OGR_G_GetX(geom, 0);
				grps[grpidx].xptr++;
				*grps[grpidx].yptr = OGR_G_GetY(geom, 0);
				grps[grpidx].yptr++;
			}
		}
		OGR_F_Destroy(feat);
		i++;
	}
	OGR_L_ResetReading(lyr);
	
	
	// The structure is now in place and we can start computing the distances.
	
	// We don't need the group id's anymore, and we can close the OGR datasource.
	free(gid);
	OGR_DS_Destroy(ds);
	
	// Print a header line to standard output
	fprintf(stdout, "from_id\tto_id\tpt_distance\n");
	
	// Number of threads = number of processors (or cores)
	#if defined (_OPENMP)
	omp_set_num_threads(omp_get_num_procs());
	#endif
	#pragma omp parallel for default(shared) private(i, j)
	for (i = 0; i < ngrps; i++) {
		for (j = 0; j < ngrps; j++) {
			compute_group_distance(grps[i], grps[j], median, manhattan);
		}
	}
	
	
	// Free the ressources for the group points.
	for (i = 0; i < ngrps; i++) {
		free_group(grps[i]);
	}
	
	
	return 0;
}





void compute_group_distance(group a, group b, int median, int manhattan) {
	
	int i, j, cnt;
	double *dist, *dist_ptr;
	double d;
	
	cnt = 0;
	
	if (median) {
		
		// Allocate the memory for storing all the distances.
		dist = calloc(a.npts * b.npts, sizeof(double));
		dist_ptr = dist;
		
		if (manhattan) {
			for (i = 0; i < a.npts; i++) {
				for (j = 0; j < b.npts; j++) {
					*dist_ptr = fabs(a.x[i] - b.x[j]) + fabs(a.y[i] - b.y[j]);
					dist_ptr++;
					cnt++;
				}
			}
		} else {
			for (i = 0; i < a.npts; i++) {
				for (j = 0; j < b.npts; j++) {
					*dist_ptr = sqrt( pow(a.x[i] - b.x[j], 2) + pow(a.y[i] - b.y[j], 2) );
					dist_ptr++;
					cnt++;
				}
			}
		}
		
		qsort(dist, a.npts * b.npts, sizeof(double), comp_dbl);
		if (cnt % 2 == 0) {
			d = (dist[cnt/2] + dist[(cnt/2)-1]) / 2.0f;
		} else {
			d = dist[(cnt-1)/2];
		}
		
		free(dist);
		
	} else {
		
		d = 0.0f;
		
		if (manhattan) {
			for (i = 0; i < a.npts; i++) {
				for (j = 0; j < b.npts; j++) {
					d += fabs(a.x[i] - b.x[j]) + fabs(a.y[i] - b.y[j]);
					cnt++;
				}
			}
		} else {
			for (i = 0; i < a.npts; i++) {
				for (j = 0; j < b.npts; j++) {
					d += sqrt( pow(a.x[i] - b.x[j], 2) + pow(a.y[i] - b.y[j], 2) );
					cnt++;
				}
			}
		}
		d /= (double)cnt;
	}
	
	fprintf(stdout, "%d\t%d\t%f\n", a.gid, b.gid, d);
}





group make_group(int npts, int gid) {
	group g;
	g.npts = npts;
	g.gid = gid;
	g.x = calloc(npts, sizeof(double));
	g.y = calloc(npts, sizeof(double));
	g.xptr = g.x;
	g.yptr = g.y;
	return g;
}


void free_group(group g) {
	free(g.x);
	free(g.y);
}


int find_group_with_id(group *grps, int gid, int ngrps) {
	int i;
	group g;
	for (i = 0; i < ngrps; i++) {
		g = grps[i];
		if (g.gid == gid) {
			return i;
		}
	}
	return -1;
}


int comp_int_asc(const void *i, const void *j) {
    return *(int *)i - *(int *)j;
}

int comp_dbl(const void *i, const void *j) {
	double a = *(double*)i;
	double b = *(double*)j;
	return (a > b) - (a < b);
}

