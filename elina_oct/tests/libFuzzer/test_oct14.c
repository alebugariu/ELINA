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
		opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

		opt_oct_t* octagon1;
		if (create_octagon(octagon1, man, top, dim, data, dataSize,
				&dataIndex)) {
			opt_oct_t* octagon2;
			if (create_octagon(octagon2, man, top, dim, data, dataSize,
					&dataIndex)) {

				//meet == glb, join == lub
				//join absorbtion
				if (!opt_oct_is_eq(man,
						opt_oct_join(man, false, octagon1,
								opt_oct_meet(man, false, octagon1, octagon2)),
						octagon1)) {
					abort();
				}
			}
		}
	}
	return 0;
}

