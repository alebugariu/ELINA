#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const int *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
		FILE *fp;
		fp = fopen("out25.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_oct_manager_alloc();
		opt_pk_array_t * top = opt_pk_array_top(man, dim, 0);
		opt_pk_array_t * bottom = opt_oct_bottom(man, dim, 0);

		opt_pk_array_t* polyhedron1;
		if (create_polyhedron(&polyhedron1, man, top, dim, data, dataSize,
				&dataIndex, fp)) {
			opt_pk_array_t* polyhedron2;
			if (create_polyhedron(&polyhedron2, man, top, dim, data, dataSize,
					&dataIndex, fp)) {
				opt_pk_array_t* lub = opt_pk_join(man, false, polyhedron1, polyhedron2);

				opt_pk_array_t* bound;
				if (create_polyhedron(&bound, man, top, dim, data, dataSize,
						&dataIndex, fp)) {
					//meet == glb, join == lub
					//join is the least upper bound
					if (assume_fuzzable(opt_pk_is_leq(man, polyhedron1, bound))) {
						if (assume_fuzzable(
								opt_pk_is_leq(man, polyhedron2, bound))) {
							if (!opt_pk_is_leq(man, lub, bound)) {
								fclose(fp);
								return 1;
							}
						}
					}
				}
			}
		}
	}
	fclose(fp);
	return 0;
}

