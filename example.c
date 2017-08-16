#include <time.h>
#include "pk.h"
#include "opt_pk.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

ap_linexpr0_t * create_linexpr0_newpolka(unsigned short int dim, long *values) {
	ap_coeff_t *cst, *coeff;
	ap_linexpr0_t * linexpr0 = ap_linexpr0_alloc(AP_LINEXPR_SPARSE,
			dim);
	cst = &linexpr0->cst;
	ap_scalar_set_int(cst->val.scalar, values[dim]);

	size_t i;
	for (i = 0; i < dim; i++) {
		ap_linterm_t * linterm = &linexpr0->p.linterm[i];
		linterm->dim = i;
		coeff = &linterm->coeff;
		ap_scalar_set_int(coeff->val.scalar, values[i]);
	}
	return linexpr0;
}

bool create_polyhedron_newpolka(pk_t** polyhedron, ap_manager_t* man,
		pk_t * top, int dim, ap_lincons0_array_t constraints) {
	*polyhedron = pk_meet_lincons_array(man, false, top, &constraints);
	return true;
}


elina_linexpr0_t * create_linexpr0_elina(unsigned short int dim, long *values) {
	elina_coeff_t *cst, *coeff;
	elina_linexpr0_t * linexpr0 = elina_linexpr0_alloc(ELINA_LINEXPR_SPARSE,
			dim);
	cst = &linexpr0->cst;
	elina_scalar_set_int(cst->val.scalar, values[dim]);

	size_t i;
	for (i = 0; i < dim; i++) {
		elina_linterm_t * linterm = &linexpr0->p.linterm[i];
		linterm->dim = i;
		coeff = &linterm->coeff;
		elina_scalar_set_int(coeff->val.scalar, values[i]);
	}
	return linexpr0;
}

bool create_polyhedron_elina(opt_pk_array_t** polyhedron, elina_manager_t* man,
		opt_pk_array_t * top, int dim, elina_lincons0_array_t constraints) {
	*polyhedron = opt_pk_meet_lincons_array(man, false, top, &constraints);
	return true;
}




int main(int argc, char **argv) {
	int dim = 2;
	int nbcons = 2;

	ap_manager_t * man = pk_manager_alloc(false);
	pk_t * top = pk_top(man, dim, 0);

	pk_t* polyhedron1;

	ap_lincons0_array_t lincons0 = ap_lincons0_array_make(nbcons);
	lincons0.p[0].constyp = AP_CONS_EQ;
	lincons0.p[1].constyp = AP_CONS_EQ;
	long values1[3] = {LONG_MAX, LONG_MAX, LONG_MIN/2};
	ap_linexpr0_t * linexpr0 = create_linexpr0_newpolka(dim, values1);
	lincons0.p[0].linexpr0 = linexpr0;
	long values2[3] = {LONG_MIN/2, LONG_MAX, LONG_MIN/2};
	ap_linexpr0_t * linexpr1 = create_linexpr0_newpolka(dim, values2);
	lincons0.p[1].linexpr0 = linexpr1;


	elina_manager_t * man1 = opt_pk_manager_alloc(false);
	opt_pk_array_t * top1 = opt_pk_top(man1, dim, 0);

	opt_pk_array_t* polyhedron2;

	elina_lincons0_array_t lincons1 = elina_lincons0_array_make(nbcons);
	lincons1.p[0].constyp = ELINA_CONS_EQ;
	lincons1.p[1].constyp = ELINA_CONS_EQ;
	elina_linexpr0_t * linexpr3 = create_linexpr0_elina(dim, values1);
	lincons1.p[0].linexpr0 = linexpr3;
	elina_linexpr0_t * linexpr4 = create_linexpr0_elina(dim, values2);
	lincons1.p[1].linexpr0 = linexpr4;



	if (create_polyhedron_elina(&polyhedron2, man1, top1, dim, lincons1) ) {
		// x <= top
		printf("ELINA Polyhedron\n");
		elina_lincons0_array_t arr = opt_pk_to_lincons_array(man1,polyhedron2);
		elina_lincons0_array_fprint(stdout,&arr,NULL);
		printf("polyhedron1 <= top: ");
		printf("%d\n", opt_pk_is_leq(man1, polyhedron2,top1));
		fflush(stdout);
	}

	if (create_polyhedron_newpolka(&polyhedron1, man, top, dim, lincons0) ) {
		// x <= top
		printf("Newpolka Polyhedron\n");
		pk_fprint(stdout,man,polyhedron1,NULL);
		printf("polyhedron1 <= top: ");
		printf("%d\n", pk_is_leq(man, polyhedron1,top));
		fflush(stdout);
	}
	return 0;
}


