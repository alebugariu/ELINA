#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const int *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	size_t dim = MIN_DIM;

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex)) {

		elina_manager_t * man = opt_oct_manager_alloc();
		opt_oct_t * top = opt_oct_top(man, dim, 0);

		opt_oct_t* octagon1;
		if (create_octagon(octagon1, man, top, dim, data, dataSize,
				&dataIndex)) {
			opt_oct_t* octagon2;
			if (create_octagon(octagon2, man, top, dim, data, dataSize,
					&dataIndex)) {

				// <= is anti symmetric
				if (assume_fuzzable(
						opt_oct_is_leq(man, octagon1, octagon2)
								&& opt_oct_is_leq(man, octagon2, octagon1))) {
					if (!opt_oct_is_eq(man, octagon1, octagon2)) {
						abort();
					}
				}
			}
		}
	}
	return 0;
}

