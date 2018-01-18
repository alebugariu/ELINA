#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out25.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		opt_pk_array_t* polyhedron1;
		unsigned char number1;
		if (get_polyhedron(&polyhedron1, man, top, &number1, data, dataSize,
				&dataIndex, fp)) {

			opt_pk_array_t* polyhedron2;
			unsigned char number2;
			if (get_polyhedron(&polyhedron2, man, top, &number2, data, dataSize,
					&dataIndex, fp)) {

				opt_pk_array_t* lub = opt_pk_join(man, DESTRUCTIVE, polyhedron1,
						polyhedron2);
				unsigned char number3;
				opt_pk_internal_t * lub_internal = opt_pk_init_from_manager(man,
						ELINA_FUNID_JOIN);

				if (lub_internal->exn != ELINA_EXC_OVERFLOW) {

					opt_pk_array_t* bound;
					if (get_polyhedron(&bound, man, top, &number3, dim, data,
							dataSize, &dataIndex, fp)) {
						//meet == glb, join == lub
						//join is the least upper bound
						if (assume_fuzzable(
								opt_pk_is_leq(man, polyhedron1, bound))) {
							if (assume_fuzzable(
									opt_pk_is_leq(man, polyhedron2, bound))) {
								if (opt_pk_is_leq(man, lub, bound) == false) {
									fprintf(fp, "found polyhedron %d!\n", number1);
									print_polyhedron(man, polyhedron1, number1, fp);
									fprintf(fp, "found polyhedron %d!\n", number2);
									print_polyhedron(man, polyhedron2, number2, fp);
									fprintf(fp, "found bound %d!\n", number3);
									print_polyhedron(man, bound, number3, fp);
									fflush(fp);
									free_pool(man);
									free_polyhedron(man, &top);
									free_polyhedron(man, &bottom);
									free_polyhedron(man, &polyhedron1);
									free_polyhedron(man, &polyhedron2);
									free_polyhedron(man, &bound);
									opt_pk_free(man, lub);
									elina_manager_free(man);
									fclose(fp);
									return 1;
								}
							}
						}
						free_polyhedron(man, bound);
					}
				}
				free_polyhedron(man, &polyhedron2);
				opt_pk_free(man, lub);
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

