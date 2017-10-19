#include <time.h>
#include "opt_pk.h"
#include "opt_pk_internal.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

elina_linexpr0_t * create_linexpr0(int dim, long *values) {
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

bool create_polyhedron(opt_pk_array_t** polyhedron, elina_manager_t* man,
		opt_pk_array_t * top, int dim, elina_lincons0_array_t constraints) {
	*polyhedron = opt_pk_meet_lincons_array(man, false, top, &constraints);
	opt_pk_internal_t * internal_pk = opt_pk_init_from_manager(man,
			ELINA_FUNID_MEET_LINCONS_ARRAY);
	if (internal_pk->exn != ELINA_EXC_OVERFLOW) {
		return true;
	}
	// should return false, because this test case causes overflow in meet_lincons
	return false;
}

int main(int argc, char **argv) {
	int dim = 8;
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
	long values1[9] = { -1, -1, 566248488304639, -1, -1, -1, -1, -1, -1 };
	elina_linexpr0_t * linexpr0 = create_linexpr0(dim, values1);
	lincons0.p[0].linexpr0 = linexpr0;
	long values2[9] = {0, 0, 0, 0, 0, 0, 48059, 0, 0};
	elina_linexpr0_t * linexpr1 = create_linexpr0(dim, values2);
	lincons0.p[1].linexpr0 = linexpr1;
	long values3[9] = { 0, 13194139533312, 0, 3105, 0, 12, 4683743664558571520, -4919131752989213765, -4919131752989213765 };
	elina_linexpr0_t * linexpr2 = create_linexpr0(dim, values3);
	lincons0.p[2].linexpr0 = linexpr2;
	long values4[9] =
			{ 0, 0, 0, 0, 0, 52093255680, 1099511627776, 0, 0 };
	elina_linexpr0_t * linexpr3 = create_linexpr0(dim, values4);
	lincons0.p[3].linexpr0 = linexpr3;
	long values5[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	elina_linexpr0_t * linexpr4 = create_linexpr0(dim, values5);
	lincons0.p[4].linexpr0 = linexpr4;
	long values6[9] = { 0, 0, 0, 0, 0, 0, 206411833278464, 0, 0 };
	elina_linexpr0_t * linexpr5 = create_linexpr0(dim, values6);
	lincons0.p[5].linexpr0 = linexpr5;
	long values7[9] = { -4294967296, -1, -1, -1, -1, -1, -1, -1, -1 };
	elina_linexpr0_t * linexpr6 = create_linexpr0(dim, values7);
	lincons0.p[6].linexpr0 = linexpr6;
	long values8[9] = { -1, -1099494850561, -1, -1, -1, -1, -1, -1, -1 };
	elina_linexpr0_t * linexpr7 = create_linexpr0(dim, values8);
	lincons0.p[7].linexpr0 = linexpr7;

	if (create_polyhedron(&polyhedron1, man, top, dim, lincons0)) {
		printf("polyhedron less equal polyhedron: ");
		printf("%d\n", (opt_pk_is_leq(man, polyhedron1, polyhedron1)));
	}
	return 0;
}

