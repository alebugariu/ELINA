#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out32.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		//meet == glb, join == lub
		//widening reaches a fixed point
		opt_pk_array_t* polyhedron1;
		unsigned char number1;
		if (get_polyhedron(&polyhedron1, man, top, &number1, data, dataSize,
				&dataIndex, fp)) {

			opt_pk_array_t* wideningResult;
			int i = 0;
			while (true) {
				opt_pk_array_t* polyhedron2;
				unsigned char number2;
				if (get_polyhedron(&polyhedron2, man, top, &number2, data,
						dataSize, &dataIndex, fp)) {

					wideningResult = opt_pk_widening(man, polyhedron1,
							opt_pk_join(man, DESTRUCTIVE, polyhedron1,
									polyhedron2));
					if (opt_pk_is_leq(man, wideningResult, polyhedron1)
							== true) {
						break; // we reached a fixed point
					}
					polyhedron1 = wideningResult;
					i++;
					if (!(R(i))) {
						fprintf(fp, "found polyhedron %d!\n", number1);
						print_polyhedron(man, polyhedron1, number1, fp);
						fprintf(fp, "found polyhedron %d!\n", number2);
						print_polyhedron(man, polyhedron2, number2, fp);
						fflush(fp);
						free_pool(man);
						free_polyhedron(man, &top);
						free_polyhedron(man, &bottom);
						free_polyhedron(man, &polyhedron1);
						free_polyhedron(man, &polyhedron2);
						elina_manager_free(man);
						fclose(fp);
						return 1;
					}
				} else {
					break;
				}
			}
			free_polyhedron(man, &polyhedron1);
		}
		free_polyhedron(man, &top);
		free_polyhedron(man, &bottom);
	}
	elina_manager_free(man);
	fclose(fp);
	return 0;
}

