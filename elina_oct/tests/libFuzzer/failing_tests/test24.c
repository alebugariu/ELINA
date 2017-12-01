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

opt_oct_t* assign(int dim, elina_manager_t* man, opt_oct_t* octagon,
		long *assignment_values, int assignedToVariable) {
	elina_linexpr0_t** expr_array = (elina_linexpr0_t**) malloc(
			sizeof(elina_linexpr0_t*));
	elina_linexpr0_t* expression = create_polyhedral_linexpr0(dim,
			assignment_values);
	expr_array[0] = expression;
	elina_dim_t* tdim = (elina_dim_t*) malloc(sizeof(elina_dim_t));
	tdim[0] = assignedToVariable;
	opt_oct_t* assign_result = opt_oct_assign_linexpr_array(man, false, octagon,
			tdim, expr_array, 1, NULL);
	fprintf(stdout, "Assign result: ");
	elina_lincons0_array_t a = opt_oct_to_lincons_array(man, assign_result);
	elina_lincons0_array_print(&a, NULL);
	fflush(stdout);
	return assign_result;
}

int main(int argc, char **argv) {
	int dim = 4;

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	opt_oct_t* octagon1 = top;
	opt_oct_t* octagon2 = top;

	long assignment_values[5] = { 0, 1, 1, -1, -1 };
	octagon1 = assign(dim, man, octagon1, assignment_values, 1);
	long assignment_values1[5] = { 0, 1, 0, 0, 0 };
	octagon1 = assign(dim, man, octagon1, assignment_values1, 2);
	long assignment_values2[5] = { 0, 1, 1, -1, -1 };
	octagon1 = assign(dim, man, octagon1, assignment_values2, 3);
	octagon1 = assign(dim, man, octagon1, assignment_values2, 0);
	long assignment_values3[5] = { -1, -1, -1, -1, -1 };
	octagon1 = assign(dim, man, octagon1, assignment_values3, 1);

	printf("Successfully created octagon1!\n");
	elina_lincons0_array_t a1 = opt_oct_to_lincons_array(man, octagon1);
	elina_lincons0_array_print(&a1, NULL);
	fflush(stdout);

	octagon2 = assign(dim, man, octagon2, assignment_values, 1);
	octagon2 = assign(dim, man, octagon2, assignment_values1, 2);
	octagon2 = assign(dim, man, octagon2, assignment_values2, 3);
	octagon2 = assign(dim, man, octagon2, assignment_values2, 0);
	octagon2 = assign(dim, man, octagon2, assignment_values3, 0);

	printf("Successfully created octagon2!\n");
	elina_lincons0_array_t a2 = opt_oct_to_lincons_array(man, octagon2);
	elina_lincons0_array_print(&a2, NULL);
	fflush(stdout);

	// meet absorbtion
	printf("absorbtion result: ");
	printf("%d\n",
			opt_oct_is_eq(man,
					opt_oct_meet(man, false, octagon1,
							opt_oct_join(man, false, octagon1, octagon2)),
					octagon1));
	fflush(stdout);
	return 0;
}

