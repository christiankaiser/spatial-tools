

#if !defined(ST_VARIOGRAM_DEF)
#define ST_VARIOGRAM_DEF 1

enum st_variogram_model {
	ST_VGMODEL_UNKNOWN = 0,
	ST_VGMODEL_EXPONENTIAL = 1,
	ST_VGMODEL_SPHERICAL = 2,
	ST_VGMODEL_GAUSSIAN = 3,
	ST_VGMODEL_POWER = 10
};




typedef struct {
	enum st_variogram_model model;		// The model type.
	double range;
	double sill;
	double nugget;
	double power;					// Only for power model.
} st_variogram;

#endif


/**
 * Returns the value for the provided variogram at a given distance h.
 * Returns NAN if an error occurs.
 */
double st_variogram_value (st_variogram *variogram, double h);


/**
 * Variogram value implmentations for the individual models.
 */
double st_variogram_value_exponential (st_variogram *variogram, double h);
double st_variogram_value_spherical (st_variogram *variogram, double h);
double st_variogram_value_gaussian (st_variogram *variogram, double h);
double st_variogram_value_power (st_variogram *variogram, double h);




/**
 * Returns a string representation of the variogram model.
 */
const char *st_variogram_model_name (st_variogram *vg);

