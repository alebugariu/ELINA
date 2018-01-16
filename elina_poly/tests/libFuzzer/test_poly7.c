#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
	FILE *fp;
	fp = fopen("out7.txt", "w+");

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		opt_pk_array_t* polyhedron1;
		if (get_polyhedron(&polyhedron1, man, top, &number1, data, dataSize, &dataIndex, fp)) {

			opt_pk_array_t* polyhedron2;
			if (get_polyhedron(&polyhedron2, man, top, &number2, data, dataSize, &dataIndex, fp)) {

				opt_pk_array_t* join12 = opt_pk_join(man, DESTRUCTIVE,
						polyhedron1, polyhedron2);
				opt_pk_internal_t * join12_internal = opt_pk_init_from_manager(
						man, ELINA_FUNID_JOIN);

				if (join12_internal->exn != ELINA_EXC_OVERFLOW) {

					//meet == glb, join == lub
					//x <= x join y
					if (opt_pk_is_leq(man, polyhedron1, join12) == false) {
						opt_pk_free(man, top);
						opt_pk_free(man, bottom);
						opt_pk_free(man, polyhedron1);
						opt_pk_free(man, polyhedron2);
						opt_pk_free(man, join12);
						elina_manager_free(man);
						fclose(fp);
						return 1;
					}
				}
				opt_pk_free(man, polyhedron2);
				opt_pk_free(man, join12);
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

