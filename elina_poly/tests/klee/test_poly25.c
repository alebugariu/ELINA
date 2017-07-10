#include <time.h>
#include <klee/klee.h>
#include "opt_pk_internal.h"
#include "test_poly.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
	unsigned short int dim = make_symbolic_dimension();

	elina_manager_t * man = opt_pk_manager_alloc(false);
	opt_pk_array_t * top = opt_pk_top(man, dim, 0);
	opt_pk_array_t * bottom = opt_pk_bottom(man, dim, 0);

	opt_pk_array_t * polyhedron1 = create_polyhedron(man, top, "1", dim);
	opt_pk_array_t * polyhedron2 = create_polyhedron(man, top, "2", dim);
	opt_pk_array_t * lub = opt_pk_join(man, false, polyhedron1, polyhedron2);

	opt_pk_array_t * bound = create_polyhedron(man, top, "possible bound", dim);

	//meet == glb, join == lub
	//join is the least upper bound
	klee_assume(opt_pk_is_leq(man, polyhedron1, bound));
	klee_assume(opt_pk_is_leq(man, polyhedron2, bound));
	klee_assert(opt_pk_is_leq(man, lub, bound));
	return 0;
}

