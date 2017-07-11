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

elina_lincons0_array_t create_constraints(unsigned short int dim, uint8_t *data,
		size_t dataSize, unsigned int *dataIndex) {
	size_t i;
	size_t nbcons;

	make_fuzzable(&nbcons, sizeof(nbcons), data, dataSize, dataIndex); //number of constraints
	assume_fuzzable(
			nbcons >= MIN_NBCONS & nbcons <= MAX_NBCONS & nbcons >= dim);
	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(nbcons);

	for (i = 0; i < nbcons; i++) {
		elina_constyp_t type;
		make_fuzzable(&type, sizeof(type), data, dataSize, dataIndex); //type
		assume_fuzzable(type == ELINA_CONS_SUPEQ | type == ELINA_CONS_EQ);
		lincons0.p[i].constyp = type;

		int fuzzableValues[5];
		make_fuzzable(fuzzableValues, sizeof(fuzzableValues), data, dataSize, dataIndex);
		assume_fuzzable(
				fuzzableValues[0] < dim & fuzzableValues[1] < dim
						& fuzzableValues[0] != fuzzableValues[1]
						& fuzzableValues[0] >= 0
						& fuzzableValues[1] >= 0);
		assume_fuzzable(
				fuzzableValues[2] == 1 | fuzzableValues[2] == -1
						| fuzzableValues[2] == 0);
		assume_fuzzable(
				fuzzableValues[3] == 1 | fuzzableValues[3] == -1
						| fuzzableValues[3] == 0);
		assume_fuzzable(fuzzableValues[4] > 0);
		elina_linexpr0_t * linexpr0 = create_linexpr0(dim, fuzzableValues[0],
				fuzzableValues[1], fuzzableValues[2],
				fuzzableValues[3], fuzzableValues[4]);
		lincons0.p[i].linexpr0 = linexpr0;
	}
	return lincons0;
}

char * linexpr0_toString(elina_linexpr0_t* a) {
	size_t i;
	elina_scalar_t* pscalar = 0;
	elina_coeff_t* coeff;
	elina_dim_t dim;
	bool first;
	int sgn;

	char * constraintAsString = (char *) calloc(500, sizeof(char));
	char sign[5];
	char value[15];
	char buffer[50];
	strcpy(constraintAsString, "");
	strcpy(sign, "");
	strcpy(value, "");
	strcpy(buffer, "");

	double scalar;

	first = true;
	elina_linexpr0_ForeachLinterm(a,i,dim,coeff)
	{
		switch (coeff->discr) {
		case ELINA_COEFF_SCALAR:
			pscalar = coeff->val.scalar;
			sgn = elina_scalar_sgn(pscalar);
			if (sgn > 0) {
				scalar = pscalar->val.dbl;
				if (!first) {
					strcpy(sign, " + ");
				}
			} else {
				scalar = -pscalar->val.dbl;
				strcpy(sign, " - ");
			}
			break;
		case ELINA_COEFF_INTERVAL:
			break;
		}
		strcpy(value, "");
		strcpy(buffer, "");
		snprintf(value, 20, "%.*g", 10, scalar + 0.0);
		snprintf(buffer, 500, "%s%s%s*x%lu", constraintAsString, sign, value,
				(unsigned long) dim);
		strcpy(constraintAsString, buffer);
		first = false;
	}
	/* Constant */
	if (first || !elina_coeff_zero(&a->cst)) {
		switch (a->cst.discr) {
		case ELINA_COEFF_SCALAR:
			pscalar = a->cst.val.scalar;
			sgn = elina_scalar_sgn(pscalar);
			if (sgn >= 0) {
				scalar = pscalar->val.dbl;
				if (!first) {
					strcpy(sign, " + ");
				}
			} else {
				scalar = -pscalar->val.dbl;
				strcpy(sign, " - ");
			}
			strcpy(value, "");
			strcpy(buffer, "");
			snprintf(value, 20, "%.*g", 10, scalar + 0.0);
			snprintf(buffer, 500, "%s%s%s", constraintAsString, sign, value);
			strcpy(constraintAsString, buffer);
			break;
		case ELINA_COEFF_INTERVAL:
			break;
		}
	}
	return constraintAsString;
}

void print_constraints(elina_lincons0_array_t* array) {
	int i;
	printf("Created array of constraints of size %lu\n",
			(unsigned long) array->size);
	for (i = 0; i < array->size; i++) {
		printf("%d: %s >=0\n", i, linexpr0_toString((&array->p[i])->linexpr0));
	}
}

opt_oct_t* create_octagon(elina_manager_t* man, opt_oct_t * top,
		char * octagonNumber, unsigned short int dim, uint8_t *data, size_t dataSize, unsigned int *dataIndex) {
	elina_lincons0_array_t constraints = create_constraints(dim, octagonNumber);
	opt_oct_t* octagon = opt_oct_meet_lincons_array(man, false, top,
			&constraints);
	//printf("Created octagon %s!\n", octagonNumber);
	return octagon;
}

void make_fuzzable(void *array, size_t size, uint8_t *data, size_t dataSize,
		unsigned int *dataIndex) {
	if (dataSize <= dataIndex) {
		exit(ASSUME_FALSE);
	}
	memcopy(array, &data[dataIndex], dataIndex + size);
	*dataIndex += size;
}

void assume_fuzzable(bool condition) {
	if (!condition) {
		exit(ASSUME_FALSE);
	}
}

unsigned short int make_fuzzable_dimension(uint8_t *data, size_t dataSize,
		unsigned int *dataIndex) {
	unsigned short int dim;
	make_fuzzable(&dim, sizeof(dim), data, dataSize, dataIndex);
	assume_fuzzable(dim > MIN_DIM & dim < MAX_DIM);
	return dim;
}
