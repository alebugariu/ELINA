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

	//meet == glb, join == lub
	//widening reaches a fixed point
	opt_pk_array_t * polyhedron1 = create_polyhedron(man, top, "1", dim);
	opt_pk_array_t * wideningResult;
	int i = 0;
	while (true) {
		opt_pk_array_t * polyhedron2 = create_polyhedron(man, top, "2", dim);
		wideningResult = opt_pk_widening(man, polyhedron1, opt_pk_join(man, false, polyhedron1, polyhedron2));
		if (opt_pk_is_leq(man, wideningResult, polyhedron1)) {
			break; // we reached a fixed point
		}
		polyhedron1 = wideningResult;
		i++;
		klee_assert(R(i));
	}
	return 0;
}

