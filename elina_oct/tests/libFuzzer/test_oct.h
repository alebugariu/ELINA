#ifndef TEST_OCT_H_
#define TEST_OCT_H_

#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"

#define MIN_DIM 1
#define MAX_DIM 20
#define MIN_NBCONS 1
#define MAX_NBCONS 50
#define MIN_NBOPS 0
#define MAX_NBOPS 5
#define MIN_VALUE -9999
#define MAX_VALUE 9999
#define MIN_RANDOM_VARIABLE 0
#define MAX_RANDOM_VARIABLE 15
#define VAR_THRESHOLD 12

#define DESTRUCTIVE false

#define FROM_TOP 1
#define WITH_ASSIGNMENT 2
#define RANDOM_PROGRAM 3
#define CONSTRUCTION_METHOD RANDOM_PROGRAM

#define TOP 1
#define BOTTOM 2

#define ASSIGN 1
#define PROJECT 2
#define MEET 3
#define JOIN 4
#define WIDENING 5

//for widening
#define K 100
#define R(i) i < K

bool create_octagon(opt_oct_t** octagon, elina_manager_t* man, opt_oct_t * top,
		opt_oct_t * bottom, int dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp);

bool create_variable(int *variable, bool assign, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp);

bool create_assignment(elina_linexpr0_t*** assignmentArray,
		int assignedToVariable, elina_dim_t ** tdim, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp);

bool create_conditional(elina_lincons0_array_t *conditionalExpression, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp);

bool make_fuzzable(void *array, size_t size, const long *data, size_t dataSize,
		unsigned int *dataIndex);
bool assume_fuzzable(bool condition);

bool make_fuzzable_dimension(int * dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp);

#endif /* TEST_OCT_H_ */
