#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include <string.h>
#include <stdio.h>

bool is_meet_compatible_direct(elina_manager_t * man, opt_oct_t * x, opt_oct_t * y) {
	if (opt_oct_is_leq(man, x, y)) {
		return opt_oct_is_eq(man, opt_oct_meet(man, false, x, y), x);
	}
	return true;
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

int main(int argc, char **argv) {
	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, 2, 0);

	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(1);
    lincons0.p[0].constyp = ELINA_CONS_SUPEQ;
    elina_linexpr0_t * linexpr0 = create_linexpr0(2, 0, 1, 1, 1, 4294967167);
	lincons0.p[0].linexpr0 = linexpr0;
    print_constraints(&lincons0);
	opt_oct_t* octagon1 = opt_oct_meet_lincons_array(man, false, top,
				&lincons0);
	opt_oct_mat_t *oo1 = octagon1->closed ? octagon1->closed : octagon1->m;
	printf("First: %d\n",oo1==octagon1->closed);
	print_opt_hmat(oo1->mat,2);

	elina_lincons0_array_t lincons1 = elina_lincons0_array_make(1);
	lincons1.p[0].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr1 = create_linexpr0(2, 1, 0, 1, 1, 4294967167);
	lincons1.p[0].linexpr0 = linexpr1;
	print_constraints(&lincons1);
	opt_oct_t* octagon2 = opt_oct_meet_lincons_array(man, false, top,
					&lincons1);
	opt_oct_mat_t *oo2 = octagon2->closed ? octagon2->closed : octagon2->m;
	printf("Second: %d\n",oo2==octagon2->closed);
	print_opt_hmat(oo2->mat,2);

	opt_oct_t* result = opt_oct_meet(man, false, octagon1, octagon2);
	opt_oct_mat_t *ooResult = result->closed ? result->closed : result->m;
	printf("Second: %d\n",ooResult==result->closed);
	print_opt_hmat(ooResult->mat,2);

	printf("meet is compatible direct: ");
	printf("%d\n", opt_oct_is_leq(man, octagon1, octagon2) && opt_oct_is_eq(man, result, octagon1));
	return 0;
}










