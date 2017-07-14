#ifndef TEST_OCT_H_
#define TEST_OCT_H_

#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"

#define MIN_DIM 1
#define MAX_DIM 20
#define MIN_NBCONS 1
#define MAX_NBCONS 50
#define ASSUME_FALSE -5
#define ASSERTION_FAILED -10

//for widening
#define K 3
#define R(i) i < K

opt_oct_t* create_octagon(elina_manager_t* man, opt_oct_t * top,
		char * octagonNumber, unsigned short int dim, const uint8_t *data,
		size_t dataSize, unsigned int *dataIndex);

void make_fuzzable(void *array, size_t size, const uint8_t *data, size_t dataSize,
		unsigned int *dataIndex);
void assume_fuzzable(bool condition);

unsigned short int make_fuzzable_dimension(const uint8_t *data, size_t dataSize,
		unsigned int *dataIndex);

#endif /* TEST_OCT_H_ */
