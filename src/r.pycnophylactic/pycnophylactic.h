

/*
 * pycnophylactic
 */
int pycnophylactic(char *frast, char *vrast, char *orast, int density, int maxIterations);



void exportMatrixDouble(double *matrix, int sizex, int sizey, char *fpath);


int findNumberOfRegions(int* fmatrix, int sizex, int sizey);
void computeDensity(int *fmatrix, double *vmatrix, int *pixelsPerRegion, int sizex, int sizey);
void computePopulationTotals(double *pop, int* fmatrix, double *vmatrix, int sizex, int sizey);
void numberOfPixelsPerRegion(int *pixelsPerRegion, int *fmatrix, int sizex, int sizey);

void computeAdjustmentForSmoothness(double *smatrix, double *vmatrix, int *fmatrix, int sizex, int sizey);
void applyDecrementingFactorPerRegion(double *smatrix, int *fmatrix, int sizex, int sizey, int *pixelsPerRegion, int nregions);

void matrixSum(double *res, double *factor, int sizex, int sizey, double minvalue);
void enforcePycnophylacticConstraint(double *pop, double *pop_new, int *fmatrix, double *vmatrix, int sizex, int sizey, int *pixelsPerRegion, int nregions);
