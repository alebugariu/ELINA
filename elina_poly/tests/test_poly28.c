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

	//meet == glb, join == lub
	//y <= x widening y
	klee_assert(
			opt_pk_is_leq(man, polyhedron2,
					opt_pk_widening(man, polyhedron1, polyhedron2)));
	return 0;
}
