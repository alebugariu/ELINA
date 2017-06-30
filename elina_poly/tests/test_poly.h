#ifndef TEST_POLY_H_
#define TEST_POLY_H_

#define MIN_DIM 1
#define MAX_DIM 20
#define MIN_NBCONS 1
#define MAX_NBCONS 50

#include "opt_pk.h"

//for widening
#define K 3
#define R(i) i < K

opt_pk_array_t * create_polyhedron(elina_manager_t* man, opt_pk_array_t * top,
		char * polyhedronNumber, unsigned short int dim);

unsigned short int make_symbolic_dimension();

#endif /* TEST_POLY_H_ */
