#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
	FILE *fp;
	fp = fopen("out6.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_pk_manager_alloc(false);
		opt_pk_array_t * top = opt_pk_top(man, dim, 0);

		opt_pk_array_t* polyhedron1;
		if (create_polyhedron(&polyhedron1, man, top, dim, data, dataSize,
				&dataIndex, fp)) {

			//meet == glb, join == lub
			//x join top == top
			if (!opt_pk_is_eq(man, opt_pk_join(man, false, top, polyhedron1),
					top)) {
				opt_pk_free(man, top);
				opt_pk_free(man, polyhedron1);
				elina_manager_free(man);
				fclose(fp);
				return 1;
			}
			opt_pk_free(man, polyhedron1);
		}
		opt_pk_free(man, top);
		elina_manager_free(man);
	}
	fclose(fp);
	return 0;
}

