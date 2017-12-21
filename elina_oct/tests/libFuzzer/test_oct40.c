#include <time.h>
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out40.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		opt_oct_t* octagon1;
		int number1;
		if (get_octagon_from_pool(&octagon1, &number1, data, dataSize, &dataIndex)) {

			opt_oct_t* octagon2;
			int number2;
			if (get_octagon_from_pool(&octagon2, &number2, data, dataSize, &dataIndex)) {

				// assignment is monotone
				if (assume_fuzzable(opt_oct_is_leq(man, octagon1, octagon2))) {

					int assignedToVariable;
					if (create_variable(&assignedToVariable, true, dim, data,
							dataSize, &dataIndex, fp)) {
						elina_linexpr0_t** assignmentArray;
						elina_dim_t * tdim;

						create_assignment(&assignmentArray, assignedToVariable,
								&tdim, fp);

						opt_oct_t* assign_result1 =
								opt_oct_assign_linexpr_array(man,
								DESTRUCTIVE, octagon1, tdim, assignmentArray, 1,
								NULL);

						opt_oct_t* assign_result2 =
								opt_oct_assign_linexpr_array(man,
								DESTRUCTIVE, octagon2, tdim, assignmentArray, 1,
								NULL);

						if (opt_oct_is_leq(man, assign_result1, assign_result2)
								== false) {

							elina_lincons0_array_t a1 =
									opt_oct_to_lincons_array(man, octagon1);
							fprintf(fp, "found octagon%d: ", number1);
							elina_lincons0_array_fprint(fp, &a1, NULL);
							elina_lincons0_array_t a2 =
									opt_oct_to_lincons_array(man, octagon2);
							fprintf(fp, "found octagon%d: ", number2);
							elina_lincons0_array_fprint(fp, &a2, NULL);
							fflush(fp);
							elina_lincons0_array_clear(&a1);
							elina_lincons0_array_clear(&a2);
							free_pool(man);
							free(assignmentArray);
							free(tdim);
							elina_manager_free(man);
							fclose(fp);
							return 1;
						}
						free(assignmentArray);
						free(tdim);
					}
				}
			}
		}
	}
	elina_manager_free(man);
	fclose(fp);
	return 0;
}
