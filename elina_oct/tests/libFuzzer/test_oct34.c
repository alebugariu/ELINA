#include <time.h>
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out34.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_oct_manager_alloc();
		opt_oct_t * top = opt_oct_top(man, dim, 0);
		opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

		opt_oct_t* octagon1;
		unsigned char number1;
		if (get_octagon(&octagon1, man, top, &number1, data, dataSize,
				&dataIndex, fp)) {
			if (opt_oct_is_bottom(man, octagon1) == false) {

				// assignment cannot return bottom if the current set of constraints is not bottom

				unsigned char assignedToVariable;
				if (create_variable(&assignedToVariable, true, dim, data,
						dataSize, &dataIndex, fp)) {
					elina_linexpr0_t** assignmentArray;
					elina_dim_t * tdim;

					if (create_assignment(&assignmentArray, assignedToVariable,
							&tdim, dim, data, dataSize, &dataIndex, fp)) {

						opt_oct_t* assign_result1 =
								opt_oct_assign_linexpr_array(man, DESTRUCTIVE,
										octagon1, tdim, assignmentArray, 1,
										NULL);

						if (opt_oct_is_bottom(man, assign_result1) == true) {
							fprintf(fp, "found octagon %d!\n", number1);
							print_octagon(man, octagon1, number1, fp);
							fflush(fp);
							free_pool(man);
							free_octagon(man, &top);
							free_octagon(man, &bottom);
							free_octagon(man, &octagon1);
							opt_oct_free(man, assign_result1);
							elina_manager_free(man);
							fclose(fp);
							return 1;
						}
						opt_oct_free(man, assign_result1);
						free(assignmentArray);
						free(tdim);
					}
				}
				free_octagon(man, &octagon1);
			}
		}
		free_octagon(man, &top);
		free_octagon(man, &bottom);
	}
	elina_manager_free(man);
	fclose(fp);
	return 0;
}
