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
	long nbcons = 3;

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

	opt_pk_array_t* polyhedron1;

	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(nbcons);
		lincons0.p[0].constyp = ELINA_CONS_SUPEQ;
		lincons0.p[1].constyp = ELINA_CONS_EQ;
		lincons0.p[2].constyp = ELINA_CONS_EQ;
		long values1[3] = {  -1099511627776, 0, 0};
		elina_linexpr0_t * linexpr0 = create_linexpr0(dim, values1);
		lincons0.p[0].linexpr0 = linexpr0;
		long values2[3] = {-9223372036854775808, 0, 4261412864};
		elina_linexpr0_t * linexpr1 = create_linexpr0(dim, values2);
		lincons0.p[1].linexpr0 = linexpr1;
		long values3[3] = {4737096, 9288674231451648, -72057413615747072};
		elina_linexpr0_t * linexpr2 = create_linexpr0(dim, values3);
		printf("linexpr2: %p",linexpr2);
		fflush(stdout);
		lincons0.p[2].linexpr0 = linexpr2;

	if (create_polyhedron(&polyhedron1, man, top, dim, lincons0)) {
		printf("polyhedron meet bottom == bottom: ");
		printf("%d\n", opt_pk_is_eq(man, opt_pk_meet(man, false, polyhedron1, bottom), bottom));
	}
	return 0;
}

