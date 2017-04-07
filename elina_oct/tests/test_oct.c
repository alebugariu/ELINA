#include <time.h>
#include <klee/klee.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include <string.h>
#include <stdio.h>

elina_linexpr0_t * create_linexpr0(unsigned short int dim, int v1, int v2,
		int coeff1, int coeff2, char * octagonNumber) {
	elina_coeff_t *cst, *coeff;
	elina_linexpr0_t * linexpr0 = elina_linexpr0_alloc(ELINA_LINEXPR_SPARSE, 2);
	cst = &linexpr0->cst;

	char buffer_scalar[80] = "scalar value for octagon ";
	elina_int_t scalar_value;
	klee_make_symbolic(&scalar_value, sizeof(scalar_value),
			strcat(buffer_scalar, octagonNumber));
	//otherwise scalar_value is concretized to 0, because klee cannot deal with floats
	klee_assume(scalar_value >= dim & scalar_value < 10);
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

bool areEqual(int line1[4], int line2[4]) {
	bool direct = (line1[0] == line2[0]) & (line1[1] == line2[1]) & (line1[2] == line2[2])
					& (line1[3] == line2[3]);
	bool commutative = (line1[0] == line2[1]) & (line1[1] == line2[0]) & (line1[2] == line2[3])
					& (line1[3] == line2[2]);
	return  direct | commutative;
}

bool areDifferent(int nbcons, int symbolicValues[nbcons][4]) {
	int i, j;
	for (i = 0; i < nbcons - 1; i++) {
		for (j = i + 1; j < nbcons; j++) {
			if (areEqual(symbolicValues[i], symbolicValues[j])) {
				return false;
			}
		}
	}
	return true;
}

elina_lincons0_array_t create_constraints(unsigned short int dim, size_t nbcons,
		char * octagonNumber) {
	size_t i;
	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(nbcons);
	char buffer_type[80] = "type for octagon ";
	char buffer_sym[80] = "symbolic variables for octagon ";
	char buffer_v1[80] = "v1 for octagon ";
	char buffer_v2[80] = "v2 for octagon ";
	char buffer_coeff1[80] = "coeff1 for octagon ";
	char buffer_coeff2[80] = "coeff2 for octagon ";
	int symbolicValues[nbcons][4];
	klee_make_symbolic(symbolicValues, sizeof(symbolicValues),
			strcat(buffer_sym, octagonNumber));
	for (i = 0; i < nbcons; i++) {
		elina_constyp_t type;
		klee_make_symbolic(&type, sizeof(type),
				strcat(buffer_type, octagonNumber));
		klee_assume(type == ELINA_CONS_SUPEQ);
		lincons0.p[i].constyp = type;
		klee_assume(
				symbolicValues[i][0] < dim & symbolicValues[i][1] < dim
						& symbolicValues[i][0] != symbolicValues[i][1]
						& symbolicValues[i][0] >= 0
						& symbolicValues[i][1] >= 0);

		klee_assume(symbolicValues[i][2] == 1 | symbolicValues[i][2] == -1);
		klee_assume(symbolicValues[i][3] == 1 | symbolicValues[i][3] == -1);

		elina_linexpr0_t * linexpr0 = create_linexpr0(dim, symbolicValues[i][0],
				symbolicValues[i][1], symbolicValues[i][2],
				symbolicValues[i][3], octagonNumber);
		lincons0.p[i].linexpr0 = linexpr0;
	}
	klee_assume(areDifferent(nbcons, symbolicValues));
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
		char * octagonNumber, unsigned short int dim, size_t nbcons) {
	elina_lincons0_array_t constraints = create_constraints(dim, nbcons,
			octagonNumber);
	print_constraints(&constraints);
	opt_oct_t* octagon = opt_oct_meet_lincons_array(man, false, top,
			&constraints);
	klee_assume(!opt_oct_is_bottom(man, octagon));
	printf("Created not bottom octagon %s!\n", octagonNumber);
	print_constraints(&constraints);
	return octagon;
}

int main(int argc, char **argv) {
	unsigned short int dim;
	size_t nbcons;
	klee_make_symbolic(&dim, sizeof(dim), "number of variables");
	klee_make_symbolic(&nbcons, sizeof(nbcons), "number of constraints");
	klee_assume(dim >= 3 & dim < 10);
	klee_assume(nbcons == 2);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	opt_oct_t* octagon1 = create_octagon(man, top, "1", dim, nbcons);
	/*klee_assert(opt_oct_is_leq(man, bottom, octagon1));
	 klee_assert(opt_oct_is_leq(man, octagon1, top));

	 //meet == glb, join == lub
	 klee_assert(opt_oct_is_eq(man, opt_oct_join(man, false, bottom, octagon1),octagon1));
	 klee_assert(opt_oct_is_eq(man, opt_oct_join(man, false, top, octagon1), top));

	 opt_oct_t* octagon2 = create_octagon(man, top, "2", dim, nbcons);
	 klee_assert(opt_oct_is_leq(man, octagon2 , opt_oct_join(man, false, octagon1, octagon2)));
	 klee_assert(opt_oct_is_leq(man, opt_oct_meet(man, false, octagon1, octagon2), octagon1));*/
	return 0;
}
