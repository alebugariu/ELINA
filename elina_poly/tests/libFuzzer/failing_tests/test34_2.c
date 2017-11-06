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
		fflush(NULL);
		elina_scalar_set_to_int(coeff->val.scalar, values[i],
				ELINA_SCALAR_DOUBLE);
	}
	return linexpr0;
}

bool create_polyhedron_from_bottom(opt_pk_array_t** polyhedron,
		elina_manager_t* man, opt_pk_array_t * top, opt_pk_array_t * bottom,
		int dim, long nbcons, elina_lincons0_array_t allConstraints) {
	*polyhedron = bottom;

	size_t i;
	for (i = 0; i < nbcons; i++) {
		elina_lincons0_array_t constraints = elina_lincons0_array_make(1);
		constraints.p[0].constyp = allConstraints.p[i].constyp;
		constraints.p[0].linexpr0 = allConstraints.p[i].linexpr0;
		opt_pk_array_t* meet_result = opt_pk_meet_lincons_array(man, false, top,
				&constraints);
		opt_pk_internal_t * internal_pk = opt_pk_init_from_manager(man,
				ELINA_FUNID_MEET_LINCONS_ARRAY);
		if (internal_pk->exn == ELINA_EXC_OVERFLOW) {
			return false;
		}
		*polyhedron = opt_pk_join(man, false, *polyhedron, meet_result);
		internal_pk = opt_pk_init_from_manager(man, ELINA_FUNID_JOIN);
		if (internal_pk->exn == ELINA_EXC_OVERFLOW) {
			return false;
		}
	}
	return true;
}

opt_pk_array_t* assign(int dim, elina_manager_t* man,
		opt_pk_array_t* polyhedron1, long *assignment_values,
		int assignedToVariable) {
	elina_linexpr0_t** expr_array = (elina_linexpr0_t**) malloc(
			sizeof(elina_linexpr0_t*));
	elina_linexpr0_t* expression = create_linexpr0(dim, assignment_values);
	expr_array[0] = expression;
	elina_dim_t* tdim = (elina_dim_t*) malloc(sizeof(elina_dim_t));
	tdim[0] = assignedToVariable;
	opt_pk_array_t* assign_result = opt_pk_assign_linexpr_array(man, false,
			polyhedron1, tdim, expr_array, 1, NULL);
	fprintf(stdout, "Assign result: ");
	elina_lincons0_array_t a = opt_pk_to_lincons_array(man, assign_result);
	elina_lincons0_array_print(&a, NULL);
	fflush(stdout);
	return assign_result;
}

int main(int argc, char **argv) {
	int dim = 2;
	long nbcons = 2;

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);

	opt_pk_array_t* polyhedron1;

	elina_lincons0_array_t lincons0 = elina_lincons0_array_make(nbcons);
	lincons0.p[0].constyp = ELINA_CONS_EQ;
	lincons0.p[1].constyp = ELINA_CONS_EQ;
	long values1[3] = { 255, 0, 1024 };
	elina_linexpr0_t * linexpr0 = create_linexpr0(dim, values1);
	lincons0.p[0].linexpr0 = linexpr0;
	long values2[3] = { 40, 0, 0 };
	elina_linexpr0_t * linexpr1 = create_linexpr0(dim, values2);
	lincons0.p[1].linexpr0 = linexpr1;

	if (create_polyhedron_from_bottom(&polyhedron1, man, top, bottom, dim,
			nbcons, lincons0)) {
		long assignment_values0[3] = { 0, 0, 0 };
		polyhedron1 = assign(dim, man, polyhedron1, assignment_values0, 0);

		long assignment_values1[3] = { 255, 0, 0 };
		polyhedron1 = assign(dim, man, polyhedron1, assignment_values1, 0);

		fprintf(stdout, "Successfully created!\n");
		fflush(stdout);
		if (opt_pk_is_eq(man, polyhedron1, bottom) == false) {
			long assignment_values2[3] = { 0, 0, 0 };
			polyhedron1 = assign(dim, man, polyhedron1, assignment_values2, 0);
			//this test case doesn't fail
			printf("assignment result == bottom: ");
			printf("%d\n", opt_pk_is_eq(man, polyhedron1, bottom));
		}
	}
	return 0;
}

