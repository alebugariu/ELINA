#include <time.h>
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out47.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		opt_oct_t* octagon1;
		unsigned char number1;
		if (get_octagon_from_pool(&octagon1, &number1, data, dataSize,
				&dataIndex)) {

			opt_oct_t* octagon2;
			unsigned char number2;
			if (get_octagon_from_pool(&octagon2, &number2, data, dataSize,
					&dataIndex)) {

				// conditional is monotone
				if (assume_fuzzable(opt_oct_is_leq(man, octagon1, octagon2))) {

					elina_lincons0_array_t conditionalArray;

					if (create_conditional(&conditionalArray, data, dataSize,
							&dataIndex, fp)) {

						opt_oct_t* cond_result1 = opt_oct_meet_lincons_array(
								man,
								DESTRUCTIVE, octagon1, &conditionalArray);

						opt_oct_t* cond_result2 = opt_oct_meet_lincons_array(
								man,
								DESTRUCTIVE, octagon2, &conditionalArray);

						if (opt_oct_is_leq(man, cond_result1, cond_result2)
								== false) {
							fprintf(fp, "found octagon %d!\n", number1);
							print_history(man, number1, fp);
							fflush(fp);
							fprintf(fp, "found octagon %d!\n", number2);
							print_history(man, number2, fp);
							fflush(fp);
							free_pool(man);
							opt_oct_free(man, cond_result1);
							opt_oct_free(man, cond_result2);
							elina_manager_free(man);
							fclose(fp);
							return 1;
						}
					}
				}
			}
		}
	}
	elina_manager_free(man);
	fclose(fp);
	return 0;
}
