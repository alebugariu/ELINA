#include <time.h>
#include "opt_pk.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

elina_linexpr0_t * create_linexpr0(unsigned short int dim, int *values) {
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
	return true;
}

int main(int argc, char **argv) {
	int dim = 2;
	int nbcons = 2;

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);

	opt_pk_array_t* polyhedron1;

	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(nbcons);
	lincons0.p[0].constyp = ELINA_CONS_EQ;
	lincons0.p[1].constyp = ELINA_CONS_EQ;
	int values1[3] = {1834972265, 285212770, 286331153};
	elina_linexpr0_t * linexpr0 = create_linexpr0(dim, values1);
	lincons0.p[0].linexpr0 = linexpr0;
	int values2[3] = {2019885056, 1601463137, 1769239137};
	elina_linexpr0_t * linexpr1 = create_linexpr0(dim, values2);
	lincons0.p[1].linexpr0 = linexpr1;

	if (create_polyhedron(&polyhedron1, man, top, dim, lincons0)) {
		// x <= top
		printf("polyhedron <= top: ");
		printf("%d\n", opt_pk_is_leq(man, polyhedron1, top));
	}
	return 0;
}

