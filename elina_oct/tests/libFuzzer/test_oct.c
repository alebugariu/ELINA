#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

elina_linexpr0_t * create_linexpr0(int dim, long v1, long v2, long coeff1,
		long coeff2, long scalar_value) {
	elina_coeff_t *cst, *coeff;
	elina_linexpr0_t * linexpr0 = elina_linexpr0_alloc(ELINA_LINEXPR_SPARSE, 2);
	cst = &linexpr0->cst;

	elina_scalar_set_to_int(cst->val.scalar, scalar_value, ELINA_SCALAR_DOUBLE);

	elina_linterm_t * linterm = &linexpr0->p.linterm[0];
	linterm->dim = v1;
	coeff = &linterm->coeff;
	elina_scalar_set_to_int(coeff->val.scalar, coeff1, ELINA_SCALAR_DOUBLE);

	linterm = &linexpr0->p.linterm[1];
	linterm->dim = v2;
	coeff = &linterm->coeff;
	elina_scalar_set_to_int(coeff->val.scalar, coeff2, ELINA_SCALAR_DOUBLE);
	return linexpr0;
}

bool create_number_of_constraints(long *nbcons, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	if (!make_fuzzable(nbcons, sizeof(nbcons), data, dataSize, dataIndex)) {
		return false;
	}
	if (!assume_fuzzable(
			*nbcons >= MIN_NBCONS && *nbcons <= MAX_NBCONS && *nbcons >= dim)) {
		return false;
	}
	fprintf(fp, "Number of constraints: %ld\n", *nbcons);
	fflush(fp);
	return true;
}

bool create_a_constraint(elina_linexpr0_t** constraint, elina_constyp_t *type,
		int dim, unsigned char randomVariable, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	size_t i;
	if (!make_fuzzable(type, sizeof(*type), data, dataSize, dataIndex)) {
		return false;
	}
	if (!assume_fuzzable(*type == ELINA_CONS_SUPEQ || *type == ELINA_CONS_EQ)) {
		return false;
	}
	fprintf(fp, "Type: %c\n", *type == ELINA_CONS_EQ ? 'e' : 's');
	fflush(fp);

	long fuzzableValues[5];
	if (!make_fuzzable(fuzzableValues, 5 * sizeof(long), data, dataSize,
			dataIndex)) {
		return false;
	}
	if (!assume_fuzzable(
			fuzzableValues[0] < dim && fuzzableValues[1] < dim
					&& fuzzableValues[0] != fuzzableValues[1]
					&& fuzzableValues[0] >= 0 && fuzzableValues[1] >= 0)) {
		return false;
	}
	if (!assume_fuzzable(
			fuzzableValues[2] == 1 || fuzzableValues[2] == -1
					|| fuzzableValues[2] == 0)) {
		return false;
	}
	if (!assume_fuzzable(
			fuzzableValues[3] == 1 || fuzzableValues[3] == -1
					|| fuzzableValues[3] == 0)) {
		return false;
	}
	if (!assume_fuzzable(
			!(randomVariable <= VAR_THRESHOLD)
					&& (fuzzableValues[4] >= MIN_VALUE
							&& fuzzableValues[4] <= MAX_VALUE))) {
		return false;
	}
	fprintf(fp, "Values: %ld, %ld, %ld, %ld, %ld\n", fuzzableValues[0],
			fuzzableValues[1], fuzzableValues[2], fuzzableValues[3],
			fuzzableValues[4]);
	fflush(fp);
	*constraint = create_linexpr0(dim, fuzzableValues[0], fuzzableValues[1],
			fuzzableValues[2], fuzzableValues[3], fuzzableValues[4]);
	return true;
}

