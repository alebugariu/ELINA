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

	//meet == glb, join == lub
	//x widening bottom == x
	klee_assume(opt_pk_is_leq(man, polyhedron1, bottom));
	klee_assert(
			opt_pk_is_eq(man, opt_pk_widening(man, polyhedron1, bottom),
					polyhedron1));
	return 0;
}

