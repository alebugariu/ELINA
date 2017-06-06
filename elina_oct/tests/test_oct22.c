#include <time.h>
#include <klee/klee.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

bool is_meet_compatible_direct(elina_manager_t * man, opt_oct_t * x,
		opt_oct_t * y) {
	if (opt_oct_is_leq(man, x, y)) {
		return opt_oct_is_eq(man, opt_oct_meet(man, false, x, y), x);
	}
	return true;
}

int main(int argc, char **argv) {
	unsigned short int dim;
	make_symbolic_dimension(&dim);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	opt_oct_t* octagon1 = create_octagon(man, top, "1", dim);
	opt_oct_t* octagon2 = create_octagon(man, top, "2", dim);

	//meet == glb, join == lub
	//meet is compatible (direct)
	klee_assert(is_meet_compatible_direct(man, octagon1, octagon2));
	return 0;
}
