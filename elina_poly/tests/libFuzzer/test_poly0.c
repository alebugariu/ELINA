#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out0.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		opt_pk_array_t* polyhedron1;
		unsigned char number1;
		if (get_polyhedron(&polyhedron1, man, top, &number1, data,
				dataSize, &dataIndex, fp)) {
			// bottom <= x
			if (opt_pk_is_leq(man, bottom, polyhedron1) == false) {
				free_polyhedron(man, top);
				free_polyhedron(man, bottom);
				free_polyhedron(man, polyhedron1);
				elina_manager_free(man);
				fclose(fp);
				return 1;
			}
			free_polyhedron(man, polyhedron1);
		}
		free_polyhedron(man, top);
		free_polyhedron(man, bottom);
		elina_manager_free(man);
	}
	fclose(fp);
	return 0;
}
