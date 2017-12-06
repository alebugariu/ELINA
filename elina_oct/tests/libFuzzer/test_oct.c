#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

int pool_size = -1;
opt_oct_t** pool;
int dim;

elina_linexpr0_t * create_linexpr0(int dim, long v1, long v2, long coeff1,
		long coeff2, long scalar_value) {
	elina_coeff_t *cst, *coeff;
	elina_linexpr0_t * linexpr0 = elina_linexpr0_alloc(ELINA_LINEXPR_SPARSE, 2);
	cst = &linexpr0->cst;

	elina_scalar_set_double(cst->val.scalar, scalar_value);

	elina_linterm_t * linterm = &linexpr0->p.linterm[0];
	linterm->dim = v1;
	coeff = &linterm->coeff;
	elina_scalar_set_double(coeff->val.scalar, coeff1);

	linterm = &linexpr0->p.linterm[1];
	linterm->dim = v2;
	coeff = &linterm->coeff;
	elina_scalar_set_double(coeff->val.scalar, coeff2);
	return linexpr0;
}

elina_linexpr0_t * create_polyhedral_linexpr0(int dim, long *values) {
	elina_coeff_t *cst, *coeff;
	elina_linexpr0_t * linexpr0 = elina_linexpr0_alloc(ELINA_LINEXPR_SPARSE,
			dim);
	cst = &linexpr0->cst;
	elina_scalar_set_double(cst->val.scalar, values[dim]);

	size_t i;
	for (i = 0; i < dim; i++) {
		elina_linterm_t * linterm = &linexpr0->p.linterm[i];
		linterm->dim = i;
		coeff = &linterm->coeff;
		elina_scalar_set_double(coeff->val.scalar, values[i]);
	}
	return linexpr0;
}

bool exists(elina_manager_t* man, opt_oct_t *octagon) {
	int i;
	for (i = 0; i <= pool_size; i++) {
		if (opt_oct_is_eq(man, pool[i], octagon)) {
			return true;
		}
	}
	return false;
}

void initialize_pool(elina_manager_t* man, opt_oct_t * top, opt_oct_t * bottom,
		int dim, FILE *fp) {

	long coefficients[3] = { 0, -1, 1 };
	long constants[4] = { 0, LONG_MIN, LONG_MAX, rand()
			% (MAX_VALUE + 1 - MIN_VALUE) + MIN_VALUE };
	int type[2] = { ELINA_CONS_SUPEQ, ELINA_CONS_EQ };

	pool = (opt_oct_t **) malloc(MAX_POOL_SIZE * sizeof(opt_oct_t *));

	pool[++pool_size] = top;
	elina_lincons0_array_t a = opt_oct_to_lincons_array(man, pool[pool_size]);
	fprintf(fp, "octagon %d: ", pool_size);
	elina_lincons0_array_fprint(fp, &a, NULL);
	pool[++pool_size] = bottom;
	a = opt_oct_to_lincons_array(man, pool[pool_size]);
	fprintf(fp, "octagon %d: ", pool_size);
	elina_lincons0_array_fprint(fp, &a, NULL);
	fflush(stdout);

	int v1, v2;
	int coeff1, coeff2;
	int i, j;
	int counter = 0;
	for (v1 = 0; v1 < dim - 1; v1++) {
		for (v2 = v1 + 1; v2 < dim; v2++) {
			for (coeff1 = 0; coeff1 < 3; coeff1++) {
				for (coeff2 = 0; coeff2 < 3; coeff2++) {
					for (i = 0; i < 4; i++) {
						for (j = 0; j < 2; j++) {
							elina_lincons0_array_t a_constraint =
									elina_lincons0_array_make(1);
							a_constraint.p[0].constyp = type[j];
							a_constraint.p[0].linexpr0 = create_linexpr0(dim,
									v1, v2, coefficients[coeff1],
									coefficients[coeff2], constants[i]);
							opt_oct_t* octagon = opt_oct_meet_lincons_array(man,
							DESTRUCTIVE, top, &a_constraint);
							if (!exists(man, octagon)) {
								pool[++pool_size] = octagon;
								a = opt_oct_to_lincons_array(man,
										pool[pool_size]);
								fprintf(fp, "octagon %d: ", pool_size);
								elina_lincons0_array_fprint(fp, &a, NULL);
								fflush(fp);
							}
						}
					}
				}
			}
		}
	}
	fprintf(fp, "Successfully initialized the pool!\n");
	fflush(fp);
}

