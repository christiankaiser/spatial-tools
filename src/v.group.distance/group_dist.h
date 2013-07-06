/*
 *  group_dist.h
 *  v.group.distance
 *
 *  Created by Christian Kaiser on 15.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */



/*
 * Structure for storing the points of a group.
 */
typedef struct {
	int gid;		// Group id
	int npts;		// Number of points in the group
	double *x;		// Array of x coordinates
	double *y;		// Array of y coordinates
	double *xptr;	// A pointer to the x array
	double *yptr;	// A pointer to the y array
} group;



/*
 * Function for computing the group distances.
 */
int group_dist(char *infile, char *layer, char *group_attr, int median, int manhattan);



/*
 * Computes the group distance between two groups.
 */
void compute_group_distance(group a, group b, int median, int manhattan);



group make_group(int npts, int gid);
void free_group(group g);

/*
 * Finds a group with a given id inside an array of groups.
 * Returns the index of the group, or -1 in case of no such
 * group has been found.
 */
int find_group_with_id(group *grps, int gid, int ngrps);


/*
 * Function for sorting an array of integers in ascending order.
 */
int comp_int_asc(const void *, const void *);
int comp_dbl(const void *i, const void *j);


