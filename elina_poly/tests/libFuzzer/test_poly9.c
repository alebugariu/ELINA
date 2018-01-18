#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out9.txt", "w+");

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

				opt_pk_array_t* join12 = opt_pk_join(man, DESTRUCTIVE,
						polyhedron1, polyhedron2);
				opt_pk_internal_t * join12_internal = opt_pk_init_from_manager(
						man, ELINA_FUNID_JOIN);

				opt_pk_array_t* join21 = opt_pk_join(man, DESTRUCTIVE,
						polyhedron2, polyhedron1);
				opt_pk_internal_t * join21_internal = opt_pk_init_from_manager(
						man, ELINA_FUNID_JOIN);

				if (join12_internal->exn != ELINA_EXC_OVERFLOW
						&& join21_internal->exn != ELINA_EXC_OVERFLOW) {
					//meet == glb, join == lub
					//join is commutative
					if (opt_pk_is_eq(man, join12, join21) == false) {
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
						opt_pk_free(man, join12);
						opt_pk_free(man, join21);
						elina_manager_free(man);
						fclose(fp);
						return 1;
					}
				}
				free_polyhedron(man, &polyhedron2);
				opt_pk_free(man, join12);
				opt_pk_free(man, join21);
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

