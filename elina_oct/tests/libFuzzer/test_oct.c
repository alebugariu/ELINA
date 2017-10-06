#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

elina_linexpr0_t * create_linexpr0(long dim, long v1, long v2, long coeff1,
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

bool create_constraints(elina_lincons0_array_t *lincons0, long dim,
		const long *data, size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	size_t i;
	long nbcons = MIN_NBCONS;
	if (!make_fuzzable(&nbcons, sizeof(nbcons), data, dataSize, dataIndex)) //number of constraints
			{
		return false;
	}
	if (!assume_fuzzable(
			nbcons >= MIN_NBCONS && nbcons <= MAX_NBCONS && nbcons >= dim)) {
		return false;
	}
	fprintf(fp, "Number of constraints: %ld\n", nbcons);
	fflush(fp);
	*lincons0 = elina_lincons0_array_make(nbcons);
	for (i = 0; i < nbcons; i++) {
		elina_constyp_t type;
		if (!make_fuzzable(&type, sizeof(type), data, dataSize, dataIndex)) //type
				{
			return false;
		}
		if (!assume_fuzzable(
				type == ELINA_CONS_SUPEQ || type == ELINA_CONS_EQ)) {
			return false;
		}
		fprintf(fp, "Type: %c\n", type == 0 ? 'e' : 's');
		fflush(fp);
		lincons0->p[i].constyp = type;

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
		fprintf(fp, "Values: %ld, %ld, %ld, %ld, %ld\n", fuzzableValues[0],
				fuzzableValues[1], fuzzableValues[2], fuzzableValues[3],
				fuzzableValues[4]);
		fflush(fp);
		elina_linexpr0_t * linexpr0 = create_linexpr0(dim, fuzzableValues[0],
				fuzzableValues[1], fuzzableValues[2], fuzzableValues[3],
				fuzzableValues[4]);
		lincons0->p[i].linexpr0 = linexpr0;
	}
	return true;
}

bool create_octagon(opt_oct_t** octagon, elina_manager_t* man, opt_oct_t * top,
		long dim, const long *data, size_t dataSize, unsigned int *dataIndex,
		FILE *fp) {
	elina_lincons0_array_t constraints;
	if (!create_constraints(&constraints, dim, data, dataSize, dataIndex, fp)) {
		return false;
	}
	*octagon = opt_oct_meet_lincons_array(man, false, top, &constraints);
	return true;
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
	return condition;
}

bool make_fuzzable_dimension(long *dim, const long *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {
	if (make_fuzzable(dim, sizeof(int), data, dataSize, dataIndex)) {
		if (assume_fuzzable(*dim > MIN_DIM && *dim < MAX_DIM)) {
			fprintf(fp, "Dim: %ld\n", *dim);
			fflush(fp);
			return true;
		}
	}
	return false;
}
