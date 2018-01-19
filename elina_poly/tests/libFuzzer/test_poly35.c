#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out35.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		// assignment should return bottom if the current set of constraints is bottom

		unsigned char assignedToVariable;
		if (create_variable(&assignedToVariable, true, dim, data, dataSize,
				&dataIndex, fp)) {

			elina_linexpr0_t** assignmentArray;
			elina_dim_t * tdim;

			if (create_assignment(&assignmentArray, assignedToVariable, &tdim,
					dim, data, dataSize, &dataIndex, fp)) {

				opt_pk_array_t* assign_result1 = opt_pk_assign_linexpr_array(
						man,
						DESTRUCTIVE, bottom, tdim, assignmentArray, 1,
						NULL);
				opt_pk_internal_t * assign1_internal = opt_pk_init_from_manager(
						man, ELINA_FUNID_ASSIGN_LINEXPR_ARRAY);

				if (assign1_internal->exn != ELINA_EXC_OVERFLOW) {

					if (opt_pk_is_bottom(man, assign_result1) == false) {
						elina_lincons0_array_t a1 = opt_pk_to_lincons_array(
								man, assign_result1);
						fprintf(fp, "found non-bottom assignment result: ");
						elina_lincons0_array_fprint(fp, &a1, NULL);
						fflush(fp);
						elina_lincons0_array_clear(&a1);
						free_pool(man);
						free_polyhedron(man, &top);
						free_polyhedron(man, &bottom);
						opt_pk_free(man, assign_result1);
						free(assignmentArray);
						free(tdim);
						elina_manager_free(man);
						fclose(fp);
						return 1;
					}
				}
				opt_pk_free(man, assign_result1);
				free(assignmentArray);
				free(tdim);
			}
		}
		free_polyhedron(man, &top);
		free_polyhedron(man, &bottom);
	}
	elina_manager_free(man);
	fclose(fp);
	return 0;
}

