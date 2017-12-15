#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

int pool_size = -1;
int initial_pool_size = -1;
opt_oct_t** pool = NULL;
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
	for (i = 2; i <= pool_size; i++) {
		if (opt_oct_is_eq(man, pool[i], octagon)) {
			return true;
		}
	}
	return false;
}

void initialize_pool(elina_manager_t* man, opt_oct_t * top, opt_oct_t * bottom,
		int dim, FILE *fp) {

	srand(SEED);
	long coefficients[3] = { 0, -1, 1 };
	long constants[3] = { 0, LONG_MIN, LONG_MAX };
	int type[2] = { ELINA_CONS_SUPEQ, ELINA_CONS_EQ };

	pool = (opt_oct_t **) malloc(MAX_POOL_SIZE * sizeof(opt_oct_t *));

	pool[++pool_size] = top;
	pool[++pool_size] = bottom;

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
							double probability = (double) rand()
									/ (double) ((unsigned) RAND_MAX + 1);
							if (probability
									<= (NBOPS - pool_size) / pow(dim, 3)) {

								long constant;
								if (i == 3) {
									constant = rand() % (MAX_VALUE + 1 - MIN_VALUE)+ MIN_VALUE;
								} else {
									constant = constants[i];
								}
								elina_lincons0_array_t a_constraint =
										elina_lincons0_array_make(1);
								a_constraint.p[0].constyp = type[j];
								a_constraint.p[0].linexpr0 = create_linexpr0(
										dim, v1, v2, coefficients[coeff1],
										coefficients[coeff2], constants[i]);
								opt_oct_t* octagon = opt_oct_meet_lincons_array(
										man,
										DESTRUCTIVE, top, &a_constraint);
								if (!opt_oct_is_bottom(man, octagon)
										&& !opt_oct_is_top(man, octagon)
										&& !exists(man, octagon)) {
									pool[++pool_size] = octagon;
								} else {
									opt_oct_free(man, octagon);
								}
							}
						}
					}
				}
			}
		}
	}
	initial_pool_size = pool_size;
	fprintf(fp, "Successfully initialized the pool!\n");
	fflush(fp);
	printf("Initial pool size: %d\n", initial_pool_size);
	fflush(stdout);
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

