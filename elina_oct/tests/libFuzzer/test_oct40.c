#include <time.h>
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
	FILE *fp;
	fp = fopen("out40.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_oct_manager_alloc();
		opt_oct_t * top = opt_oct_top(man, dim, 0);
		opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

		opt_oct_t* octagon1;
		if (create_octagon(&octagon1, man, top, bottom, dim, data, dataSize,
				&dataIndex, fp)) {
			opt_oct_t* octagon2;
			if (create_octagon(&octagon2, man, top, bottom, dim, data, dataSize,
					&dataIndex, fp)) {

				// assignment is monotone
				if (assume_fuzzable(opt_oct_is_leq(man, octagon1, octagon2))) {

					int assignedToVariable;
					if (create_variable(&assignedToVariable, true, dim, data,
							dataSize, &dataIndex, fp)) {
						elina_linexpr0_t** assignmentArray;
						elina_dim_t * tdim;

						if (create_assignment(&assignmentArray,
								assignedToVariable, &tdim, dim, data, dataSize,
								&dataIndex, fp)) {

							opt_oct_t* assign_result1 =
									opt_oct_assign_linexpr_array(man,
									DESTRUCTIVE, octagon1, tdim,
											assignmentArray, 1,
											NULL);

							opt_oct_t* assign_result2 =
									opt_oct_assign_linexpr_array(man,
									DESTRUCTIVE, octagon2, tdim,
											assignmentArray, 1,
											NULL);

							if (opt_oct_is_leq(man, assign_result1,
									assign_result2) == false) {
								opt_oct_free(man, top);
								opt_oct_free(man, bottom);
								opt_oct_free(man, octagon1);
								opt_oct_free(man, octagon2);
								opt_oct_free(man, assign_result1);
								opt_oct_free(man, assign_result2);
								free(assignmentArray);
								free(tdim);
								elina_manager_free(man);
								fclose(fp);
								return 1;
							}
							opt_oct_free(man, assign_result1);
							opt_oct_free(man, assign_result2);
							free(assignmentArray);
							free(tdim);
						}
					}
				}
				opt_oct_free(man, octagon2);
			}
			opt_oct_free(man, octagon1);
		}
		opt_oct_free(man, top);
		opt_oct_free(man, bottom);
		elina_manager_free(man);
	}
	fclose(fp);
	return 0;
}

