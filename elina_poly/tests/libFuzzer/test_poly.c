#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

int pool_size = 0;
int initial_pool_size = 0;
opt_pk_array_t ** pool = NULL;
#if NBOPS
unsigned char history[NBOPS][5] = { 0 };
#else
unsigned char history[NBOPS][5];
#endif

int dim;

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

bool exists(elina_manager_t* man, opt_pk_array_t *polyhedron) {
	int i;
	for (i = 0; i < pool_size; i++) {
		if (opt_pk_is_eq(man, pool[i], polyhedron)) {
			return true;
		}
	}
	return false;
}

//uniformly distributed random number in the range [0, max)
long random_with_max(long max) {
	unsigned long num_bins = (unsigned long) max;
	unsigned long num_rand = (unsigned long) RAND_MAX + 1;
	unsigned long bin_size = num_rand / num_bins, defect = num_rand % num_bins;

	long x;
	do {
		x = random();
	} while (num_rand - defect <= (unsigned long) x);

	return x / bin_size;
}

void choose(elina_manager_t* man, opt_pk_array_t * top, long coefficients[3],
		long constants[3], int type, int totalNumber, unsigned * expectedNumber,
		int * got, int n_chosen, int len, int at, int max_types, int * counter) {
	int i;
	long count = 0;
	if (n_chosen == len) {
		if (!got) {
			return;
		}
		// do we want to add this polyhedron to the pool?
		unsigned randomValue = random_with_max(totalNumber - *counter);
		if (randomValue < *expectedNumber) {
			long initialValues[MAX_DIM + 1];
			for (i = 0; i < len; i++) {
				if (got[i] == 3) {
					initialValues[i] = rand()
							% (MAX_VALUE + 1 - MIN_VALUE)+ MIN_VALUE;
				} else {
					if (i == dim) //the constant
							{
						initialValues[i] = constants[got[i]];
					} else {
						initialValues[i] = coefficients[got[i]];
					}
				}
			}
			//here we have all the values of the new polyhedron
			elina_lincons0_array_t a_constraint = elina_lincons0_array_make(1);
			a_constraint.p[0].constyp = type;
			a_constraint.p[0].linexpr0 = create_polyhedral_linexpr0(dim,
					initialValues);
			opt_pk_array_t* polyhedron = opt_pk_meet_lincons_array(man,
			DESTRUCTIVE, top, &a_constraint);
			fflush(stdout);
			if (!opt_pk_is_bottom(man, polyhedron)
					&& !opt_pk_is_top(man, polyhedron)
					&& !exists(man, polyhedron)) {
				(*expectedNumber)--;
				elina_lincons0_array_t a = opt_pk_to_lincons_array(man,
						polyhedron);
				printf("polyhedron %d: ", pool_size);
				elina_lincons0_array_fprint(stdout, &a,
				NULL);
				fflush(stdout);
				elina_lincons0_array_clear(&a);
				pool[pool_size++] = polyhedron;
			} else {
				opt_pk_free(man, polyhedron);
			}
			elina_lincons0_array_clear(&a_constraint);
		}
		(*counter)++;
		return;
	}

	for (i = at; i < max_types; i++) {
		if (got) {
			got[n_chosen] = i;
		}
		choose(man, top, coefficients, constants, type, totalNumber,
				expectedNumber, got, n_chosen + 1, len, i, max_types, counter);
	}
}

unsigned long long number_of_combinations(unsigned long long n,
		unsigned long long k) {
	if (k > n) {
		return 0;
	}
	unsigned long long r = 1;
	for (unsigned long long d = 1; d <= k; ++d) {
		r *= n--;
		r /= d;
	}
	return r;
}

