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

elina_linexpr0_t * create_polyhedral_linexpr0(int dim, long *values) {
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

bool create_octagon(opt_oct_t** octagon, elina_manager_t* man, opt_oct_t * top,
		int dim, elina_lincons0_array_t constraints) {
	*octagon = opt_oct_meet_lincons_array(man, false, top, &constraints);
	return true;
}

int main(int argc, char **argv) {
	int dim = 15;
	int nbcons = 23;
	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);

	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(nbcons);

	lincons0.p[0].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr0 = create_linexpr0(dim, 5, 4, -1, -1, -1);
	lincons0.p[0].linexpr0 = linexpr0;

	lincons0.p[1].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr1 = create_linexpr0(dim, 1, 0, -1, -1, -1);
	lincons0.p[1].linexpr0 = linexpr1;

	lincons0.p[2].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr2 = create_linexpr0(dim, 1, 0, -1, -1, -1);
	lincons0.p[2].linexpr0 = linexpr2;

	lincons0.p[3].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr3 = create_linexpr0(dim, 1, 0, -1, -1, -1);
	lincons0.p[3].linexpr0 = linexpr3;

	lincons0.p[4].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr4 = create_linexpr0(dim, 12, 3, -1, -1, -1);
	lincons0.p[4].linexpr0 = linexpr4;

	lincons0.p[5].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr5 = create_linexpr0(dim, 1, 0, -1, -1, -1);
	lincons0.p[5].linexpr0 = linexpr5;

	lincons0.p[6].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr6 = create_linexpr0(dim, 2, 0, -1, -1, -1);
	lincons0.p[6].linexpr0 = linexpr6;

	lincons0.p[7].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr7 = create_linexpr0(dim, 1, 0, -1, -1, -1);
	lincons0.p[7].linexpr0 = linexpr7;

	lincons0.p[8].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr8 = create_linexpr0(dim, 5, 3, -1, -1, -1); //h
	lincons0.p[8].linexpr0 = linexpr8;

	lincons0.p[9].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr9 = create_linexpr0(dim, 1, 0, -1, -1, -1); //h
	lincons0.p[9].linexpr0 = linexpr9;

	lincons0.p[10].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr10 = create_linexpr0(dim, 1, 0, -1, -1, -1); //h
	lincons0.p[10].linexpr0 = linexpr10;

	lincons0.p[11].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr11 = create_linexpr0(dim, 0, 2, -1, -1, -1); //h
	lincons0.p[11].linexpr0 = linexpr11;

	lincons0.p[12].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr12 = create_linexpr0(dim, 1, 0, -1, -1, -1); //h
	lincons0.p[12].linexpr0 = linexpr12;

	lincons0.p[13].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr13 = create_linexpr0(dim, 7, 0, -1, -1, -1); //h
	lincons0.p[13].linexpr0 = linexpr13;

	lincons0.p[14].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr14 = create_linexpr0(dim, 1, 0, -1, -1, -1);
	lincons0.p[14].linexpr0 = linexpr14;

	lincons0.p[15].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr15 = create_linexpr0(dim, 5, 4, -1, -1, -1);
	lincons0.p[15].linexpr0 = linexpr15;

	lincons0.p[16].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr16 = create_linexpr0(dim, 1, 0, -1, -1, -1);
	lincons0.p[16].linexpr0 = linexpr16;

	lincons0.p[17].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr17 = create_linexpr0(dim, 7, 0, -1, -1, -1);
	lincons0.p[17].linexpr0 = linexpr17;

	lincons0.p[18].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr18 = create_linexpr0(dim, 1, 0, -1, -1, -1);
	lincons0.p[18].linexpr0 = linexpr18;

	lincons0.p[19].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr19 = create_linexpr0(dim, 5, 4, -1, -1, -1);
	lincons0.p[19].linexpr0 = linexpr19;

	lincons0.p[20].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr20 = create_linexpr0(dim, 1, 0, -1, -1, -1);
	lincons0.p[20].linexpr0 = linexpr20;

	lincons0.p[21].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr21 = create_linexpr0(dim, 5, 4, -1, -1, -1);
	lincons0.p[21].linexpr0 = linexpr21;

	lincons0.p[22].constyp = ELINA_CONS_SUPEQ;
	elina_linexpr0_t * linexpr22 = create_linexpr0(dim, 1, 0, -1, -1, -1);
	lincons0.p[22].linexpr0 = linexpr22;

	opt_oct_t* octagon = top;
	opt_oct_t* other;
	create_octagon(&other, man, top, dim, lincons0);
	printf("Successfully created other octagon!\n");
	elina_lincons0_array_t a = opt_oct_to_lincons_array(man, other);
	elina_lincons0_array_print(&a, NULL);
	octagon = opt_oct_meet(man, false, octagon, other);
	printf("Successfully created octagon!\n");
	elina_lincons0_array_t a1 = opt_oct_to_lincons_array(man, octagon);
	elina_lincons0_array_print(&a1, NULL);
	fflush(stdout);
	printf("is top: %d\n", opt_oct_is_top(man, octagon));
	printf("equals top: %d\n", opt_oct_is_eq(man, octagon, top));
}

