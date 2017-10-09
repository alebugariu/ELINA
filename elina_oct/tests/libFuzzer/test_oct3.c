#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
	FILE *fp;
	fp = fopen("out3.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_oct_manager_alloc();
		opt_oct_t * top = opt_oct_top(man, dim, 0);

		opt_oct_t* octagon1;
		if (create_octagon(&octagon1, man, top, dim, data, dataSize, &dataIndex,
				fp)) {
			opt_oct_t* octagon2;
			if (create_octagon(&octagon2, man, top, dim, data, dataSize,
					&dataIndex, fp)) {
				opt_oct_t* octagon3;
				if (create_octagon(&octagon3, man, top, dim, data, dataSize,
						&dataIndex, fp)) {

					// <= is transitive
					if (assume_fuzzable(
							opt_oct_is_leq(man, octagon1, octagon2)
									&& opt_oct_is_leq(man, octagon2,
											octagon3))) {
						if (!opt_oct_is_leq(man, octagon1, octagon3)) {
							opt_oct_free(man, top);
							opt_oct_free(man, octagon1);
							opt_oct_free(man, octagon2);
							opt_oct_free(man, octagon3);
							elina_manager_free(man);
							fclose(fp);
							return 1;
						}
					}
					opt_oct_free(man, octagon3);
				}
				opt_oct_free(man, octagon2);
			}
			opt_oct_free(man, octagon1);
		}
		opt_oct_free(man, top);
		elina_manager_free(man);
	}
	fclose(fp);
	return 0;
}

