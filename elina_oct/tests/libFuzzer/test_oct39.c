#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const uint64_t *data, size_t dataSize) {
	unsigned int dataIndex = 0;
		size_t dim = MIN_DIM;

		if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex)) {

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	//meet == glb, join == lub
	//widening reaches a fixed point
	opt_oct_t* octagon1 = create_octagon(man, top, "1", dim);
	opt_oct_t* narrowingResult;
	int i = 0;
	while (true) {
		opt_oct_t* octagon2 = create_octagon(man, top, "2", dim);
		narrowingResult = opt_oct_narrowing(man, octagon1, octagon2);
		if (opt_oct_is_leq(man, octagon1, narrowingResult)) {
			break; // we reached a fixed point
		}
		octagon1 = narrowingResult;
		i++;
		klee_assert(R(i));
	}
		}
	return 0;
}

