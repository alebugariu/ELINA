#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include <string.h>
#include <stdio.h>

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

bool create_octagon(opt_oct_t** octagon, elina_manager_t* man, opt_oct_t * top,
		int dim, elina_lincons0_array_t constraints) {
	*octagon = opt_oct_meet_lincons_array(man, false, top, &constraints);
	return true;
}

int main(int argc, char **argv) {
	int dim = 7;
	int nbcons = 7;

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);
	opt_oct_t* octagon1;

	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(nbcons);

	lincons0.p[0].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr0 = create_linexpr0(dim, 0, 6, 1, 1, 72057594037927936);
	lincons0.p[0].linexpr0 = linexpr0;

	lincons0.p[1].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr1 = create_linexpr0(dim, 0, 1, 0, 1, 0);
	lincons0.p[1].linexpr0 = linexpr1;

	lincons0.p[2].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr2 = create_linexpr0(dim, 3, 1, 1, 0, 0);
	lincons0.p[2].linexpr0 = linexpr2;

	lincons0.p[3].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr3 = create_linexpr0(dim, 0, 1, 0, 1, 0);
	lincons0.p[3].linexpr0 = linexpr3;

	lincons0.p[4].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr4 = create_linexpr0(dim, 2, 4, 1, 1, 0);
	lincons0.p[4].linexpr0 = linexpr4;

	lincons0.p[5].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr5 = create_linexpr0(dim, 0, 1, 1, 0, 1);
	lincons0.p[5].linexpr0 = linexpr5;

	lincons0.p[6].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr6 = create_linexpr0(dim, 0, 4, 0, 1, 1);
	lincons0.p[6].linexpr0 = linexpr6;

	if (create_octagon(&octagon1, man, top, dim, lincons0)) {
		opt_oct_mat_t *oo = octagon1->closed ? octagon1->closed : octagon1->m;
		print_opt_hmat(oo->mat, dim);

		opt_oct_t * meet_result = opt_oct_meet(man, false, top, octagon1);
		opt_oct_mat_t *oo_result =
				meet_result->closed ? meet_result->closed : meet_result->m;
		print_opt_hmat(oo_result->mat, dim);
		printf("top meet octagon == octagon: ");
		printf("%d\n", (opt_oct_is_eq(man, meet_result, octagon1)));
	}
	return 0;
}