bool create_constraints(elina_lincons0_array_t *lincons0, long nbcons, int dim,
		unsigned char randomVariable, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {
	*lincons0 = elina_lincons0_array_make(nbcons);
	elina_linexpr0_t* constraint;
	elina_constyp_t type;
	size_t i;
	for (i = 0; i < nbcons; i++) {
		if (!create_a_constraint(&constraint, &type, dim, randomVariable, data,
				dataSize, dataIndex, fp)) {
			elina_lincons0_array_clear(lincons0);
			return false;
		}
		lincons0->p[i].constyp = type;
		lincons0->p[i].linexpr0 = constraint;
	}
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

	fprintf(fp, "Assignment expression: ");
	fflush(fp);
	long assignmentValues[5];
	if (!make_fuzzable(assignmentValues, 5 * sizeof(long), data, dataSize,
			dataIndex)) {
		return false;
	}
	if (!assume_fuzzable(
			assignmentValues[0] < dim && assignmentValues[1] < dim
					&& assignmentValues[0] != assignmentValues[1]
					&& assignmentValues[0] >= 0 && assignmentValues[1] >= 0)) {
		return false;
	}
	if (!assume_fuzzable(
			assignmentValues[2] == 1 || assignmentValues[2] == -1
					|| assignmentValues[2] == 0)) {
		return false;
	}
	if (!assume_fuzzable(
			assignmentValues[3] == 1 || assignmentValues[3] == -1
					|| assignmentValues[3] == 0)) {
		return false;
	}
	if (!assume_fuzzable(
			!(randomVariable <= VAR_THRESHOLD)
					&& (assignmentValues[4] >= MIN_VALUE
							&& assignmentValues[4] <= MAX_VALUE))) {
		return false;
	}
	fprintf(fp, "Values: %ld, %ld, %ld, %ld, %ld\n", assignmentValues[0],
			assignmentValues[1], assignmentValues[2], assignmentValues[3],
			assignmentValues[4]);
	fflush(fp);

	elina_linexpr0_t* expression = create_linexpr0(dim, assignmentValues[0],
			assignmentValues[1], assignmentValues[2], assignmentValues[3],
			assignmentValues[4]);
	*assignmentArray = (elina_linexpr0_t**) malloc(sizeof(elina_linexpr0_t*));
	*assignmentArray[0] = expression;

	*tdim = (elina_dim_t *) malloc(sizeof(elina_dim_t));
	*tdim[0] = assignedToVariable;
	return true;
}

bool create_variable(int *variable, bool assign, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	if (!make_fuzzable(variable, sizeof(int), data, dataSize, dataIndex)) {
		return false;
	}
	if (!assume_fuzzable(*variable >= 0 && *variable < dim)) {
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

bool create_octagon_from_top(opt_oct_t** octagon, elina_manager_t* man,
		opt_oct_t * top, int dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {
	long nbcons;
	if (!create_number_of_constraints(&nbcons, dim, data, dataSize, dataIndex,
			fp)) {
		return false;
	}

	unsigned char randomVariable;
	if (!create_random_variable(&randomVariable, dim, data, dataSize, dataIndex,
			fp)) {
		return false;
	}

	elina_lincons0_array_t constraints;
	if (!create_constraints(&constraints, nbcons, dim, randomVariable, data,
			dataSize, dataIndex, fp)) {
		return false;
	}

	*octagon = opt_oct_meet_lincons_array(man, DESTRUCTIVE, top, &constraints);
	elina_lincons0_array_clear(&constraints);
	return true;
}

bool create_octagon_from_bottom(opt_oct_t** octagon, elina_manager_t* man,
		opt_oct_t * top, opt_oct_t * bottom, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	*octagon = bottom;

	long nbcons;
	if (!create_number_of_constraints(&nbcons, dim, data, dataSize, dataIndex,
			fp)) {
		return false;
	}

	unsigned char randomVariable;
	if (!create_random_variable(&randomVariable, dim, data, dataSize, dataIndex,
			fp)) {
		return false;
	}

	elina_lincons0_array_t constraints;
	if (!create_constraints(&constraints, nbcons, dim, randomVariable, data,
			dataSize, dataIndex, fp)) {
		return false;
	}

	size_t i;
	for (i = 0; i < nbcons; i++) {
		elina_lincons0_array_t a_constraint = elina_lincons0_array_make(1);
		a_constraint.p[0].constyp = constraints.p[i].constyp;
		a_constraint.p[0].linexpr0 = constraints.p[i].linexpr0;

		opt_oct_t* meet_result = opt_oct_meet_lincons_array(man,
		DESTRUCTIVE, top, &a_constraint);

		*octagon = opt_oct_join(man, DESTRUCTIVE, *octagon, meet_result);
		opt_oct_free(man, meet_result);
	}
	elina_lincons0_array_clear(&constraints);
	return true;
}

bool create_octagon_with_assignment(opt_oct_t** octagon, elina_manager_t* man,
		opt_oct_t * top, opt_oct_t * bottom, int dim, const long *data,
		size_t dataSize, unsigned int *dataIndex, FILE *fp) {

	int top_or_bottom;
	if (!make_fuzzable(&top_or_bottom, sizeof(int), data, dataSize,
			dataIndex)) {
		return false;
	}
	if (!assume_fuzzable(top_or_bottom == TOP || top_or_bottom == BOTTOM)) {
		return false;
	}

	bool result;
	switch (top_or_bottom) {
	case TOP: {
		fprintf(fp, "Started with top!\n");
		fflush(fp);
		result = create_octagon_from_top(octagon, man, top, dim, data, dataSize,
				dataIndex, fp);
		if (result == false) {
			return false;
		}
		break;
	}
	case BOTTOM: {
		fprintf(fp, "Started with bottom!\n");
		fflush(fp);
		result = create_octagon_from_bottom(octagon, man, top, bottom, dim,
				data, dataSize, dataIndex, fp);
		if (result == false) {
			return false;
		}
		break;
	}
	}

	int nbops;
	if (!make_fuzzable(&nbops, sizeof(int), data, dataSize, dataIndex)) {
		return false;
	}
	if (!assume_fuzzable(nbops >= MIN_NBOPS && nbops <= MAX_NBOPS)) {
		return false;
	}

	fprintf(fp, "Number of operators: %d\n", nbops);
	fflush(fp);
	size_t i;
	for (i = 0; i < nbops; i++) {
		int operator;
		if (!make_fuzzable(&operator, sizeof(int), data, dataSize, dataIndex)) {
			opt_oct_free(man, *octagon);
			return false;
		}
		if (!assume_fuzzable(operator == ASSIGN || operator == PROJECT)) {
			opt_oct_free(man, *octagon);
			return false;
		}

		switch (operator) {
		case ASSIGN: {
			fprintf(fp, "ASSIGN\n");
			fflush(fp);
			int assignedToVariable;
			if (!create_variable(&assignedToVariable, true, dim, data, dataSize,
					dataIndex, fp)) {
				opt_oct_free(man, *octagon);
				return false;
			}
			elina_linexpr0_t** assignmentArray;
			elina_dim_t * tdim;

			if (!create_assignment(&assignmentArray, assignedToVariable, &tdim,
					dim, data, dataSize, dataIndex, fp)) {
				opt_oct_free(man, *octagon);
				return false;
			}
			*octagon = opt_oct_assign_linexpr_array(man,
			DESTRUCTIVE, *octagon, tdim, assignmentArray, 1,
			NULL);
			free(assignmentArray);
			free(tdim);
			break;
		}
		case PROJECT: {
			fprintf(fp, "PROJECT\n");
			fflush(fp);
			int projectedVariable;
			if (!create_variable(&projectedVariable, false, dim, data, dataSize,
					dataIndex, fp)) {
				opt_oct_free(man, *octagon);
				return false;
			}
			elina_dim_t * tdim = (elina_dim_t *) malloc(sizeof(elina_dim_t));
			tdim[0] = projectedVariable;
			*octagon = opt_oct_forget_array(man,
			DESTRUCTIVE, *octagon, tdim, 1, false);
			free(tdim);
			break;
		}
		}
	}
	fprintf(fp, "Successfully created!\n");
	fflush(fp);
	return true;
}

bool create_octagon_as_random_program(opt_oct_t** octagon, elina_manager_t* man,
		opt_oct_t * top, int dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {

	int nbops;
	if (!make_fuzzable(&nbops, sizeof(int), data, dataSize, dataIndex)) {
		return false;
	}
	if (!assume_fuzzable(nbops >= MIN_NBOPS + 1 && nbops <= MAX_NBOPS)) {
		return false;
	}

	*octagon = top;

	fprintf(fp, "Number of operators: %d\n", nbops);
	fflush(fp);
	size_t i;
	for (i = 0; i < nbops; i++) {
		int operator;
		if (!make_fuzzable(&operator, sizeof(int), data, dataSize, dataIndex)) {
			if (!opt_oct_is_top(man, *octagon)) {
				opt_oct_free(man, *octagon);
			}
			return false;
		}
		if (!assume_fuzzable(
				operator == ASSIGN || operator == PROJECT || operator == MEET
						|| operator == JOIN || operator == WIDENING)) {
			if (!opt_oct_is_top(man, *octagon)) {
				opt_oct_free(man, *octagon);
			}
			return false;
		}

		switch (operator) {
		case ASSIGN: {
			fprintf(fp, "ASSIGN\n");
			fflush(fp);
			int assignedToVariable;
			if (!create_variable(&assignedToVariable, true, dim, data, dataSize,
					dataIndex, fp)) {
				if (!opt_oct_is_top(man, *octagon)) {
					opt_oct_free(man, *octagon);
				}
				return false;
			}
			elina_linexpr0_t** assignmentArray;
			elina_dim_t * tdim;

			if (!create_assignment(&assignmentArray, assignedToVariable, &tdim,
					dim, data, dataSize, dataIndex, fp)) {
				if (!opt_oct_is_top(man, *octagon)) {
					opt_oct_free(man, *octagon);
				}
				return false;
			}
			*octagon = opt_oct_assign_linexpr_array(man,
			DESTRUCTIVE, *octagon, tdim, assignmentArray, 1,
			NULL);
			free(assignmentArray);
			free(tdim);
			break;
		}
		case PROJECT: {
			fprintf(fp, "PROJECT\n");
			fflush(fp);
			int projectedVariable;
			if (!create_variable(&projectedVariable, false, dim, data, dataSize,
					dataIndex, fp)) {
				if (!opt_oct_is_top(man, *octagon)) {
					opt_oct_free(man, *octagon);
				}
				return false;
			}
			elina_dim_t * tdim = (elina_dim_t *) malloc(sizeof(elina_dim_t));
			tdim[0] = projectedVariable;
			*octagon = opt_oct_forget_array(man,
			DESTRUCTIVE, *octagon, tdim, 1, false);
			free(tdim);
			break;
		}
		case MEET: {
			fprintf(fp, "MEET\n");
			fflush(fp);
			opt_oct_t *other;
			if (!create_octagon_from_top(&other, man, top, dim, data, dataSize,
					dataIndex, fp)) {
				if (!opt_oct_is_top(man, *octagon)) {
					opt_oct_free(man, *octagon);
				}
				return false;
			}
			*octagon = opt_oct_meet(man, DESTRUCTIVE, *octagon, other);
			break;
		}
		case JOIN: {
			fprintf(fp, "JOIN\n");
			fflush(fp);
			opt_oct_t *other;
			if (!create_octagon_from_top(&other, man, top, dim, data, dataSize,
					dataIndex, fp)) {
				if (!opt_oct_is_top(man, *octagon)) {
					opt_oct_free(man, *octagon);
				}
				return false;
			}
			*octagon = opt_oct_join(man, DESTRUCTIVE, *octagon, other);
			break;
		}
		case WIDENING: {
			fprintf(fp, "WIDENING\n");
			fflush(fp);
			opt_oct_t *other;
			if (!create_octagon_from_top(&other, man, top, dim, data, dataSize,
					dataIndex, fp)) {
				if (!opt_oct_is_top(man, *octagon)) {
					opt_oct_free(man, *octagon);
				}
				return false;
			}
			if (!assume_fuzzable(opt_oct_is_leq(man, *octagon, other))) {
				if (!opt_oct_is_top(man, *octagon)) {
					opt_oct_free(man, *octagon);
				}
				return false;
			}
			*octagon = opt_oct_widening(man, *octagon, other);
			break;
		}
		}
	}
	fprintf(fp, "Successfully created!\n");
	fflush(fp);
	return true;
}

bool create_octagon(opt_oct_t** octagon, elina_manager_t* man, opt_oct_t * top,
		opt_oct_t * bottom, int dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {
	switch (CONSTRUCTION_METHOD) {
	case FROM_TOP:
		return create_octagon_from_top(octagon, man, top, dim, data, dataSize,
				dataIndex, fp);
	case WITH_ASSIGNMENT:
		return create_octagon_with_assignment(octagon, man, top, bottom, dim,
				data, dataSize, dataIndex, fp);
	case RANDOM_PROGRAM:
		return create_octagon_as_random_program(octagon, man, top, dim, data,
				dataSize, dataIndex, fp);
	}
	return false;
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

bool assume_fuzzable(bool condition) {
	return (condition == true);
}

bool make_fuzzable_dimension(int *dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {
	if (make_fuzzable(dim, sizeof(int), data, dataSize, dataIndex)) {
		if (assume_fuzzable(*dim > MIN_DIM && *dim < MAX_DIM)) {
			fprintf(fp, "Dim: %d\n", *dim);
			fflush(fp);
			return true;
		}
	}
	return false;
}
