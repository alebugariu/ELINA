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
	fp = fopen("out25.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_oct_manager_alloc();
		opt_oct_t * top = opt_oct_top(man, dim, 0);
		opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

		opt_oct_t* octagon1;
		if (create_octagon(&octagon1, man, top, dim, data, dataSize, &dataIndex,
				fp)) {
			opt_oct_t* octagon2;
			if (create_octagon(&octagon2, man, top, dim, data, dataSize,
					&dataIndex, fp)) {
				opt_oct_t* lub = opt_oct_join(man, false, octagon1, octagon2);

				opt_oct_t* bound;
				if (create_octagon(&bound, man, top, dim, data, dataSize,
						&dataIndex, fp)) {
					//meet == glb, join == lub
					//join is the least upper bound
					if (assume_fuzzable(opt_oct_is_leq(man, octagon1, bound))) {
						if (assume_fuzzable(
								opt_oct_is_leq(man, octagon2, bound))) {
							if (!opt_oct_is_leq(man, lub, bound)) {
								fclose(fp);
								opt_oct_free(man, top);
								opt_oct_free(man, bottom);
								opt_oct_free(man, octagon1);
								opt_oct_free(man, octagon2);
								elina_manager_free(man);
								return 1;
							}
						}
					}
				}
			}
		}
	}
	fclose(fp);
	return 0;
}