void create_assignment(elina_linexpr0_t*** assignmentArray,
		int assignedToVariable, elina_dim_t ** tdim, FILE *fp) {
	unsigned char randomVariable =
			rand()
					% (MAX_RANDOM_VARIABLE + 1 - MIN_RANDOM_VARIABLE)+ MIN_RANDOM_VARIABLE;

	long fuzzableValues[MAX_DIM + 1];
	int j;
	fprintf(fp, "Assignment expression: ");
	fflush(fp);
	for (j = 0; j < dim; j++) {
		if (randomVariable <= VAR_THRESHOLD) {
			fuzzableValues[j] =
					rand() % (MAX_VALUE + 1 - MIN_VALUE) + MIN_VALUE;
		} else {
			fuzzableValues[j] = rand();
		}
		fprintf(fp, "%ld, ", fuzzableValues[j]);
		fflush(fp);
	}
	fprintf(fp, "%ld\n", fuzzableValues[j]);
	fflush(fp);
	elina_linexpr0_t* expression = create_polyhedral_linexpr0(dim,
			fuzzableValues);
	*assignmentArray = (elina_linexpr0_t**) malloc(sizeof(elina_linexpr0_t*));
	*assignmentArray[0] = expression;

	*tdim = (elina_dim_t *) malloc(sizeof(elina_dim_t));
	*tdim[0] = assignedToVariable;
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
	int i;
	for (i = 0; i < nbops; i++) {
		int operator;
		if (!make_fuzzable(&operator, sizeof(int), data, dataSize, dataIndex)) {
			return false;
		}
		operator = operator % 4;
		if (!assume_fuzzable(
				operator == ASSIGN || operator == PROJECT || operator == MEET
						|| operator == JOIN || operator == WIDENING)) {
			return false;
		}

		switch (operator) {
		case ASSIGN: {
			fprintf(fp, "%d. ASSIGN\n", i);
			fflush(fp);
			int assignedToVariable;
			if (!create_variable(&assignedToVariable, true, dim, data, dataSize,
					dataIndex, fp)) {
				return false;
			}

			int number;
			if (!create_number(&number, pool_size, data, dataSize, dataIndex)) {
				return false;
			}

			elina_linexpr0_t** assignmentArray;
			elina_dim_t * tdim;

			create_assignment(&assignmentArray, assignedToVariable, &tdim, fp);

			opt_oct_t *octagon = pool[number];
			elina_lincons0_array_t a = opt_oct_to_lincons_array(man, octagon);
			fprintf(fp, "octagon %d before assignment: ", number);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a);

			opt_oct_t *result = opt_oct_assign_linexpr_array(man,
			DESTRUCTIVE, octagon, tdim, assignmentArray, 1,
			NULL);
			if (!opt_oct_is_bottom(man, result) && !opt_oct_is_top(man, result)
					&& !opt_oct_is_eq(man, result, octagon)
					&& !exists(man, result)) {
				pool[++pool_size] = result;
			} else {
				opt_oct_free(man, result);
			}

			free(assignmentArray);
			free(tdim);
			break;
		}
		case PROJECT: {
			fprintf(fp, "%d. PROJECT\n", i);
			fflush(fp);
			int projectedVariable;
			if (!create_variable(&projectedVariable, false, dim, data, dataSize,
					dataIndex, fp)) {
				return false;
			}

			int number;
			if (!create_number(&number, pool_size, data, dataSize, dataIndex)) {
				return false;
			}
			opt_oct_t *octagon = pool[number];
			elina_lincons0_array_t a = opt_oct_to_lincons_array(man, octagon);
			fprintf(fp, "octagon %d before project: ", number);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a);

			elina_dim_t * tdim = (elina_dim_t *) malloc(sizeof(elina_dim_t));
			tdim[0] = projectedVariable;
			opt_oct_t *result = opt_oct_forget_array(man,
			DESTRUCTIVE, octagon, tdim, 1, false);

			if (!opt_oct_is_bottom(man, result) && !opt_oct_is_top(man, result)
					&& !opt_oct_is_eq(man, result, octagon)
					&& !exists(man, result)) {
				pool[++pool_size] = result;
			} else {
				opt_oct_free(man, result);
			}

			free(tdim);
			break;
		}
		case MEET: {
			fprintf(fp, "%d. MEET\n", i);
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
			elina_lincons0_array_t a1 = opt_oct_to_lincons_array(man, octagon1);
			fprintf(fp, "octagon %d: ", number1);
			elina_lincons0_array_fprint(fp, &a1, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a1);

			opt_oct_t *octagon2 = pool[number2];
			elina_lincons0_array_t a2 = opt_oct_to_lincons_array(man, octagon2);
			fprintf(fp, "octagon %d: ", number2);
			elina_lincons0_array_fprint(fp, &a2, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a2);

			opt_oct_t *result = opt_oct_meet(man, DESTRUCTIVE, octagon1,
					octagon2);
			if (!opt_oct_is_bottom(man, result) && !opt_oct_is_top(man, result)
					&& !opt_oct_is_eq(man, result, octagon1)
					&& !opt_oct_is_eq(man, result, octagon2)
					&& !exists(man, result)) {
				pool[++pool_size] = result;
			} else {
				opt_oct_free(man, result);
			}
			break;
		}
		case JOIN: {
			fprintf(fp, "%d. JOIN\n", i);
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
			elina_lincons0_array_t a1 = opt_oct_to_lincons_array(man, octagon1);
			fprintf(fp, "octagon %d: ", number1);
			elina_lincons0_array_fprint(fp, &a1, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a1);

			opt_oct_t *octagon2 = pool[number2];
			elina_lincons0_array_t a2 = opt_oct_to_lincons_array(man, octagon2);
			fprintf(fp, "octagon %d: ", number2);
			elina_lincons0_array_fprint(fp, &a2, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a2);

			opt_oct_t *result = opt_oct_join(man, DESTRUCTIVE, octagon1,
					octagon2);
			if (!opt_oct_is_bottom(man, result) && !opt_oct_is_top(man, result)
					&& !opt_oct_is_eq(man, result, octagon1)
					&& !opt_oct_is_eq(man, result, octagon2)
					&& !exists(man, result)) {
				pool[++pool_size] = result;
			} else {
				opt_oct_free(man, result);
			}
			break;
		}
		case WIDENING: {
			fprintf(fp, "%d. WIDENING\n", i);
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
			if (!opt_oct_is_leq(man, octagon1, octagon2)) {
				opt_oct_t * aux = octagon1;
				octagon1 = octagon2;
				octagon2 = aux;
				int auxNumber = number1;
				number1 = number2;
				number2 = auxNumber;
			}

			elina_lincons0_array_t a1 = opt_oct_to_lincons_array(man, octagon1);
			fprintf(fp, "octagon %d: ", number1);
			elina_lincons0_array_fprint(fp, &a1, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a1);

			elina_lincons0_array_t a2 = opt_oct_to_lincons_array(man, octagon2);
			fprintf(fp, "octagon %d: ", number2);
			elina_lincons0_array_fprint(fp, &a2, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a2);

			opt_oct_t *result = opt_oct_widening(man, octagon1, octagon2);
			if (!opt_oct_is_bottom(man, result) && !opt_oct_is_top(man, result)
					&& !opt_oct_is_eq(man, result, octagon1)
					&& !opt_oct_is_eq(man, result, octagon2)
					&& !exists(man, result)) {
				pool[++pool_size] = result;
				elina_lincons0_array_t a = opt_oct_to_lincons_array(man,
						pool[pool_size]);
				fprintf(fp, "octagon %d: ", pool_size);
				elina_lincons0_array_fprint(fp, &a, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a);
			} else {
				opt_oct_free(man, result);
			}
			break;
		}
		}
	}
	return true;
}

void remove_old_octagons(elina_manager_t* man) {
	int index;
	for (index = initial_pool_size + 1; index < pool_size; index++) {
		opt_oct_free(man, pool[index]);
	}
	pool_size = initial_pool_size;
}

bool create_pool(elina_manager_t* man, opt_oct_t * top, opt_oct_t * bottom,
		int dim, const long *data, size_t dataSize, unsigned int *dataIndex,
		FILE *fp) {
	if (pool_size == -1) {
		initialize_pool(man, top, bottom, dim, fp);
	} else {
		remove_old_octagons(man);
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
	fprintf(fp, "Initial pool size: %d\n", pool_size);
	fflush(fp);
	return dim;
}

void free_pool(elina_manager_t* man) {
	int index;
	for (index = 0; index < pool_size; index++) {
		opt_oct_free(man, pool[index]);
	}
	pool_size = -1;
	free(pool);
}
