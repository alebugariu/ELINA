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
	int dim = 7;
	int nbcons = 10;
	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);

	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(nbcons);

	lincons0.p[0].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr0 = create_linexpr0(dim, 0, 1, 1, 1, 2);
	lincons0.p[0].linexpr0 = linexpr0;

	lincons0.p[1].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr1 = create_linexpr0(dim, 0, 1, -1, 1, 2);
	lincons0.p[1].linexpr0 = linexpr1;

	lincons0.p[2].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr2 = create_linexpr0(dim, 0, 1, -1, 1, 2);
	lincons0.p[2].linexpr0 = linexpr2;

	lincons0.p[3].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr3 = create_linexpr0(dim, 1, 5, 0, 1, 2);
	lincons0.p[3].linexpr0 = linexpr3;

	lincons0.p[4].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr4 = create_linexpr0(dim, 0, 1, -1, 1, 2);
	lincons0.p[4].linexpr0 = linexpr4;

	lincons0.p[5].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr5 = create_linexpr0(dim, 0, 1, 0, 0, 2);
	lincons0.p[5].linexpr0 = linexpr5;

	lincons0.p[6].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr6 = create_linexpr0(dim, 0, 6, 0, 1, 2);
	lincons0.p[6].linexpr0 = linexpr6;

	lincons0.p[7].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr7 = create_linexpr0(dim, 4, 1, -1, 1, 2);
	lincons0.p[7].linexpr0 = linexpr7;

	lincons0.p[8].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr8 = create_linexpr0(dim, 0, 1, -1, -1, 2);
	lincons0.p[8].linexpr0 = linexpr8;

	lincons0.p[9].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr9 = create_linexpr0(dim, 0, 1, -1, 1, 2);
	lincons0.p[9].linexpr0 = linexpr9;

	opt_oct_t* octagon;
	create_octagon(&octagon, man, top, dim, lincons0);
	opt_oct_mat_t *oo = octagon->closed ? octagon->closed : octagon->m;
	//printf("DENSE: %d\n", oo->is_dense);
	//print_opt_hmat(oo->mat, dim);
	printf("top meet octagon == octagon: ");
	opt_oct_t * meet_result = opt_oct_meet(man, false, top, octagon);
	opt_oct_mat_t *oo_result = meet_result->closed ? meet_result->closed : meet_result->m;
	//print_opt_hmat(oo_result->mat, dim);
	bool res = opt_oct_is_eq(man, meet_result, octagon);
	printf("%d\n", res);
}