bool make_fuzzable(void *array, size_t size, const long *data, size_t dataSize,
		unsigned int *dataIndex) {
	int numberOfElements = dataSize / sizeof(long);
	if (numberOfElements <= *dataIndex) {
		return false;
	}
	if (numberOfElements <= (*dataIndex + size)) {
		return false;
	}

	memcpy(array, data + *dataIndex, size);
	*dataIndex += size;
	return true;
}

bool create_random_variable(unsigned char *randomVariable, int dim,
		const long *data, size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	if (!make_fuzzable(randomVariable, sizeof(unsigned char), data, dataSize,
			dataIndex)) {
		return false;
	}
	if (!assume_fuzzable(
			*randomVariable >= MIN_RANDOM_VARIABLE
					&& *randomVariable <= MAX_RANDOM_VARIABLE)) {
		return false;
	}
	return true;
}

bool create_assignment(elina_linexpr0_t*** assignmentArray,
		int assignedToVariable, elina_dim_t ** tdim, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	unsigned char randomVariable;
	if (!create_random_variable(&randomVariable, dim, data, dataSize, dataIndex,
			fp)) {
		return false;
	}

	long fuzzableValues[MAX_DIM + 1];
	if (!make_fuzzable(fuzzableValues, (dim + 1) * sizeof(long), data, dataSize,
			dataIndex)) {
		return false;
	}
	int j;
	fprintf(fp, "Assignment expression: ");
	fflush(fp);
	for (j = 0; j < dim; j++) {
		if (!assume_fuzzable(
				!(randomVariable <= VAR_THRESHOLD)
						&& (fuzzableValues[j] >= MIN_VALUE
								&& fuzzableValues[j] <= MAX_VALUE))) {
			return false;
		}
		fprintf(fp, "%ld, ", fuzzableValues[j]);
		fflush(fp);
	}
	if (!assume_fuzzable(
			!(randomVariable <= VAR_THRESHOLD)
					&& (fuzzableValues[j] >= MIN_VALUE
							&& fuzzableValues[j] <= MAX_VALUE))) {
		return false;
	}
	fprintf(fp, "%ld\n", fuzzableValues[j]);
	fflush(fp);

	elina_linexpr0_t* expression = create_polyhedral_linexpr0(dim,
			fuzzableValues);
	*assignmentArray = (elina_linexpr0_t**) malloc(sizeof(elina_linexpr0_t*));
	*assignmentArray[0] = expression;

	*tdim = (elina_dim_t *) malloc(sizeof(elina_dim_t));
	*tdim[0] = assignedToVariable;
	return true;
}

bool create_number(int *number, int dimension, const long *data,
		size_t dataSize, unsigned int *dataIndex) {
	if (!make_fuzzable(number, sizeof(int), data, dataSize, dataIndex)) {
		return false;
	}
	if (!assume_fuzzable(*number >= 0 && *number < dimension)) {
		return false;
	}
	return true;
}

bool create_variable(int *variable, bool assign, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	if (!create_number(variable, dim, data, dataSize, dataIndex)) {
		return false;
	}
	if (assign) {
		fprintf(fp, "Assigned to variable: %d\n", *variable);
	} else {
		fprintf(fp, "Projected variable: %d\n", *variable);
	}
	fflush(fp);
	return true;
}

