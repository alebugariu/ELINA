#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out20.txt", "w+");

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

				opt_pk_array_t* polyhedron3;
				unsigned char number3;
				if (get_polyhedron(&polyhedron3, man, top, &number3, data,
						dataSize, &dataIndex, fp)) {

					opt_pk_array_t* meet12 = opt_pk_meet(man, DESTRUCTIVE,
							polyhedron1, polyhedron2);
					opt_pk_internal_t * meet12_internal =
							opt_pk_init_from_manager(man, ELINA_FUNID_MEET);

					opt_pk_array_t* meet12_3 = opt_pk_meet(man, DESTRUCTIVE,
							meet12, polyhedron3);
					opt_pk_internal_t * meet12_3_internal =
							opt_pk_init_from_manager(man, ELINA_FUNID_MEET);

					opt_pk_array_t* meet23 = opt_pk_meet(man, DESTRUCTIVE,
							polyhedron2, polyhedron3);
					opt_pk_internal_t * meet23_internal =
							opt_pk_init_from_manager(man, ELINA_FUNID_MEET);

					opt_pk_array_t* meet1_23 = opt_pk_meet(man, DESTRUCTIVE,
							polyhedron1, meet23);
					opt_pk_internal_t * meet1_23_internal =
							opt_pk_init_from_manager(man, ELINA_FUNID_MEET);

					if (meet12_internal->exn != ELINA_EXC_OVERFLOW
							&& meet12_3_internal->exn != ELINA_EXC_OVERFLOW
							&& meet23_internal->exn != ELINA_EXC_OVERFLOW
							&& meet1_23_internal->exn != ELINA_EXC_OVERFLOW) {

						//meet == glb, join == lub
						//meet is associative
						if (opt_pk_is_eq(man, meet12_3, meet1_23) == false) {
							fprintf(fp, "found polyhedron %d!\n", number1);
							print_polyhedron(man, polyhedron1, number1, fp);
							fprintf(fp, "found polyhedron %d!\n", number2);
							print_polyhedron(man, polyhedron2, number2, fp);
							fprintf(fp, "found polyhedron %d!\n", number3);
							print_polyhedron(man, polyhedron3, number3, fp);
							fflush(fp);
							free_pool(man);
							free_polyhedron(man, &top);
							free_polyhedron(man, &bottom);
							free_polyhedron(man, &polyhedron1);
							free_polyhedron(man, &polyhedron2);
							free_polyhedron(man, &polyhedron3);
							opt_pk_free(man, meet12);
							opt_pk_free(man, meet12_3);
							opt_pk_free(man, meet23);
							opt_pk_free(man, meet1_23);
							elina_manager_free(man);
							fclose(fp);
							return 1;
						}
					}
					free_polyhedron(man, &polyhedron3);
					opt_pk_free(man, meet12);
					opt_pk_free(man, meet12_3);
					opt_pk_free(man, meet23);
					opt_pk_free(man, meet1_23);
				}
				free_polyhedron(man, &polyhedron2);
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

