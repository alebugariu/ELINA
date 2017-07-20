#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include <string.h>
#include <stdio.h>

elina_linexpr0_t * create_linexpr0(int dim, int v1, int v2, int coeff1,
		int coeff2, int scalar_value) {
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

bool create_octagon(opt_oct_t** octagon, elina_manager_t* man, opt_oct_t * top,
		int dim, elina_lincons0_array_t constraints) {
	*octagon = opt_oct_meet_lincons_array(man, false, top, &constraints);
	printf("Create octagon: %d %d\n", (*octagon)->dim, (*octagon)->intdim);
	return true;
}

int main(int argc, char **argv) {
	unsigned int dataIndex = 0;
	int dim = 2;

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);
	opt_oct_t* octagon1;

	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(dim);
	lincons0.p[0].constyp = ELINA_CONS_SUPEQ;
	lincons0.p[1].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr0 = create_linexpr0(dim, 0, 1, 0, 1, 2);
	lincons0.p[0].linexpr0 = linexpr0;
	elina_linexpr0_t * linexpr1 = create_linexpr0(dim, 1, 0, 1, 0, 513);
	lincons0.p[1].linexpr0 = linexpr1;

	if (create_octagon(&octagon1, man, top, dim, lincons0)) {
		// bottom <= x
		printf("bottom <= octagon: ");
		printf("Created octagon: %d %d\n", octagon1->dim, octagon1->intdim);
		printf("%d\n", opt_oct_is_leq(man, bottom, octagon1));
	}
	return 0;
}

