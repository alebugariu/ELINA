#ifndef TEST_POLY_H_
#define TEST_POLY_H_

#define MIN_DIM 1
#define MAX_DIM 20
#define MIN_NBCONS 1
#define MAX_NBCONS 50

#include "opt_pk.h"

//for widening
#define K 100
#define R(i) i < K

bool create_polyhedron(opt_pk_array_t** polyhedron, elina_manager_t* man, opt_pk_array_t * top,
		long dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp);

bool make_fuzzable(void *array, size_t size, const long *data, size_t dataSize,
		unsigned int *dataIndex);
bool assume_fuzzable(bool condition);

bool make_fuzzable_dimension(long * dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp);

#endif /* TEST_POLY_H_ */
