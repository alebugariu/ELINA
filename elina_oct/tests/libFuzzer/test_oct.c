#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

int pool_size = 0;
int initial_pool_size = 0;
opt_oct_t** pool = NULL;
unsigned char history[NBOPS][5] = { 0 };

int dim;
time_t seed;

elina_linexpr0_t * create_octogonal_linexpr0(int dim, long v1, long v2,
		long coeff1, long coeff2, long scalar_value) {
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
	for (i = 0; i < pool_size; i++) {
		if (opt_oct_is_eq(man, pool[i], octagon)) {
			return true;
		}
	}
	return false;
}

void initialize_pool(elina_manager_t* man, opt_oct_t * top, opt_oct_t * bottom,
		int dim, FILE *fp) {

	seed = time(NULL);
	srand(seed);
	long coefficients[3] = { 0, -1, 1 };
	long constants[3] = { 0, LONG_MIN, LONG_MAX };
	int type[2] = { ELINA_CONS_SUPEQ, ELINA_CONS_EQ };

	pool = (opt_oct_t **) malloc(MAX_POOL_SIZE * sizeof(opt_oct_t *));

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
									constant =
											rand()
													% (MAX_VALUE + 1 - MIN_VALUE)+ MIN_VALUE;
								} else {
									constant = constants[i];
								}
								elina_lincons0_array_t a_constraint =
										elina_lincons0_array_make(1);
								a_constraint.p[0].constyp = type[j];
								a_constraint.p[0].linexpr0 =
										create_octogonal_linexpr0(dim, v1, v2,
												coefficients[coeff1],
												coefficients[coeff2],
												constants[i]);
								opt_oct_t* octagon = opt_oct_meet_lincons_array(
										man,
										DESTRUCTIVE, top, &a_constraint);
								if (!opt_oct_is_bottom(man, octagon)
										&& !opt_oct_is_top(man, octagon)
										&& !exists(man, octagon)) {
									elina_lincons0_array_t a =
											opt_oct_to_lincons_array(man,
													octagon);
									printf("octagon %d: ", pool_size);
									elina_lincons0_array_fprint(stdout, &a,
									NULL);
									fflush(stdout);
									elina_lincons0_array_clear(&a);
									pool[pool_size++] = octagon;
								} else {
									opt_oct_free(man, octagon);
								}
								elina_lincons0_array_clear(&a_constraint);
							}
						}
					}
				}
			}
		}
	}
	elina_lincons0_array_t a = opt_oct_to_lincons_array(man, top);
	printf("octagon %d: ", pool_size);
	elina_lincons0_array_fprint(stdout, &a, NULL);
	fflush(stdout);
	elina_lincons0_array_clear(&a);
	pool[pool_size++] = top;

	a = opt_oct_to_lincons_array(man, bottom);
	printf("octagon %d: ", pool_size);
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
	int resultOctagonIndex = 0;

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

			opt_oct_t *octagon = pool[number];
			elina_lincons0_array_t a = opt_oct_to_lincons_array(man, octagon);
			fprintf(fp, "\noctagon %d before assignment: ", number);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a);

			opt_oct_t *result = opt_oct_assign_linexpr_array(man,
			DESTRUCTIVE, octagon, tdim, assignmentArray, 1,
			NULL);

			if (!opt_oct_is_bottom(man, result) && !opt_oct_is_top(man, result)
					&& !opt_oct_is_eq(man, result, octagon)
					&& !exists(man, result)) {
				history[resultOctagonIndex][0] = pool_size;
				history[resultOctagonIndex][1] = ASSIGN;
				history[resultOctagonIndex][2] = assignedToVariable;
				history[resultOctagonIndex][3] = number;
				history[resultOctagonIndex++][4] = currentAssignmentIndex;
				pool[pool_size++] = result;
			} else {
				opt_oct_free(man, result);
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

			opt_oct_t *octagon = pool[number];
			elina_lincons0_array_t a = opt_oct_to_lincons_array(man, octagon);
			fprintf(fp, "octagon %d before project: ", number);
			elina_lincons0_array_fprint(fp, &a, NULL);
			fflush(fp);
			elina_lincons0_array_clear(&a);

			elina_dim_t * tdim = (elina_dim_t *) malloc(sizeof(elina_dim_t));
			tdim[0] = projectedVariable;

			bool top1 = opt_oct_is_top(man, octagon);
			opt_oct_t *result = opt_oct_forget_array(man,
			DESTRUCTIVE, octagon, tdim, 1, false);

			if (!opt_oct_is_bottom(man, result) && !opt_oct_is_top(man, result)
					&& !opt_oct_is_eq(man, result, octagon)
					&& !exists(man, result)) {
				history[resultOctagonIndex][0] = pool_size;
				history[resultOctagonIndex][1] = PROJECT;
				history[resultOctagonIndex][2] = projectedVariable;
				history[resultOctagonIndex++][3] = number;
				pool[pool_size++] = result;
			} else {
				opt_oct_free(man, result);
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

			opt_oct_t *octagon1 = pool[number1];
			elina_lincons0_array_t a1 = opt_oct_to_lincons_array(man, octagon1);
			fprintf(fp, "octagon %d: ", number1);
			elina_lincons0_array_fprint(fp, &a1, NULL);
			fflush(fp);
			/*printf("\noctagon %d: ", number1);
			 elina_lincons0_array_fprint(stdout, &a1, NULL);
			 fflush(stdout);
			 */
			elina_lincons0_array_clear(&a1);

			opt_oct_t *octagon2 = pool[number2];
			elina_lincons0_array_t a2 = opt_oct_to_lincons_array(man, octagon2);
			fprintf(fp, "octagon %d: ", number2);
			elina_lincons0_array_fprint(fp, &a2, NULL);
			fflush(fp);
			/*printf("\noctagon %d: ", number2);
			 elina_lincons0_array_fprint(stdout, &a2, NULL);
			 fflush(stdout);
			 */
			elina_lincons0_array_clear(&a2);

			bool top1 = opt_oct_is_top(man, octagon1);
			bool top2 = opt_oct_is_top(man, octagon2);

			opt_oct_t *result = opt_oct_meet(man, DESTRUCTIVE, octagon1,
					octagon2);
			/*if (opt_oct_is_top(man, octagon2) && !top2) {
			 printf("***********************ERRROOOR at MEET octagon %d\n",
			 number2);
			 fflush(stdout);
			 }
			 if (opt_oct_is_top(man, octagon1) && !top1) {
			 printf("***********************ERRROOOR at MEET octagon %d\n",
			 number1);
			 fflush(stdout);
			 }*/

			if (!opt_oct_is_bottom(man, result) && !opt_oct_is_top(man, result)
					&& !opt_oct_is_eq(man, result, octagon1)
					&& !opt_oct_is_eq(man, result, octagon2)
					&& !exists(man, result)) {
				history[resultOctagonIndex][0] = pool_size;
				history[resultOctagonIndex][1] = MEET;
				history[resultOctagonIndex][2] = number1;
				history[resultOctagonIndex++][3] = number2;
				pool[pool_size++] = result;
			} else {
				opt_oct_free(man, result);
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
				history[resultOctagonIndex][0] = pool_size;
				history[resultOctagonIndex][1] = JOIN;
				history[resultOctagonIndex][2] = number1;
				history[resultOctagonIndex++][3] = number2;
				pool[pool_size++] = result;
			} else {
				opt_oct_free(man, result);
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

			opt_oct_t *octagon1 = pool[number1];
			opt_oct_t *octagon2 = pool[number2];

			opt_oct_t *result;
			if (opt_oct_is_leq(man, octagon1, octagon2)) {
				elina_lincons0_array_t a1 = opt_oct_to_lincons_array(man,
						octagon1);
				fprintf(fp, "octagon %d: ", number1);
				elina_lincons0_array_fprint(fp, &a1, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a1);

				elina_lincons0_array_t a2 = opt_oct_to_lincons_array(man,
						octagon2);
				fprintf(fp, "octagon %d: ", number2);
				elina_lincons0_array_fprint(fp, &a2, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a2);

				result = opt_oct_widening(man, octagon1, octagon2);
			} else {
				elina_lincons0_array_t a1 = opt_oct_to_lincons_array(man,
						octagon2);
				fprintf(fp, "octagon %d: ", number2);
				elina_lincons0_array_fprint(fp, &a1, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a1);

				elina_lincons0_array_t a2 = opt_oct_to_lincons_array(man,
						octagon1);
				fprintf(fp, "octagon %d: ", number1);
				elina_lincons0_array_fprint(fp, &a2, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a2);

				result = opt_oct_widening(man, octagon2, octagon1);
			}

			if (!opt_oct_is_bottom(man, result) && !opt_oct_is_top(man, result)
					&& !opt_oct_is_eq(man, result, octagon1)
					&& !opt_oct_is_eq(man, result, octagon2)
					&& !exists(man, result)) {
				history[resultOctagonIndex][0] = pool_size;
				history[resultOctagonIndex][1] = WIDENING;
				history[resultOctagonIndex][2] = number1;
				history[resultOctagonIndex++][3] = number2;
				pool[pool_size++] = result;
				elina_lincons0_array_t a = opt_oct_to_lincons_array(man,
						result);
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

void remove_old_octagons(elina_manager_t* man) {
	int index;
	for (index = initial_pool_size; index < pool_size; index++) {
		opt_oct_free(man, pool[index]);
	}
	pool_size = initial_pool_size;

	clear_history();
}

bool pool_is_empty() {
	return pool_size == 0;
}

bool create_pool(elina_manager_t* man, opt_oct_t * top, opt_oct_t * bottom,
		int dim, const long *data, size_t dataSize, unsigned int *dataIndex,
		FILE *fp) {
	if (!FROM_POOL) {
		return true;
	}
	if (pool_is_empty()) {
		initialize_pool(man, top, bottom, dim, fp);
	} else {
		remove_old_octagons(man);
	}
	return increase_pool(man, dim, data, dataSize, dataIndex, fp);
}

void print_history(elina_manager_t* man, unsigned char number, FILE *fp) {
	if (number < initial_pool_size) {
		fprintf(fp, "Octagon %d is from the initial pool:\n", number);
		opt_oct_t* octagon = pool[number];
		elina_lincons0_array_t a = opt_oct_to_lincons_array(man, octagon);
		elina_lincons0_array_fprint(fp, &a,
		NULL);
		fflush(fp);
		elina_lincons0_array_clear(&a);
	} else {
		fprintf(fp, "History for octagon %d: \n", number);
		fflush(stdout);
		int index;
		for (index = 0; index < NBOPS; index++) {
			if (history[index][0] == number) {
				switch (history[index][1]) {
				case ASSIGN:
					fprintf(fp,
							"ASSIGN [to variable x%d, assignment expression with index %d, octagon %d]\n",
							history[index][2], history[index][4],
							history[index][3]);
					fflush(fp);
					print_history(man, history[index][3], fp);
					break;
				case PROJECT:
					fprintf(fp, "PROJECT [variable x%d, from octagon %d]\n",
							history[index][2], history[index][3]);
					fflush(fp);
					print_history(man, history[index][3], fp);
					break;
				case MEET:
					fprintf(fp, "MEET [octagon %d, octagon %d]\n",
							history[index][2], history[index][3]);
					fflush(fp);
					print_history(man, history[index][2], fp);
					print_history(man, history[index][3], fp);
					break;
				case JOIN:
					fprintf(fp, "JOIN [octagon %d, octagon %d]\n",
							history[index][2], history[index][3]);
					fflush(fp);
					print_history(man, history[index][2], fp);
					print_history(man, history[index][3], fp);
					break;
				case WIDENING:
					fprintf(fp, "WIDENING [octagon %d, octagon %d]\n",
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

void print_octagon(elina_manager_t* man, opt_oct_t* octagon,
		unsigned char number, FILE *fp) {
	if (FROM_POOL) {
		print_history(man, number, fp);
	} else {
		elina_lincons0_array_t a = opt_oct_to_lincons_array(man, octagon);
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

bool create_octogonal_constraint(elina_linexpr0_t** constraint,
		elina_constyp_t *type, int dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {
	unsigned char overflowFlag;
	if (!create_number(&overflowFlag, OVERFLOW + 1, data, dataSize,
			dataIndex)) {
		return false;
	}

	*type = overflowFlag % 2 == ELINA_CONS_SUPEQ ?
			ELINA_CONS_SUPEQ : ELINA_CONS_EQ;

	fprintf(fp, "Type: %c\n", type == ELINA_CONS_EQ ? 'e' : 's');
	fflush(fp);

	long fuzzableValues[5];
	if (!make_fuzzable(fuzzableValues, (5) * sizeof(long), data, dataSize,
			dataIndex)) {
		return false;
	}

	fuzzableValues[0] = fuzzableValues[0] % dim;
	fuzzableValues[1] = fuzzableValues[1] % dim;

	if (!assume_fuzzable(fuzzableValues[0] != fuzzableValues[1])) {
		return false;
	}

	int modulo1 = fuzzableValues[2] % 3;
	if (modulo1 == 2) {
		fuzzableValues[2] = -1;
	} else {
		fuzzableValues[2] = modulo1;
	}

	int modulo2 = fuzzableValues[3] % 3;
	if (modulo2 == 2) {
		fuzzableValues[3] = -1;
	} else {
		fuzzableValues[3] = modulo2;
	}

	if (overflowFlag != OVERFLOW) {
		fuzzableValues[4] = fuzzableValues[4]
				% (MAX_VALUE + 1 - MIN_VALUE)+ MIN_VALUE;
	}

	fprintf(fp, "Values: %ld, %ld, %ld, %ld, %ld\n", fuzzableValues[0],
			fuzzableValues[1], fuzzableValues[2], fuzzableValues[3],
			fuzzableValues[4]);
	fflush(fp);
	*constraint = create_octogonal_linexpr0(dim, fuzzableValues[0],
			fuzzableValues[1], fuzzableValues[2], fuzzableValues[3],
			fuzzableValues[4]);
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
		fprintf(fp, "%d. ", i);
		fflush(fp);
		if (!create_octogonal_constraint(&constraint, &type, dim, data,
				dataSize, dataIndex, fp)) {
			elina_lincons0_array_clear(lincons0);
			return false;
		}
		lincons0->p[i].constyp = type;
		lincons0->p[i].linexpr0 = constraint;
	}
	return true;
}

bool create_octagon_from_top(opt_oct_t** octagon, elina_manager_t* man,
		opt_oct_t * top, int dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {
	unsigned long nbcons;
	if (!create_number_of_constraints(&nbcons, dim, data, dataSize, dataIndex,
			fp)) {
		return false;
	}

	elina_lincons0_array_t constraints;
	if (!create_constraints(&constraints, nbcons, dim, data, dataSize,
			dataIndex, fp)) {
		return false;
	}

	*octagon = opt_oct_meet_lincons_array(man, DESTRUCTIVE, top, &constraints);
	elina_lincons0_array_clear(&constraints);
	return true;
}

bool get_octagon_from_pool(opt_oct_t** octagon, unsigned char *number,
		const long *data, size_t dataSize, unsigned int *dataIndex) {
	if (!create_number(number, pool_size, data, dataSize, dataIndex)) {
		return false;
	}
	*octagon = pool[*number];
	return true;
}

bool get_octagon(opt_oct_t** octagon, elina_manager_t* man, opt_oct_t * top,
		unsigned char *number, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {
	if (FROM_POOL) {
		return get_octagon_from_pool(octagon, number, data, dataSize, dataIndex);
	}
	return create_octagon_from_top(octagon, man, top, dim, data, dataSize,
			dataIndex, fp);
}

bool assume_fuzzable(bool condition) {
	return (condition == true);
}

int create_dimension(FILE *fp) {
	if (pool_is_empty()) {
		dim = rand() % (MAX_DIM + 1 - MIN_DIM) + MIN_DIM;
	}
	fprintf(fp, "Dim: %d\n", dim);
	fprintf(fp, "Seed: %ld\n", seed);
	fprintf(fp, "Initial pool size: %d\n", pool_size);
	fflush(fp);
	return dim;
}

void free_pool(elina_manager_t* man) {
	if (FROM_POOL) {
		int index;
		for (index = 0; index < pool_size; index++) {
			opt_oct_free(man, pool[index]);
		}
		pool_size = 0;
		free(pool);

		clear_history();
	}
}