bool increase_pool(elina_manager_t* man, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {

	int nbops = NBOPS;

	fprintf(fp, "Number of operators: %d\n", nbops);
	fflush(fp);
	size_t i;
	for (i = 0; i < nbops; i++) {
		int operator;
		if (!make_fuzzable(&operator, sizeof(int), data, dataSize, dataIndex)) {
			return false;
		}
		operator = operator % 4;
		if (!assume_fuzzable(
				operator == ASSIGN || operator == MEET || operator == JOIN
						|| operator == WIDENING)) {
			return false;
		}

		switch (operator) {
		case ASSIGN: {
			fprintf(fp, "ASSIGN\n");
			fflush(fp);
			int assignedToVariable;
			if (!create_variable(&assignedToVariable, true, dim, data, dataSize,
					dataIndex, fp)) {
				return false;
			}
			elina_linexpr0_t** assignmentArray;
			elina_dim_t * tdim;

			if (!create_assignment(&assignmentArray, assignedToVariable, &tdim,
					dim, data, dataSize, dataIndex, fp)) {
				return false;
			}

			int number;
			if (!create_number(&number, pool_size, data, dataSize, dataIndex)) {
				return false;
			}
			opt_oct_t *octagon = pool[number];
			elina_lincons0_array_t a = opt_oct_to_lincons_array(man,
					pool[pool_size]);
			fprintf(fp, "octagon %d before assignment: ", number);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);

			opt_oct_t *result = opt_oct_assign_linexpr_array(man,
			DESTRUCTIVE, octagon, tdim, assignmentArray, 1,
			NULL);
			if (!exists(man, result)) {
				pool[++pool_size] = result;
			}

			free(assignmentArray);
			free(tdim);
			break;
		}
		case MEET: {
			fprintf(fp, "MEET\n");
			fflush(fp);
			int number1, number2;
			if (!create_number(&number1, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}
			if (!create_number(&number2, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}
			opt_oct_t *octagon1 = pool[number1];
			elina_lincons0_array_t a = opt_oct_to_lincons_array(man, octagon1);
			fprintf(fp, "octagon %d: ", number1);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);

			opt_oct_t *octagon2 = pool[number2];
			a = opt_oct_to_lincons_array(man, octagon2);
			fprintf(fp, "octagon %d: ", number2);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);

			opt_oct_t *result = opt_oct_meet(man, DESTRUCTIVE, octagon1,
					octagon2);
			if (!exists(man, result)) {
				pool[++pool_size] = result;
			}
			break;
		}
		case JOIN: {
			fprintf(fp, "JOIN\n");
			fflush(fp);
			int number1, number2;
			if (!create_number(&number1, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}
			if (!create_number(&number2, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}
			opt_oct_t *octagon1 = pool[number1];
			elina_lincons0_array_t a = opt_oct_to_lincons_array(man, octagon1);
			fprintf(fp, "octagon %d: ", number1);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);

			opt_oct_t *octagon2 = pool[number2];
			a = opt_oct_to_lincons_array(man, octagon2);
			fprintf(fp, "octagon %d: ", number2);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);

			opt_oct_t *result = opt_oct_join(man, DESTRUCTIVE, octagon1,
					octagon2);
			if (!exists(man, result)) {
				pool[++pool_size] = result;
			}
			break;
		}
		case WIDENING: {
			fprintf(fp, "WIDENING\n");
			fflush(fp);
			int number1, number2;
			if (!create_number(&number1, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}
			if (!create_number(&number2, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}
			opt_oct_t *octagon1 = pool[number1];
			opt_oct_t *octagon2 = pool[number2];
			if (!assume_fuzzable(opt_oct_is_leq(man, octagon1, octagon2))) {
				return false;
			}

			elina_lincons0_array_t a = opt_oct_to_lincons_array(man, octagon1);
			fprintf(fp, "octagon %d: ", number1);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);

			a = opt_oct_to_lincons_array(man, octagon2);
			fprintf(fp, "octagon %d: ", number2);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);

			opt_oct_t *result = opt_oct_widening(man, octagon1, octagon2);
			if (!exists(man, result)) {
				pool[++pool_size] = result;
				elina_lincons0_array_t a = opt_oct_to_lincons_array(man,
						pool[pool_size]);
				fprintf(fp, "octagon %d: ", pool_size);
				elina_lincons0_array_fprint(fp, &a, NULL);
				fflush(fp);
			}
			break;
		}
		}
	}
	return true;
}

bool create_pool(elina_manager_t* man, opt_oct_t * top, opt_oct_t * bottom,
		int dim, const long *data, size_t dataSize, unsigned int *dataIndex,
		FILE *fp) {
	if (pool_size == -1) {
		initialize_pool(man, top, bottom, dim, fp);
	}
	return increase_pool(man, dim, data, dataSize, dataIndex, fp);
}

bool get_octagon_from_pool(opt_oct_t** octagon, const long *data,
		size_t dataSize, unsigned int *dataIndex) {
	int number;
	if (!create_number(&number, pool_size, data, dataSize, dataIndex)) {
		return false;
	}
	*octagon = pool[number];
	return true;
}

bool assume_fuzzable(bool condition) {
	return (condition == true);
}

int create_dimension(FILE *fp) {
	if (pool_size == -1) {
		dim = rand() % (MAX_DIM + 1 - MIN_DIM) + MIN_DIM;
	}
	fprintf(fp, "Dim: %d\n", dim);
	fflush(fp);
	return dim;
}

void free_pool(elina_manager_t* man) {
	int i;
	for (i = 0; i < pool_size; i++) {
		opt_oct_free(man, pool[i]);
	}
	pool_size = -1;
	free(pool);
}
