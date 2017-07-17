#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

elina_linexpr0_t * create_linexpr0(unsigned short int dim, int v1, int v2,
		int coeff1, int coeff2, int scalar_value) {
	elina_coeff_t *cst, *coeff;
	elina_linexpr0_t * linexpr0 = elina_linexpr0_alloc(ELINA_LINEXPR_SPARSE, 2);
	cst = &linexpr0->cst;

	elina_scalar_reinit(cst->val.scalar, ELINA_SCALAR_DOUBLE);
	cst->val.scalar->val.dbl = (double) scalar_value;

	elina_linterm_t * linterm = &linexpr0->p.linterm[0];
	linterm->dim = v1;
	coeff = &linterm->coeff;
	elina_scalar_reinit(coeff->val.scalar, ELINA_SCALAR_DOUBLE);
	coeff->val.scalar->val.dbl = (double) coeff1;

	linterm = &linexpr0->p.linterm[1];
	linterm->dim = v2;
	coeff = &linterm->coeff;
	elina_scalar_reinit(coeff->val.scalar, ELINA_SCALAR_DOUBLE);
	coeff->val.scalar->val.dbl = (double) coeff2;
	return linexpr0;
}

bool create_constraints(elina_lincons0_array_t *lincons0, unsigned short int dim,
		const uint64_t *data, size_t dataSize, unsigned int *dataIndex) {
	size_t i;
	size_t nbcons;

	if (!make_fuzzable(&nbcons, sizeof(nbcons), data, dataSize, dataIndex)) //number of constraints
			{
		return false;
	}
	if (!assume_fuzzable(
			nbcons >= MIN_NBCONS & nbcons <= MAX_NBCONS & nbcons >= dim)) {
		return false;
	}
	*lincons0 = elina_lincons0_array_make(nbcons);
	for (i = 0; i < nbcons; i++) {
		elina_constyp_t type;
		if (!make_fuzzable(&type, sizeof(type), data, dataSize, dataIndex)) //type
				{
			return false;
		}
		if (!assume_fuzzable(
				type == ELINA_CONS_SUPEQ | type == ELINA_CONS_EQ)) {
			return false;
		}
		lincons0->p[i].constyp = type;

		int fuzzableValues[5];
		if (!make_fuzzable(fuzzableValues, sizeof(fuzzableValues), data,
				dataSize, dataIndex)) {
			return false;
		}
		if (!assume_fuzzable(
				fuzzableValues[0] < dim & fuzzableValues[1] < dim
						& fuzzableValues[0] != fuzzableValues[1]
						& fuzzableValues[0] >= 0 & fuzzableValues[1] >= 0)) {
			return false;
		}
		if (!assume_fuzzable(
				fuzzableValues[2] == 1 | fuzzableValues[2] == -1
						| fuzzableValues[2] == 0)) {
			return false;
		}
		if (!assume_fuzzable(
				fuzzableValues[3] == 1 | fuzzableValues[3] == -1
						| fuzzableValues[3] == 0)) {
			return false;
		}
		if (!assume_fuzzable(fuzzableValues[4] > 0)) {
			return false;
		}
		elina_linexpr0_t * linexpr0 = create_linexpr0(dim, fuzzableValues[0],
				fuzzableValues[1], fuzzableValues[2], fuzzableValues[3],
				fuzzableValues[4]);
		lincons0->p[i].linexpr0 = linexpr0;
	}
	return true;
}

bool create_octagon(opt_oct_t* octagon, elina_manager_t* man, opt_oct_t * top,
		unsigned short int dim, const uint64_t *data, size_t dataSize,
		unsigned int *dataIndex) {
	elina_lincons0_array_t constraints;
	if (!create_constraints(&constraints, dim, data, dataSize,
			dataIndex)) {
		return false;
	}
	octagon = opt_oct_meet_lincons_array(man, false, top, &constraints);
	return true;
}

bool make_fuzzable(void *array, size_t size, const uint64_t *data,
		size_t dataSize, unsigned int *dataIndex) {
	if (dataSize <= *dataIndex + size) {
		return false;
	}
	memcpy(array, &data[*dataIndex], size);
	*dataIndex += size;
	return true;
}

bool assume_fuzzable(bool condition) {
	return condition;
}

bool make_fuzzable_dimension(size_t *dim, const uint64_t *data,
		size_t dataSize, unsigned int *dataIndex) {
	if (make_fuzzable(dim, sizeof(dim), data, dataSize, dataIndex)) {
		if (assume_fuzzable(*dim > MIN_DIM && *dim < MAX_DIM)) {
			return true;
		}
	}
	return false;
}
