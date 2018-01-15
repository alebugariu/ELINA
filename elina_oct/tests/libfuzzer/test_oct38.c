#include <time.h>
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out48.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		// conditional should return bottom if the current set of constraints is bottom

		elina_lincons0_array_t conditionalArray;

		if (create_conditional(&conditionalArray, data, dataSize, &dataIndex,
				fp)) {

			opt_oct_t* cond_result1 = opt_oct_meet_lincons_array(man,
			DESTRUCTIVE, bottom, &conditionalArray);

			if (opt_oct_is_bottom(man, cond_result1) == false) {
				elina_lincons0_array_t a1 = opt_oct_to_lincons_array(man,
						cond_result1);
				fprintf(fp, "found non bottom conditional result: ");
				elina_lincons0_array_fprint(fp, &a1, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a1);
				free_pool(man);
				opt_oct_free(man, cond_result1);
				elina_manager_free(man);
				fclose(fp);
				return 1;
			}
		}
	}
	elina_manager_free(man);
	fclose(fp);
	return 0;
}
