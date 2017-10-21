#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
	FILE *fp;
	fp = fopen("out14.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_pk_manager_alloc(false);
		opt_pk_array_t * top = opt_pk_top(man, dim, 0);
		opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

		opt_pk_array_t* polyhedron1;
		if (create_polyhedron(&polyhedron1, man, top, dim, data, dataSize,
				&dataIndex, fp)) {
			opt_pk_array_t* polyhedron2;
			if (create_polyhedron(&polyhedron2, man, top, dim, data, dataSize,
					&dataIndex, fp)) {

				opt_pk_array_t* meet12 = opt_pk_meet(man, DESTRUCTIVE,
						polyhedron1, polyhedron2);
				opt_pk_internal_t * meet12_internal = opt_pk_init_from_manager(
						man, ELINA_FUNID_MEET);

				opt_pk_array_t* join12 = opt_pk_join(man, DESTRUCTIVE,
						polyhedron1, meet12);
				opt_pk_internal_t * join12_internal = opt_pk_init_from_manager(
						man, ELINA_FUNID_JOIN);

				if (meet12_internal->exn != ELINA_EXC_OVERFLOW
						&& join12_internal->exn != ELINA_EXC_OVERFLOW) {

					//meet == glb, join == lub
					//join absorbtion
					if (opt_pk_is_eq(man, join12, polyhedron1) == false) {
						opt_pk_free(man, top);
						opt_pk_free(man, bottom);
						opt_pk_free(man, polyhedron1);
						opt_pk_free(man, polyhedron2);
						opt_pk_free(man, meet12);
						opt_pk_free(man, join12);
						elina_manager_free(man);
						fclose(fp);
						return 1;
					}
				}
				opt_pk_free(man, polyhedron2);
				opt_pk_free(man, meet12);
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

