#include <time.h>
#include "opt_pk.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

elina_linexpr0_t * create_linexpr0(long dim, long *values) {
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
		fflush(NULL);
		elina_scalar_set_to_int(coeff->val.scalar, values[i],
				ELINA_SCALAR_DOUBLE);
	}
	return linexpr0;
}

bool create_polyhedron(opt_pk_array_t** polyhedron, elina_manager_t* man,
		opt_pk_array_t * top, long dim, elina_lincons0_array_t constraints) {
	*polyhedron = opt_pk_meet_lincons_array(man, false, top, &constraints);
	return true;
}

int main(int argc, char **argv) {
	long dim = 2;
	long nbcons = 8;

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);

	opt_pk_array_t* polyhedron1;

	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(nbcons);
	lincons0.p[0].constyp = ELINA_CONS_EQ;
	lincons0.p[1].constyp = ELINA_CONS_EQ;
	lincons0.p[2].constyp = ELINA_CONS_EQ;
	lincons0.p[3].constyp = ELINA_CONS_EQ;
	lincons0.p[4].constyp = ELINA_CONS_EQ;
	lincons0.p[5].constyp = ELINA_CONS_EQ;
	lincons0.p[6].constyp = ELINA_CONS_EQ;
	lincons0.p[7].constyp = ELINA_CONS_EQ;
	long values1[3] = { 52842235627813819, 12303106, 12 };
	elina_linexpr0_t * linexpr0 = create_linexpr0(dim, values1);
	lincons0.p[0].linexpr0 = linexpr0;
	long values2[3] = { 12303106, 0, 0 };
	elina_linexpr0_t * linexpr1 = create_linexpr0(dim, values2);
	lincons0.p[1].linexpr0 = linexpr1;
	long values3[3] = { 3377699720528059, 0, 0 };
	elina_linexpr0_t * linexpr2 = create_linexpr0(dim, values3);
	lincons0.p[2].linexpr0 = linexpr2;
	long values4[3] = {144321603058777019, 8796093070267, -4919131752997257216};
	elina_linexpr0_t * linexpr3 = create_linexpr0(dim, values4);
	lincons0.p[3].linexpr0 = linexpr3;
	long values5[3] = {  -4919131752804664389, 52841437921524667, 51539607552};
	elina_linexpr0_t * linexpr4 = create_linexpr0(dim, values5);
	lincons0.p[4].linexpr0 = linexpr4;
	long values6[3] = { -4919335965799236677, 144128382215389184, 2};
	elina_linexpr0_t * linexpr5 = create_linexpr0(dim, values6);
	lincons0.p[5].linexpr0 = linexpr5;
	long values7[3] = {  0, 137438953472, 0 };
	elina_linexpr0_t * linexpr6 = create_linexpr0(dim, values7);
	lincons0.p[6].linexpr0 = linexpr6;
	long values8[3] = { -4919131752997257216, -4919131752989213765, -4919131752989213765 };
	elina_linexpr0_t * linexpr7 = create_linexpr0(dim, values8);
	lincons0.p[7].linexpr0 = linexpr7;

	if (create_polyhedron(&polyhedron1, man, top, dim, lincons0)) {
		printf("top join polyhedrongit a == top: ");
		printf("%d\n",
				opt_pk_is_eq(man, opt_pk_join(man, false, top, polyhedron1),
						top));
	}
	return 0;
}

