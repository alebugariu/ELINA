#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	long dim;
	FILE *fp;
	fp = fopen("out3.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_pk_manager_alloc(false);
		opt_pk_array_t * top = opt_pk_top(man, dim, 0);

		opt_pk_array_t* polyhedron1;
		if (create_polyhedron(&polyhedron1, man, top, dim, data, dataSize, &dataIndex,
				fp)) {
			opt_pk_array_t* polyhedron2;
			if (create_polyhedron(&polyhedron2, man, top, dim, data, dataSize,
					&dataIndex, fp)) {
				opt_pk_array_t* polyhedron3;
				if (create_polyhedron(&polyhedron3, man, top, dim, data, dataSize,
						&dataIndex, fp)) {

					// <= is transitive
					if (assume_fuzzable(
							opt_pk_is_leq(man, polyhedron1, polyhedron2)
									&& opt_pk_is_leq(man, polyhedron2,
											polyhedron3))) {
						if (!opt_pk_is_leq(man, polyhedron1, polyhedron3)) {
							fclose(fp);
							return 1;
						}
					}
				}
			}
		}
	}
	fclose(fp);
	return 0;
}

