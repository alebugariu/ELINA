#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
	FILE *fp;
	fp = fopen("out25.txt", "w+");

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		opt_pk_array_t* polyhedron1;
		unsigned char number1;
		if (get_polyhedron(&polyhedron1, man, top, &number1, data, dataSize, &dataIndex, fp)) {

			opt_pk_array_t* polyhedron2;
			unsigned char number2;
			if (get_polyhedron(&polyhedron2, man, top, &number2, data, dataSize, &dataIndex, fp)) {
				opt_pk_array_t* lub = opt_pk_join(man, DESTRUCTIVE, polyhedron1,
						polyhedron2);
				opt_pk_internal_t * lub_internal = opt_pk_init_from_manager(man,
						ELINA_FUNID_JOIN);

				if (lub_internal->exn != ELINA_EXC_OVERFLOW) {

					opt_pk_array_t* bound;
					if (create_polyhedron(&bound, man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {
						//meet == glb, join == lub
						//join is the least upper bound
						if (assume_fuzzable(
								opt_pk_is_leq(man, polyhedron1, bound))) {
							if (assume_fuzzable(
									opt_pk_is_leq(man, polyhedron2, bound))) {
								if (opt_pk_is_leq(man, lub, bound) == false) {
									opt_pk_free(man, top);
									opt_pk_free(man, bottom);
									opt_pk_free(man, polyhedron1);
									opt_pk_free(man, polyhedron2);
									opt_pk_free(man, bound);
									opt_pk_free(man, lub);
									elina_manager_free(man);
									fclose(fp);
									return 1;
								}
							}
						}
						opt_pk_free(man, bound);
					}
				}
				opt_pk_free(man, polyhedron2);
				opt_pk_free(man, lub);
			}
			opt_pk_free(man, polyhedron1);
		}
		opt_pk_free(man, top);
		opt_pk_free(man, bottom);
		elina_manager_free(man);
	}
	fclose(fp);
	return 0;
}