void initialize_pool(elina_manager_t* man, opt_pk_array_t * top,
		opt_pk_array_t * bottom, int dim, FILE *fp) {

	pool = (opt_pk_array_t **) malloc(
			(MAX_POOL_SIZE + NBOPS) * sizeof(opt_pk_array_t *));

	unsigned expectedNumber = MAX_POOL_SIZE - 2; // top and bottom
	unsigned long long totalNumber = 2
			* number_of_combinations(4 + dim, dim + 1);

	long coefficients[3] = { 0, -1, 1 };
	long constants[3] = { 0, LONG_MIN, LONG_MAX };
	int type[2] = { ELINA_CONS_SUPEQ, ELINA_CONS_EQ };

	int counter = 0;
	int j;
	for (j = 0; j < 2; j++) {
		int indexes[MAX_DIM + 1] = {0};
		choose(man, top, coefficients, constants, type[j], totalNumber,
				&expectedNumber, indexes, 0, dim + 1, 0, 4, &counter);
	}

	elina_lincons0_array_t a = opt_pk_to_lincons_array(man, top);
	printf("polyhedron %d: ", pool_size);
	elina_lincons0_array_fprint(stdout, &a, NULL);
	fflush(stdout);
	elina_lincons0_array_clear(&a);
	pool[pool_size++] = top;

	a = opt_pk_to_lincons_array(man, bottom);
	printf("polyhedron %d: ", pool_size);
	elina_lincons0_array_fprint(stdout, &a, NULL);
	fflush(stdout);
	elina_lincons0_array_clear(&a);
	pool[pool_size++] = bottom;

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

bool create_number(unsigned char *number, int dimension, const long *data,
		size_t dataSize, unsigned int *dataIndex) {
	if (!make_fuzzable(number, sizeof(unsigned char), data, dataSize,
			dataIndex)) {
		return false;
	}
	*number = rand() % dimension;
	return true;
}

bool create_random_conditional(elina_lincons0_array_t *conditionalArray,
		FILE *fp) {
	unsigned char randomVariable =
			rand()
					% (MAX_RANDOM_VARIABLE + 1 - MIN_RANDOM_VARIABLE)+ MIN_RANDOM_VARIABLE;

	elina_constyp_t type =
			randomVariable % 2 == ELINA_CONS_SUPEQ ?
					ELINA_CONS_SUPEQ : ELINA_CONS_EQ;

	fprintf(fp, "Conditional expression: ");
	fflush(fp);
	fprintf(fp, "Type: %c\n", type == ELINA_CONS_EQ ? 'e' : 's');
	fflush(fp);

	long fuzzableValues[MAX_DIM + 1];
	fprintf(fp, "Values: ");
	fflush(fp);

	int j;
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
	if (randomVariable <= VAR_THRESHOLD) {
		fuzzableValues[j] = rand() % (MAX_VALUE + 1 - MIN_VALUE) + MIN_VALUE;
	} else {
		fuzzableValues[j] = rand();
	}
	fprintf(fp, "%ld\n", fuzzableValues[j]);
	fflush(fp);

	*conditionalArray = elina_lincons0_array_make(1);
	elina_linexpr0_t* expression = create_polyhedral_linexpr0(dim,
			fuzzableValues);
	conditionalArray->p[0].constyp = type;
	conditionalArray->p[0].linexpr0 = expression;
	return true;
}

bool create_fuzzable_conditioanl(elina_lincons0_array_t *conditionalArray,
		int dim, const long *data, size_t dataSize, unsigned int *dataIndex,
		FILE *fp) {
	unsigned char overflowFlag;
	if (!create_number(&overflowFlag, OVERFLOW + 1, data, dataSize,
			dataIndex)) {
		return false;
	}

	elina_constyp_t type =
			overflowFlag % 2 == ELINA_CONS_SUPEQ ?
					ELINA_CONS_SUPEQ : ELINA_CONS_EQ;

	fprintf(fp, "Conditional expression: ");
	fflush(fp);
	fprintf(fp, "Type: %c\n", type == ELINA_CONS_EQ ? 'e' : 's');
	fflush(fp);

	long fuzzableValues[MAX_DIM + 1];
	if (!make_fuzzable(fuzzableValues, (dim + 1) * sizeof(long), data, dataSize,
			dataIndex)) {
		return false;
	}
	fprintf(fp, "Values: ");
	fflush(fp);

	int j;
	for (j = 0; j < dim; j++) {
		if (overflowFlag != OVERFLOW) {
			fuzzableValues[j] = fuzzableValues[j]
					% (MAX_VALUE + 1 - MIN_VALUE)+ MIN_VALUE;
		}
		fprintf(fp, "%ld, ", fuzzableValues[j]);
		fflush(fp);
	}
	if (overflowFlag != OVERFLOW) {
		fuzzableValues[j] = fuzzableValues[j]
				% (MAX_VALUE + 1 - MIN_VALUE)+ MIN_VALUE;
	}
	fprintf(fp, "%ld\n", fuzzableValues[j]);
	fflush(fp);

	*conditionalArray = elina_lincons0_array_make(1);
	elina_linexpr0_t* expression = create_polyhedral_linexpr0(dim,
			fuzzableValues);
	conditionalArray->p[0].constyp = type;
	conditionalArray->p[0].linexpr0 = expression;
	return true;
}

bool create_conditional(elina_lincons0_array_t *conditionalArray,
		const long *data, size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	if (RANDOM_CONDITIONAL) {
		return create_random_conditional(conditionalArray, fp);
	}
	return create_fuzzable_conditioanl(conditionalArray, dim, data, dataSize,
			dataIndex, fp);
}

bool create_a_constraint(int index, elina_linexpr0_t** constraint,
		elina_constyp_t *type, int dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {
	size_t i;
	if (!make_fuzzable(type, sizeof(*type), data, dataSize, dataIndex)) {
		return false;
	}
	if (!assume_fuzzable(*type == ELINA_CONS_SUPEQ || *type == ELINA_CONS_EQ)) {
		return false;
	}
	fprintf(fp, "%d. Type: %c\n", index, *type == ELINA_CONS_EQ ? 'e' : 's');
	fflush(fp);

	long fuzzableValues[MAX_DIM + 1];
	if (!make_fuzzable(fuzzableValues, (dim + 1) * sizeof(long), data, dataSize,
			dataIndex)) {
		return false;
	}
	fprintf(fp, "Values: ");
	for (i = 0; i < dim; i++) {
		/*if (!assume_fuzzable(
		 !(randomVariable <= VAR_THRESHOLD)
		 && (fuzzableValues[i] >= MIN_VALUE
		 && fuzzableValues[i] <= MAX_VALUE))) {
		 return false;
		 }*/
		fprintf(fp, "%ld, ", fuzzableValues[i]);
	}
	/*if (!assume_fuzzable(
	 !(randomVariable <= VAR_THRESHOLD)
	 && (fuzzableValues[i] >= MIN_VALUE
	 && fuzzableValues[i] <= MAX_VALUE))) {
	 return false;
	 }*/
	fprintf(fp, "%ld\n", fuzzableValues[i]);
	fflush(fp);
	*constraint = create_polyhedral_linexpr0(dim, fuzzableValues);
	return true;
}

elina_linexpr0_t* create_random_polyhedral_assignment(int dim, FILE* fp) {
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
	if (randomVariable <= VAR_THRESHOLD) {
		fuzzableValues[j] = rand() % (MAX_VALUE + 1 - MIN_VALUE) + MIN_VALUE;
	} else {
		fuzzableValues[j] = rand();
	}
	fprintf(fp, "%ld\n", fuzzableValues[j]);
	fflush(fp);
	elina_linexpr0_t* expression = create_polyhedral_linexpr0(dim,
			fuzzableValues);
	return expression;
}

bool create_fuzzable_polyhedral_assignment(elina_linexpr0_t** expression,
		int dim, const long *data, size_t dataSize, unsigned int *dataIndex,
		FILE *fp) {
	unsigned char overflowFlag;
	if (!create_number(&overflowFlag, OVERFLOW + 1, data, dataSize,
			dataIndex)) {
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
		if (overflowFlag != OVERFLOW) {
			fuzzableValues[j] = fuzzableValues[j]
					% (MAX_VALUE + 1 - MIN_VALUE)+ MIN_VALUE;
		}
		fprintf(fp, "%ld, ", fuzzableValues[j]);
		fflush(fp);
	}
	if (overflowFlag != OVERFLOW) {
		fuzzableValues[j] = fuzzableValues[j]
				% (MAX_VALUE + 1 - MIN_VALUE)+ MIN_VALUE;
	}
	fprintf(fp, "%ld\n", fuzzableValues[j]);
	fflush(fp);

	*expression = create_polyhedral_linexpr0(dim, fuzzableValues);
	return true;
}

bool create_assignment(elina_linexpr0_t*** assignmentArray,
		int assignedToVariable, elina_dim_t ** tdim, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	elina_linexpr0_t* expression;
	if (RANDOM_ASSIGNMENT) {
		expression = create_random_polyhedral_assignment(dim, fp);
	} else {
		if (!create_fuzzable_polyhedral_assignment(&expression, dim, data,
				dataSize, dataIndex, fp)) {
			return false;
		}
	}
	*assignmentArray = (elina_linexpr0_t**) malloc(sizeof(elina_linexpr0_t*));
	*assignmentArray[0] = expression;

	*tdim = (elina_dim_t *) malloc(sizeof(elina_dim_t));
	*tdim[0] = assignedToVariable;
	return true;
}

bool create_variable(unsigned char *variable, bool assign, int dim,
		const long *data, size_t dataSize, unsigned int *dataIndex, FILE *fp) {
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

bool create_fuzzable_assignments(elina_linexpr0_t** assignments,
		elina_manager_t* man, int dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {

	unsigned char overflowFlag;
	if (!create_number(&overflowFlag, OVERFLOW + 1, data, dataSize,
			dataIndex)) {
		return false;
	}

	fprintf(fp, "\n");
	int i;
	for (i = 0; i < NBASSIGNMENTS; i++) {

		long fuzzableValues[MAX_DIM + 1];
		if (!make_fuzzable(fuzzableValues, (dim + 1) * sizeof(long), data,
				dataSize, dataIndex)) {
			return false;
		}
		int j;
		fprintf(fp, "%d. Assignment expression: ", i);
		fflush(fp);
		for (j = 0; j < dim; j++) {
			if (overflowFlag != OVERFLOW) {
				fuzzableValues[j] = fuzzableValues[j]
						% (MAX_VALUE + 1 - MIN_VALUE) + MIN_VALUE;
			}
			fprintf(fp, "%ld, ", fuzzableValues[j]);
			fflush(fp);
		}
		if (overflowFlag != OVERFLOW) {
			fuzzableValues[j] = fuzzableValues[j]
					% (MAX_VALUE + 1 - MIN_VALUE)+ MIN_VALUE;
		}
		fprintf(fp, "%ld\n", fuzzableValues[j]);
		fflush(fp);

		assignments[i] = create_polyhedral_linexpr0(dim, fuzzableValues);
	}
	return true;
}

bool increase_pool(elina_manager_t* man, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {

	elina_linexpr0_t *assignments[NBASSIGNMENTS];

	if (!create_fuzzable_assignments(assignments, man, dim, data, dataSize,
			dataIndex, fp)) {
		return false;
	}
	int currentAssignmentIndex = 0;
	int resultPolyhedronIndex = 0;

	fprintf(fp, "Number of operators: %d\n", NBOPS);
	fflush(fp);
	int i;
	for (i = 0; i < NBOPS; i++) {
		unsigned char operator;
		if (!make_fuzzable(&operator, sizeof(unsigned char), data, dataSize,
				dataIndex)) {
			return false;
		}
		operator = operator % 5;
		if (!assume_fuzzable(
				operator == ASSIGN || operator == PROJECT || operator == MEET
						|| operator == JOIN || operator == WIDENING)) {
			return false;
		}

		switch (operator) {
		case ASSIGN: {
			fprintf(fp, "\n%d. ASSIGN\n", i);
			fflush(fp);

			unsigned char assignedToVariable;
			if (!create_variable(&assignedToVariable, true, dim, data, dataSize,
					dataIndex, fp)) {
				return false;
			}

			unsigned char number;
			if (!create_number(&number, pool_size, data, dataSize, dataIndex)) {
				return false;
			}

			elina_linexpr0_t** assignmentArray = (elina_linexpr0_t**) malloc(
					sizeof(elina_linexpr0_t*));
			elina_linexpr0_t* assignment_expression =
					assignments[currentAssignmentIndex];
			assignmentArray[0] = assignment_expression;
			fprintf(fp, "Assignment expression: ");
			elina_linexpr0_fprint(fp, assignment_expression, NULL);
			fflush(fp);

			elina_dim_t * tdim = (elina_dim_t *) malloc(sizeof(elina_dim_t));
			tdim[0] = assignedToVariable;

			opt_pk_array_t *polyhedron = pool[number];
			elina_lincons0_array_t a = opt_pk_to_lincons_array(man, polyhedron);
			fprintf(fp, "\npolyhedron %d before assignment: ", number);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a);
			print_polyhedron(man, polyhedron, number, fp);

			opt_pk_array_t *result = opt_pk_assign_linexpr_array(man,
			DESTRUCTIVE, polyhedron, tdim, assignmentArray, 1,
			NULL);

			if (!opt_pk_is_bottom(man, result) && !opt_pk_is_top(man, result)
					&& !opt_pk_is_eq(man, result, polyhedron)
					&& !exists(man, result)) {
				history[resultPolyhedronIndex][0] = pool_size;
				history[resultPolyhedronIndex][1] = ASSIGN;
				history[resultPolyhedronIndex][2] = assignedToVariable;
				history[resultPolyhedronIndex][3] = number;
				history[resultPolyhedronIndex++][4] = currentAssignmentIndex;
				pool[pool_size++] = result;
			} else {
				opt_pk_free(man, result);
			}

			currentAssignmentIndex++;
			if (currentAssignmentIndex == NBASSIGNMENTS) {
				currentAssignmentIndex = 0;
			}

			free(assignmentArray);
			free(tdim);
			break;
		}
		case PROJECT: {
			fprintf(fp, "\n%d. PROJECT\n", i);
			fflush(fp);
			unsigned char projectedVariable;
			if (!create_variable(&projectedVariable, false, dim, data, dataSize,
					dataIndex, fp)) {
				return false;
			}

			unsigned char number;
			if (!create_number(&number, pool_size, data, dataSize, dataIndex)) {
				return false;
			}

			opt_pk_array_t *polyhedron = pool[number];
			elina_lincons0_array_t a = opt_pk_to_lincons_array(man, polyhedron);
			fprintf(fp, "polyhedron %d before project: ", number);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a);

			elina_dim_t * tdim = (elina_dim_t *) malloc(sizeof(elina_dim_t));
			tdim[0] = projectedVariable;

			bool top1 = opt_pk_is_top(man, polyhedron);
			opt_pk_array_t *result = opt_pk_forget_array(man,
			DESTRUCTIVE, polyhedron, tdim, 1, false);

			if (!opt_pk_is_bottom(man, result) && !opt_pk_is_top(man, result)
					&& !opt_pk_is_eq(man, result, polyhedron)
					&& !exists(man, result)) {
				history[resultPolyhedronIndex][0] = pool_size;
				history[resultPolyhedronIndex][1] = PROJECT;
				history[resultPolyhedronIndex][2] = projectedVariable;
				history[resultPolyhedronIndex++][3] = number;
				pool[pool_size++] = result;
			} else {
				opt_pk_free(man, result);
			}

			free(tdim);
			break;
		}
		case MEET: {
			fprintf(fp, "\n%d. MEET\n", i);
			fflush(fp);
			unsigned char number1, number2;
			if (!create_number(&number1, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}
			if (!create_number(&number2, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}

			opt_pk_array_t *polyhedron1 = pool[number1];
			elina_lincons0_array_t a1 = opt_pk_to_lincons_array(man,
					polyhedron1);
			fprintf(fp, "polyhedron %d: ", number1);
			elina_lincons0_array_fprint(fp, &a1, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a1);

			opt_pk_array_t *polyhedron2 = pool[number2];
			elina_lincons0_array_t a2 = opt_pk_to_lincons_array(man,
					polyhedron2);
			fprintf(fp, "polyhedron %d: ", number2);
			elina_lincons0_array_fprint(fp, &a2, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a2);

			opt_pk_array_t *result = opt_pk_meet(man, DESTRUCTIVE, polyhedron1,
					polyhedron2);

			if (!opt_pk_is_bottom(man, result) && !opt_pk_is_top(man, result)
					&& !opt_pk_is_eq(man, result, polyhedron1)
					&& !opt_pk_is_eq(man, result, polyhedron2)
					&& !exists(man, result)) {
				history[resultPolyhedronIndex][0] = pool_size;
				history[resultPolyhedronIndex][1] = MEET;
				history[resultPolyhedronIndex][2] = number1;
				history[resultPolyhedronIndex++][3] = number2;
				pool[pool_size++] = result;
			} else {
				opt_pk_free(man, result);
			}
			break;
		}
		case JOIN: {
			fprintf(fp, "\n%d. JOIN\n", i);
			fflush(fp);
			unsigned char number1, number2;
			if (!create_number(&number1, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}
			if (!create_number(&number2, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}

			opt_pk_array_t *polyhedron1 = pool[number1];
			elina_lincons0_array_t a1 = opt_pk_to_lincons_array(man,
					polyhedron1);
			fprintf(fp, "polyhedron %d: ", number1);
			elina_lincons0_array_fprint(fp, &a1, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a1);

			opt_pk_array_t *polyhedron2 = pool[number2];
			elina_lincons0_array_t a2 = opt_pk_to_lincons_array(man,
					polyhedron2);
			fprintf(fp, "polyhedron %d: ", number2);
			elina_lincons0_array_fprint(fp, &a2, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a2);

			opt_pk_array_t *result = opt_pk_join(man, DESTRUCTIVE, polyhedron1,
					polyhedron2);
			if (!opt_pk_is_bottom(man, result) && !opt_pk_is_top(man, result)
					&& !opt_pk_is_eq(man, result, polyhedron1)
					&& !opt_pk_is_eq(man, result, polyhedron2)
					&& !exists(man, result)) {
				history[resultPolyhedronIndex][0] = pool_size;
				history[resultPolyhedronIndex][1] = JOIN;
				history[resultPolyhedronIndex][2] = number1;
				history[resultPolyhedronIndex++][3] = number2;
				pool[pool_size++] = result;
			} else {
				opt_pk_free(man, result);
			}
			break;
		}
		case WIDENING: {
			fprintf(fp, "\n%d. WIDENING\n", i);
			fflush(fp);
			unsigned char number1, number2;
			if (!create_number(&number1, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}
			if (!create_number(&number2, pool_size, data, dataSize,
					dataIndex)) {
				return false;
			}

			opt_pk_array_t *polyhedron1 = pool[number1];
			opt_pk_array_t *polyhedron2 = pool[number2];

			opt_pk_array_t *result;
			if (opt_pk_is_leq(man, polyhedron1, polyhedron2)) {
				elina_lincons0_array_t a1 = opt_pk_to_lincons_array(man,
						polyhedron1);
				fprintf(fp, "polyhedron %d: ", number1);
				elina_lincons0_array_fprint(fp, &a1, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a1);

				elina_lincons0_array_t a2 = opt_pk_to_lincons_array(man,
						polyhedron2);
				fprintf(fp, "polyhedron %d: ", number2);
				elina_lincons0_array_fprint(fp, &a2, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a2);

				result = opt_pk_widening(man, polyhedron1, polyhedron2);
			} else {
				elina_lincons0_array_t a1 = opt_pk_to_lincons_array(man,
						polyhedron2);
				fprintf(fp, "polyhedron %d: ", number2);
				elina_lincons0_array_fprint(fp, &a1, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a1);

				elina_lincons0_array_t a2 = opt_pk_to_lincons_array(man,
						polyhedron1);
				fprintf(fp, "polyhedron %d: ", number1);
				elina_lincons0_array_fprint(fp, &a2, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a2);

				result = opt_pk_widening(man, polyhedron2, polyhedron1);
			}

			if (!opt_pk_is_bottom(man, result) && !opt_pk_is_top(man, result)
					&& !opt_pk_is_eq(man, result, polyhedron1)
					&& !opt_pk_is_eq(man, result, polyhedron2)
					&& !exists(man, result)) {
				history[resultPolyhedronIndex][0] = pool_size;
				history[resultPolyhedronIndex][1] = WIDENING;
				history[resultPolyhedronIndex][2] = number1;
				history[resultPolyhedronIndex++][3] = number2;
				pool[pool_size++] = result;
				elina_lincons0_array_t a = opt_pk_to_lincons_array(man, result);
				fprintf(fp, "polyhedron %d: ", pool_size);
				elina_lincons0_array_fprint(fp, &a, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a);
			} else {
				opt_pk_free(man, result);
			}
			break;
		}
		}
	}
	fprintf(fp, "\n");
	fflush(fp);

	for (i = 0; i < NBASSIGNMENTS; i++) {
		free(assignments[i]);
	}
	return true;
}

void clear_history() {
	int index1, index2;
	for (index1 = 0; index1 < NBOPS; index1++) {
		for (index2 = 0; index2 < 5; index2++) {
			history[index1][index2] = 0;
		}
	}
}

void remove_old_polyhedra(elina_manager_t* man) {
	int index;
	for (index = initial_pool_size; index < pool_size; index++) {
		opt_pk_free(man, pool[index]);
	}
	pool_size = initial_pool_size;

	clear_history();
}

bool pool_is_empty() {
	return pool_size == 0;
}

bool create_pool(elina_manager_t* man, opt_pk_array_t * top,
		opt_pk_array_t * bottom, int dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {
	if (!FROM_POOL) {
		return true;
	}
	if (pool_is_empty()) {
		initialize_pool(man, top, bottom, dim, fp);
	} else {
		remove_old_polyhedra(man);
	}
	return increase_pool(man, dim, data, dataSize, dataIndex, fp);
}

void print_history(elina_manager_t* man, unsigned char number, FILE *fp) {
	if (number < initial_pool_size) {
		fprintf(fp, "polyhedron %d is from the initial pool:\n", number);
		opt_pk_array_t* polyhedron = pool[number];
		elina_lincons0_array_t a = opt_pk_to_lincons_array(man, polyhedron);
		elina_lincons0_array_fprint(fp, &a,
		NULL);
		fprintf(fp, "\n");
		fflush(fp);
		elina_lincons0_array_clear(&a);
	} else {
		fprintf(fp, "History for polyhedron %d: \n", number);
		fflush(stdout);
		int index;
		for (index = 0; index < NBOPS; index++) {
			if (history[index][0] == number) {
				switch (history[index][1]) {
				case ASSIGN:
					fprintf(fp,
							"ASSIGN [to variable x%d, assignment expression with index %d, polyhedron %d]\n",
							history[index][2], history[index][4],
							history[index][3]);
					fflush(fp);
					print_history(man, history[index][3], fp);
					break;
				case PROJECT:
					fprintf(fp, "PROJECT [variable x%d, from polyhedron %d]\n",
							history[index][2], history[index][3]);
					fflush(fp);
					print_history(man, history[index][3], fp);
					break;
				case MEET:
					fprintf(fp, "MEET [polyhedron %d, polyhedron %d]\n",
							history[index][2], history[index][3]);
					fflush(fp);
					print_history(man, history[index][2], fp);
					print_history(man, history[index][3], fp);
					break;
				case JOIN:
					fprintf(fp, "JOIN [polyhedron %d, polyhedron %d]\n",
							history[index][2], history[index][3]);
					fflush(fp);
					print_history(man, history[index][2], fp);
					print_history(man, history[index][3], fp);
					break;
				case WIDENING:
					fprintf(fp, "WIDENING [polyhedron %d, polyhedron %d]\n",
							history[index][2], history[index][3]);
					print_history(man, history[index][2], fp);
					print_history(man, history[index][3], fp);
					fflush(fp);
					break;
				}
				break;
			}
		}
	}
}

void print_polyhedron(elina_manager_t* man, opt_pk_array_t* polyhedron,
		unsigned char number, FILE *fp) {
	if (FROM_POOL) {
		print_history(man, number, fp);
	} else {
		elina_lincons0_array_t a = opt_pk_to_lincons_array(man, polyhedron);
		elina_lincons0_array_fprint(fp, &a,
		NULL);
		fflush(fp);
		elina_lincons0_array_clear(&a);
	}
}

bool create_number_of_constraints(unsigned long *nbcons, int dim,
		const long *data, size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	if (!make_fuzzable(nbcons, sizeof(nbcons), data, dataSize, dataIndex)) {
		return false;
	}
	int max = dim > MIN_NBCONS ? dim : MIN_NBCONS;
	*nbcons = *nbcons % (MAX_NBCONS + 1 - max) + max;

	fprintf(fp, "Number of constraints: %ld\n", *nbcons);
	fflush(fp);
	return true;
}

bool create_constraints(elina_lincons0_array_t *lincons0, long nbcons, int dim,
		const long *data, size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	*lincons0 = elina_lincons0_array_make(nbcons);
	elina_linexpr0_t* constraint;
	elina_constyp_t type;

	fprintf(fp, "\n");
	int i;
	for (i = 0; i < nbcons; i++) {
		if (!create_a_constraint(i, &constraint, &type, dim, data, dataSize,
				dataIndex, fp)) {
			elina_lincons0_array_clear(lincons0);
			return false;
		}
		lincons0->p[i].constyp = type;
		lincons0->p[i].linexpr0 = constraint;
	}
	fprintf(fp, "\n");
	return true;
}

bool create_polyhedron_from_top(opt_pk_array_t** polyhedron,
		elina_manager_t* man, opt_pk_array_t * top, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	unsigned long nbcons;
	if (!create_number_of_constraints(&nbcons, dim, data, dataSize, dataIndex,
			fp)) {
		return false;
	}

	elina_lincons0_array_t constraints;
	if (!create_constraints(&constraints, nbcons, dim, data, dataSize,
			dataIndex, fp)) {
		elina_lincons0_array_clear(&constraints);
		return false;
	}

	*polyhedron = opt_pk_meet_lincons_array(man, DESTRUCTIVE, top,
			&constraints);
	elina_lincons0_array_clear(&constraints);

	opt_pk_internal_t * internal_pk = opt_pk_init_from_manager(man,
			ELINA_FUNID_MEET_LINCONS_ARRAY);
	if (internal_pk->exn != ELINA_EXC_OVERFLOW) {
		return true;
	}
	opt_pk_free(man, *polyhedron);
	return false;
}

bool get_polyhedron_from_pool(opt_pk_array_t** polyhedron,
		unsigned char *number, const long *data, size_t dataSize,
		unsigned int *dataIndex) {
	if (!create_number(number, pool_size, data, dataSize, dataIndex)) {
		return false;
	}
	*polyhedron = pool[*number];
	return true;
}

bool get_polyhedron(opt_pk_array_t** polyhedron, elina_manager_t* man,
		opt_pk_array_t * top, unsigned char *number, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	if (FROM_POOL) {
		return get_polyhedron_from_pool(polyhedron, number, data, dataSize,
				dataIndex);
	}
	return create_polyhedron_from_top(polyhedron, man, top, dim, data, dataSize,
			dataIndex, fp);
}

bool assume_fuzzable(bool condition) {
	return (condition == true);
}

int create_dimension(FILE *fp) {
	if (pool_is_empty()) {
		srand(SEED);
		dim = rand() % (MAX_DIM + 1 - MIN_DIM) + MIN_DIM;
	}
	fprintf(fp, "Dim: %d\n", dim);
	fprintf(fp, "Seed: %d\n", SEED);
	fprintf(fp, "Initial pool size: %d\n", initial_pool_size);
	fflush(fp);
	return dim;
}

void free_pool(elina_manager_t* man) {
	if (FROM_POOL) {
		int index;
		for (index = 0; index < pool_size; index++) {
			opt_pk_free(man, pool[index]);
		}
		pool_size = 0;
		free(pool);

		clear_history();
	}
}

void free_polyhedron(elina_manager_t* man, opt_pk_array_t** polyhedron) {
	if (!FROM_POOL) {
		opt_pk_free(man, *polyhedron);
	}
}

