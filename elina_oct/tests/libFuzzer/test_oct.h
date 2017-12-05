#ifndef TEST_OCT_H_
#define TEST_OCT_H_

#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"

#define MIN_DIM 2
#define MAX_DIM 20

#define MAX_POOL_SIZE 20000

#define MIN_NBOPS 0
#define MAX_NBOPS 5
#define MIN_VALUE -9999
#define MAX_VALUE 9999
#define MIN_RANDOM_VARIABLE 0
#define MAX_RANDOM_VARIABLE 15
#define VAR_THRESHOLD 12

#define DESTRUCTIVE false

#define ASSIGN 0
#define MEET 1
#define JOIN 2
#define WIDENING 3

//for widening
#define K 100
#define R(i) i < K

bool create_pool(elina_manager_t* man, opt_oct_t * top, opt_oct_t * bottom,
		int dim, const long *data, size_t dataSize, unsigned int *dataIndex,
		FILE *fp);

void free_pool(elina_manager_t* man);

bool get_octagon_from_pool(opt_oct_t** octagon, const long *data, size_t dataSize,
		unsigned int *dataIndex);

int create_dimension(FILE *fp);

bool assume_fuzzable(bool condition);

#endif /* TEST_OCT_H_ */
