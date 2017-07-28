#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const int *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
	FILE *fp;
	fp = fopen("out32.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_pk_manager_alloc(false);
		opt_pk_array_t * top = opt_pk_top(man, dim, 0);
		opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

		//meet == glb, join == lub
		//widening reaches a fixed point
		opt_pk_array_t* polyhedron1;
		if (create_polyhedron(&polyhedron1, man, top, dim, data, dataSize, &dataIndex,
				fp)) {
			opt_pk_array_t* wideningResult;
			int i = 0;
			while (true) {
				opt_pk_array_t* polyhedron2;
				if (create_polyhedron(&polyhedron2, man, top, dim, data, dataSize,
						&dataIndex, fp)) {
					wideningResult = opt_oct_widening(man, polyhedron1, polyhedron2);
					if (opt_pk_is_leq(man, wideningResult, polyhedron1)) {
						break; // we reached a fixed point
					}
					polyhedron1 = wideningResult;
					i++;
					if (!(R(i))) {
						fclose(fp);
						return 1;
					}
				} else {
					break;
				}
			}
		}
	}
	fclose(fp);
	return 0;
}

