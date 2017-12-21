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
	fp = fopen("out30.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		opt_oct_t* octagon1;
		int number1;
		if (get_octagon_from_pool(&octagon1, &number1, data, dataSize, &dataIndex)) {

			//meet == glb, join == lub
			//bottom widening x == x
			if (!opt_oct_is_eq(man, opt_oct_widening(man, bottom, octagon1),
					octagon1)) {
				elina_lincons0_array_t a1 = opt_oct_to_lincons_array(man,
						octagon1);
				fprintf(fp, "found octagon%d: ", number1);
				elina_lincons0_array_fprint(fp, &a1, NULL);
				fflush(fp);
				elina_lincons0_array_clear(&a1);
				free_pool(man);
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

