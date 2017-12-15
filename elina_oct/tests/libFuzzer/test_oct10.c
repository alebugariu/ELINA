#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out10.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		opt_oct_t* octagon1;
		if (get_octagon_from_pool(&octagon1, data, dataSize, &dataIndex)) {

			opt_oct_t* octagon2;
			if (get_octagon_from_pool(&octagon2, data, dataSize, &dataIndex)) {

				opt_oct_t* octagon3;
				if (get_octagon_from_pool(&octagon3, data, dataSize,
						&dataIndex)) {

					//meet == glb, join == lub
					//join is associative
					if (!opt_oct_is_eq(man,
							opt_oct_join(man, DESTRUCTIVE,
									opt_oct_join(man, DESTRUCTIVE, octagon1,
											octagon2), octagon3),
							opt_oct_join(man, DESTRUCTIVE, octagon1,
									opt_oct_join(man, DESTRUCTIVE, octagon2,
											octagon3)))) {
						fprintf(fp, "found octagon1: ");
						elina_lincons0_array_t a1 = opt_oct_to_lincons_array(
								man, octagon1);
						elina_lincons0_array_fprint(fp, &a1, NULL);
						fprintf(fp, "found octagon2: ");
						elina_lincons0_array_t a2 = opt_oct_to_lincons_array(
								man, octagon2);
						elina_lincons0_array_fprint(fp, &a2, NULL);
						fprintf(fp, "found octagon3: ");
						elina_lincons0_array_t a3 = opt_oct_to_lincons_array(
								man, octagon3);
						elina_lincons0_array_fprint(fp, &a3, NULL);
						fflush(fp);
						elina_lincons0_array_clear(&a1);
						elina_lincons0_array_clear(&a2);
						elina_lincons0_array_clear(&a3);
						free_pool(man);
						elina_manager_free(man);
						fclose(fp);
						return 1;
					}
				}
			}
		}
	}
	elina_manager_free(man);
	fclose(fp);
	return 0;
}

