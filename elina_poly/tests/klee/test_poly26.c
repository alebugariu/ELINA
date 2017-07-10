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
	opt_pk_array_t * glb = opt_pk_meet(man, false, polyhedron1, polyhedron2);

	opt_pk_array_t * bound = create_polyhedron(man, top, "possible bound", dim);

	//meet == glb, join == lub
	//meet is the greatest lower bound
	klee_assume(opt_pk_is_leq(man, bound, polyhedron1));
	klee_assume(opt_pk_is_leq(man, bound, polyhedron2));
	klee_assert(opt_pk_is_leq(man, bound, glb));
	return 0;
}
