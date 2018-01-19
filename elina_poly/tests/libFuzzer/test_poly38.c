#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out38.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		// conditional should return bottom if the current set of constraints is bottom

		elina_lincons0_array_t conditionalArray;

		if (create_conditional(&conditionalArray, data, dataSize, &dataIndex,
				fp)) {

			opt_pk_array_t* cond_result1 = opt_pk_meet_lincons_array(man,
			DESTRUCTIVE, bottom, &conditionalArray);

			if (opt_pk_is_bottom(man, cond_result1) == false) {
				elina_lincons0_array_t a1 = opt_pk_to_lincons_array(man,
						cond_result1);
				fprintf(fp, "found non bottom conditional result: ");
				elina_lincons0_array_fprint(fp, &a1, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a1);
				free_pool(man);
				opt_pk_free(man, cond_result1);
				elina_manager_free(man);
				fclose(fp);
				return 1;
			}
			opt_pk_free(man, cond_result1);
		}
		free_polyhedron(man, &top);
		free_polyhedron(man, &bottom);
	}
	elina_manager_free(man);
	fclose(fp);
	return 0;
}
