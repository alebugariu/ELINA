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

bool create_polyhedron_from_top(opt_pk_array_t** polyhedron,
		elina_manager_t* man, opt_pk_array_t * top, int dim,
		elina_lincons0_array_t constraints) {
	*polyhedron = opt_pk_meet_lincons_array(man, false, top, &constraints);
	opt_pk_internal_t * internal_pk = opt_pk_init_from_manager(man,
			ELINA_FUNID_MEET_LINCONS_ARRAY);
	if (internal_pk->exn != ELINA_EXC_OVERFLOW) {
		return true;
	}
	return false;
}

opt_pk_array_t* assign(int dim, elina_manager_t* man,
		opt_pk_array_t* polyhedron, long *assignment_values,
		int assignedToVariable) {
	elina_linexpr0_t** expr_array = (elina_linexpr0_t**) malloc(
			sizeof(elina_linexpr0_t*));
	elina_linexpr0_t* expression = create_linexpr0(dim, assignment_values);
	expr_array[0] = expression;
	elina_dim_t* tdim = (elina_dim_t*) malloc(sizeof(elina_dim_t));
	tdim[0] = assignedToVariable;
	opt_pk_array_t* assign_result = opt_pk_assign_linexpr_array(man, false,
			polyhedron, tdim, expr_array, 1, NULL);
	fprintf(stdout, "Assign result: ");
	elina_lincons0_array_t a = opt_pk_to_lincons_array(man, assign_result);
	elina_lincons0_array_print(&a, NULL);
	fflush(stdout);
	return assign_result;
}

opt_pk_array_t* project(int dim, elina_manager_t* man,
		opt_pk_array_t* polyhedron, int projectedVariable) {

	elina_dim_t * tdim = (elina_dim_t *) malloc(sizeof(elina_dim_t));
	tdim[0] = projectedVariable;
	opt_pk_array_t* project_result = opt_pk_forget_array(man, false, polyhedron,
			tdim, 1, false);
	fprintf(stdout, "Project result: ");
	elina_lincons0_array_t a = opt_pk_to_lincons_array(man, project_result);
	elina_lincons0_array_print(&a, NULL);
	fflush(stdout);
	return project_result;
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
	long values1[3] = { 39, -224, 255 };
	elina_linexpr0_t * linexpr0 = create_linexpr0(dim, values1);
	lincons0.p[0].linexpr0 = linexpr0;
	long values2[3] = { 8231, 42, 0 };
	elina_linexpr0_t * linexpr1 = create_linexpr0(dim, values2);
	lincons0.p[1].linexpr0 = linexpr1;

	if (create_polyhedron_from_top(&polyhedron1, man, top, dim, lincons0)) {

		long assignment_values0[3] = { 187, 1024, 0 };
		polyhedron1 = assign(dim, man, polyhedron1, assignment_values0, 0);

		long assignment_values1[3] = { -8193, 511, -256 };
		polyhedron1 = assign(dim, man, polyhedron1, assignment_values1, 1);

		fprintf(stdout, "Successfully created polyhedron1!\n");
		elina_lincons0_array_t a1 = opt_pk_to_lincons_array(man, polyhedron1);
		elina_lincons0_array_print(&a1, NULL);

		opt_pk_array_t* polyhedron2;

		if (create_polyhedron_from_top(&polyhedron2, man, top, dim, lincons0)) {
			polyhedron2 = assign(dim, man, polyhedron2, assignment_values0, 0);
			polyhedron2 = project(dim, man, polyhedron2, 1);

			fprintf(stdout, "Successfully created polyhedron2!\n");
			elina_lincons0_array_t a2 = opt_pk_to_lincons_array(man,
					polyhedron2);
			elina_lincons0_array_print(&a2, NULL);

			// <= is anti symmetric
			//this if condition is never true, because leq overflows and returns top, so the test case doesn't fail
			if (opt_pk_is_leq(man, polyhedron1, polyhedron2) == true
					&& opt_pk_is_leq(man, polyhedron2, polyhedron1) == true) {
				printf("polyhedron1 == polyhedron2: ");
				printf("%d\n", opt_pk_is_eq(man, polyhedron1, polyhedron2));
			}
		}
	}
	return 0;
}

