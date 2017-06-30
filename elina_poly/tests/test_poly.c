#include <time.h>
#include <klee/klee.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_poly.h"
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

elina_lincons0_array_t create_constraints(unsigned short int dim,
		char * polyhedronNumber) {
	size_t i;
	size_t nbcons;

	char buffer_nbcons[80] = "number of constraints for polyhedron ";
	char buffer_type[80] = "type for polyhedron ";
	char buffer_sym[80] = "symbolic variables for polyhedron ";
	char buffer_scalar[80] = "scalar value for polyhedron ";

	klee_make_symbolic(&nbcons, sizeof(nbcons),
			strcat(buffer_nbcons, polyhedronNumber));
	klee_assume(nbcons >= MIN_NBCONS & nbcons <= MAX_NBCONS & nbcons >= dim);
	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(nbcons);

	int symbolicValues[nbcons][5];
	klee_make_symbolic(symbolicValues, sizeof(symbolicValues),
			strcat(buffer_sym, polyhedronNumber));
	for (i = 0; i < nbcons; i++) {
		elina_constyp_t type;
		klee_make_symbolic(&type, sizeof(type),
				strcat(buffer_type, polyhedronNumber));
		klee_assume(type == ELINA_CONS_SUPEQ | type == ELINA_CONS_EQ);
		lincons0.p[i].constyp = type;
		klee_assume(
				symbolicValues[i][0] < dim & symbolicValues[i][1] < dim
						& symbolicValues[i][0] != symbolicValues[i][1]
						& symbolicValues[i][0] >= 0
						& symbolicValues[i][1] >= 0);
		klee_assume(symbolicValues[i][4] > 0);
		elina_linexpr0_t * linexpr0 = create_linexpr0(dim, symbolicValues[i][0],
				symbolicValues[i][1], symbolicValues[i][2],
				symbolicValues[i][3], symbolicValues[i][4]);
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

opt_pk_array_t * create_polyhedron(elina_manager_t* man, opt_pk_array_t * top,
		char * polyhedronNumber, unsigned short int dim) {
	elina_lincons0_array_t constraints = create_constraints(dim, polyhedronNumber);
	opt_pk_array_t * polyhedron = opt_pk_meet_lincons_array(man, false, top,
			&constraints);
	//printf("Created polyhedron %s!\n", polyhedronNumber);
	return polyhedron;
}

unsigned short int make_symbolic_dimension() {
	unsigned short int dim;
	klee_make_symbolic(&dim, sizeof(dim), "number of variables");
	klee_assume(dim > MIN_DIM & dim < MAX_DIM);
	return dim;
}
