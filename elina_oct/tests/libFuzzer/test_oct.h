#ifndef TEST_OCT_H_
#define TEST_OCT_H_

#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"

#define MIN_DIM 8
#define MAX_DIM 8

#define MAX_POOL_SIZE 3*NBOPS

#define NBOPS 32
#define NBASSIGNMENTS 4
#define MIN_VALUE -9999
#define MAX_VALUE 9999
#define MIN_RANDOM_VARIABLE 0
#define MAX_RANDOM_VARIABLE 15
#define VAR_THRESHOLD 12

#define OVERFLOW 3

#define DESTRUCTIVE false
#define RANDOM_ASSIGNMENT false
#define RANDOM_CONDITIONAL false

#define ASSIGN 0
#define PROJECT 1
#define MEET 2
#define JOIN 3
#define WIDENING 4

//for widening
#define K 100
#define R(i) i < K

bool create_pool(elina_manager_t* man, opt_oct_t * top, opt_oct_t * bottom,
		int dim, const long *data, size_t dataSize, unsigned int *dataIndex,
		FILE *fp);

void free_pool(elina_manager_t* man);

bool get_octagon_from_pool(opt_oct_t** octagon, unsigned char *number, const long *data,
		size_t dataSize, unsigned int *dataIndex);

int create_dimension(FILE *fp);

bool create_assignment(elina_linexpr0_t*** assignmentArray,
		int assignedToVariable, elina_dim_t ** tdim, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp);

bool create_conditional(elina_lincons0_array_t *conditionalArray,
		const long *data, size_t dataSize, unsigned int *dataIndex, FILE *fp);

bool create_variable(unsigned char *variable, bool assign, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp);

bool assume_fuzzable(bool condition);

void print_history(elina_manager_t* man, unsigned char number, FILE *fp);

#endif /* TEST_OCT_H_ */
