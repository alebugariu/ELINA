#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

elina_linexpr0_t * create_linexpr0(unsigned short int dim, int *values) {
	elina_coeff_t *cst, *coeff;
	elina_linexpr0_t * linexpr0 = elina_linexpr0_alloc(ELINA_LINEXPR_SPARSE,
			dim);
	cst = &linexpr0->cst;
	elina_scalar_set_to_int(cst->val.scalar, values[dim], ELINA_SCALAR_DOUBLE);

	size_t i;
	for (i = 0; i < dim; i++) {
		elina_linterm_t * linterm = &linexpr0->p.linterm[i];
		linterm->dim = i;
		coeff = &linterm->coeff;
		elina_scalar_set_to_int(coeff->val.scalar, values[i],
				ELINA_SCALAR_DOUBLE);
	}
	return linexpr0;
}

bool create_constraints(elina_lincons0_array_t *lincons0, int dim,
		const int *data, size_t dataSize, unsigned int *dataIndex, FILE *fp) {
	size_t i, j;
	int nbcons = MIN_NBCONS;
	if (!make_fuzzable(&nbcons, sizeof(nbcons), data, dataSize, dataIndex)) //number of constraints
			{
		return false;
	}
	if (!assume_fuzzable(
			nbcons >= MIN_NBCONS && nbcons <= MAX_NBCONS && nbcons >= dim)) {
		return false;
	}
	fprintf(fp, "Number of constraints: %d\n", nbcons);
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

		int fuzzableValues[MAX_DIM + 1];
		if (!make_fuzzable(fuzzableValues, (dim + 1) * sizeof(int), data,
				dataSize, dataIndex)) {
			return false;
		}
		fprintf(fp, "Values: ");
		for (j = 0; j < dim; j++) {
			fprintf(fp, "%d, ", fuzzableValues[j]);
		}
		fprintf(fp, "%d\n", fuzzableValues[j]);
		fflush(fp);
		elina_linexpr0_t * linexpr0 = create_linexpr0(dim, fuzzableValues);
		lincons0->p[i].linexpr0 = linexpr0;
	}
	return true;
}

bool create_polyhedron(opt_pk_array_t** polyhedron, elina_manager_t* man,
		opt_pk_array_t * top, int dim, const int *data, size_t dataSize,
		unsigned int *dataIndex, FILE *fp) {
	elina_lincons0_array_t constraints;
	if (!create_constraints(&constraints, dim, data, dataSize, dataIndex, fp)) {
		return false;
	}
	*polyhedron = opt_pk_meet_lincons_array(man, false, top, &constraints);
	return true;
}

bool make_fuzzable(void *array, size_t size, const int *data, size_t dataSize,
		unsigned int *dataIndex) {
	int numberOfElements = dataSize / sizeof(int);
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

bool make_fuzzable_dimension(int *dim, const int *data, size_t dataSize,
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
