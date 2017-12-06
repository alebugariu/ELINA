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
	fp = fopen("out39.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_oct_manager_alloc();
		opt_oct_t * top = opt_oct_top(man, dim, 0);
		opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

		//meet == glb, join == lub
		//narrowing reaches a fixed point
		opt_oct_t* octagon1;
		if (create_octagon(&octagon1, man, top, bottom, dim, data, dataSize,
				&dataIndex, fp)) {
			opt_oct_t* narrowingResult;
			int i = 0;
			while (true) {
				opt_oct_t* octagon2;
				if (create_octagon(&octagon2, man, top, bottom, dim, data,
						dataSize, &dataIndex, fp)) {
					narrowingResult = opt_oct_narrowing(man, octagon1,
							octagon2);
					if (opt_oct_is_leq(man, octagon1, narrowingResult)) {
						break; // we reached a fixed point
					}
					octagon1 = narrowingResult;
					i++;
					if (!(R(i))) {
						opt_oct_free(man, top);
						opt_oct_free(man, bottom);
						opt_oct_free(man, octagon1);
						opt_oct_free(man, octagon2);
						elina_manager_free(man);
						fclose(fp);
						return 1;
					}
				} else {
					break;
				}
			}
			opt_oct_free(man, octagon1);
		}
		opt_oct_free(man, top);
		opt_oct_free(man, bottom);
		elina_manager_free(man);
	}
	fclose(fp);
	return 0;
}

