#include <time.h>
#include <klee/klee.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
	unsigned short int dim;
	klee_make_symbolic(&dim, sizeof(dim), "number of variables");
	klee_assume(dim > 1);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	opt_oct_t* octagon1 = create_octagon(man, top, "1", dim);
    opt_oct_t* octagon2 = create_octagon(man, top, "2", dim);
    opt_oct_t* lub = opt_oct_join(man, false, octagon1, octagon2);

    opt_oct_t* bound = create_octagon(man, top, "possible bound", dim);

    //meet == glb, join == lub
	//join is the least upper bound
    klee_assume(opt_oct_is_leq(man, octagon1, bound));
    klee_assume(opt_oct_is_leq(man, octagon2, bound));
    klee_assume(opt_oct_is_leq(man, bound, lub));
	klee_assert(opt_oct_is_eq(man, bound, lub));
	return 0;
}




