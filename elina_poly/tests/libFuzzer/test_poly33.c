#include <time.h>
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
	FILE *fp;
	fp = fopen("out33.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_pk_manager_alloc(false);
		opt_pk_array_t * top = opt_pk_top(man, dim, 0);

		opt_pk_array_t* polyhedron1;
		if (create_polyhedron(&polyhedron1, man, top, dim, data, dataSize,
				&dataIndex, fp)) {
			opt_pk_array_t* polyhedron2;
			if (create_polyhedron(&polyhedron2, man, top, dim, data, dataSize,
					&dataIndex, fp)) {

				// assignment is monotone
				if (assume_fuzzable(
						opt_pk_is_leq(man, polyhedron1, polyhedron2))) {

					int assignedToVariable;
					if (create_variable(&assignedToVariable, dim, data,
							dataSize, &dataIndex, fp)) {
						elina_linexpr0_t** assignmentArray;
						elina_dim_t * tdim;

						if (create_assignment(&assignmentArray,
								assignedToVariable, &tdim, dim, data, dataSize,
								&dataIndex, fp)) {

							opt_pk_array_t* assign_result1 =
									opt_pk_assign_linexpr_array(man,
									DESTRUCTIVE, polyhedron1, tdim,
											assignmentArray, 1,
											NULL);
							opt_pk_internal_t * assign1_internal =
									opt_pk_init_from_manager(man,
											ELINA_FUNID_ASSIGN_LINEXPR_ARRAY);

							opt_pk_array_t* assign_result2 =
									opt_pk_assign_linexpr_array(man,
									DESTRUCTIVE, polyhedron2, tdim,
											assignmentArray, 1,
											NULL);
							opt_pk_internal_t * assign2_internal =
									opt_pk_init_from_manager(man,
											ELINA_FUNID_ASSIGN_LINEXPR_ARRAY);

							if (assign1_internal->exn != ELINA_EXC_OVERFLOW
									&& assign2_internal->exn
											!= ELINA_EXC_OVERFLOW) {

								if (opt_pk_is_leq(man, assign_result1,
										assign_result2) == false) {
									opt_pk_free(man, top);
									opt_pk_free(man, polyhedron1);
									opt_pk_free(man, polyhedron2);
									opt_pk_free(man, assign_result1);
									opt_pk_free(man, assign_result2);
									elina_manager_free(man);
									fclose(fp);
									return 1;
								}
							}
							opt_pk_free(man, assign_result1);
							opt_pk_free(man, assign_result2);
						}
					}
				}
				opt_pk_free(man, polyhedron2);
			}
			opt_pk_free(man, polyhedron1);
		}
		opt_pk_free(man, top);
		elina_manager_free(man);
	}
	fclose(fp);
	return 0;
}
