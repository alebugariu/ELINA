#ifndef TEST_OCT_H_
#define TEST_OCT_H_

#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"

#define MIN_DIM 1
#define MAX_DIM 20
#define MIN_NBCONS 1
#define MAX_NBCONS 50

//for widening
#define K 3
#define R(i) i < K

bool create_octagon(opt_oct_t* octagon, elina_manager_t* man, opt_oct_t * top,
		int dim, const int *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp);

bool make_fuzzable(void *array, size_t size, const int *data, size_t dataSize,
		unsigned int *dataIndex);
bool assume_fuzzable(bool condition);

bool make_fuzzable_dimension(int * dim, const int *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp);

#endif /* TEST_OCT_H_ */
