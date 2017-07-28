#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const int *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
	FILE *fp;
	fp = fopen("out6.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_oct_manager_alloc();
		opt_pk_array_t * top = opt_pk_array_top(man, dim, 0);

		opt_pk_array_t* polyhedron1;
		if (create_polyhedron(&polyhedron1, man, top, dim, data, dataSize, &dataIndex,
				fp)) {

			//meet == glb, join == lub
			//x join top == top
			if (!opt_pk_is_eq(man, opt_pk_join(man, false, top, polyhedron1),
					top)) {
				fclose(fp);
				return 1;
			}
		}
	}
	fclose(fp);
	return 0;
}

