#include <time.h>
#include <klee/klee.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>


bool is_join_compatible_reciprocal(elina_manager_t * man, opt_oct_t * x, opt_oct_t * y) {
	if (opt_oct_is_eq(man, opt_oct_join(man, false, x, y), y)) {
		return opt_oct_is_leq(man, x, y);
	}
	return true;
}

int main(int argc, char **argv) {
	unsigned short int dim;
	klee_make_symbolic(&dim, sizeof(dim), "number of variables");
	klee_assume(dim > 1);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	opt_oct_t* octagon1 = create_octagon(man, top, "1", dim);
	opt_oct_t* octagon2 = create_octagon(man, top, "2", dim);

	//meet == glb, join == lub
	//join is compatible (reciprocal)
	klee_assert(is_join_compatible_reciprocal(man, octagon1, octagon2));
	return 0;
}




