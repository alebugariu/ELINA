#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
	FILE *fp;
	fp = fopen("out3.txt", "w+");

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		opt_pk_array_t* polyhedron1;
		if (get_polyhedron(&polyhedron1, man, top, &number1, data, dataSize, &dataIndex, fp)) {

			opt_pk_array_t* polyhedron2;
			if (get_polyhedron(&polyhedron2, man, top, &number2, data, dataSize, &dataIndex, fp)) {

				opt_pk_array_t* polyhedron3;
				if (get_polyhedron(&polyhedron3, man, top, &number3, data, dataSize, &dataIndex, fp)) {

					// <= is transitive
					if (assume_fuzzable(
							opt_pk_is_leq(man, polyhedron1, polyhedron2)
									&& opt_pk_is_leq(man, polyhedron2,
											polyhedron3))) {
						if (opt_pk_is_leq(man, polyhedron1, polyhedron3) == false) {
							opt_pk_free(man, top);
							opt_pk_free(man, bottom);
							opt_pk_free(man, polyhedron1);
							opt_pk_free(man, polyhedron2);
							opt_pk_free(man, polyhedron3);
							elina_manager_free(man);
							fclose(fp);
							return 1;
						}
					}
					opt_pk_free(man, polyhedron3);
				}
				opt_pk_free(man, polyhedron2);
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

