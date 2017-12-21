#include <time.h>
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out43.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		opt_oct_t* octagon1;
		int number1;
		if (get_octagon_from_pool(&octagon1, &number1, data, dataSize, &dataIndex)) {

			// assign(x) less equal project(x)

			int assignedToVariable;
			if (create_variable(&assignedToVariable, true, dim, data, dataSize,
					&dataIndex, fp)) {

				elina_linexpr0_t** assignmentArray;
				elina_dim_t * tdim;

				create_assignment(&assignmentArray, assignedToVariable, &tdim,
						fp);

				opt_oct_t* assign_result1 = opt_oct_assign_linexpr_array(man,
				DESTRUCTIVE, octagon1, tdim, assignmentArray, 1,
				NULL);

				opt_oct_t* project_result1 = opt_oct_forget_array(man,
				DESTRUCTIVE, octagon1, tdim, 1, false);

				if (opt_oct_is_leq(man, assign_result1, project_result1)
						== false) {
					elina_lincons0_array_t a1 = opt_oct_to_lincons_array(man,
							octagon1);
					fprintf(fp, "found octagon%d: ", number1);
					elina_lincons0_array_fprint(fp, &a1, NULL);
					fflush(fp);
					elina_lincons0_array_clear(&a1);
					free_pool(man);
					free(assignmentArray);
					free(tdim);
					elina_manager_free(man);
					fclose(fp);
					return 1;
				}
				opt_oct_free(man, assign_result1);
				opt_oct_free(man, project_result1);
				free(assignmentArray);
				free(tdim);
			}
		}
	}
	elina_manager_free(man);
	fclose(fp);
	return 0;
}
