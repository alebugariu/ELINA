#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const int *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
		FILE *fp;
		fp = fopen("out20.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_oct_manager_alloc();
		opt_oct_t * top = opt_oct_top(man, dim, 0);
		opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

		opt_oct_t* octagon1;
		if (create_octagon(&octagon1, man, top, dim, data, dataSize,
				&dataIndex, fp)) {
			opt_oct_t* octagon2;
			if (create_octagon(&octagon2, man, top, dim, data, dataSize,
					&dataIndex, fp)) {
				opt_oct_t* octagon3;
				if (create_octagon(&octagon3, man, top, dim, data, dataSize,
						&dataIndex, fp)) {

					//meet == glb, join == lub
					//meet is associative
					if (!opt_oct_is_eq(man,
							opt_oct_meet(man, false,
									opt_oct_meet(man, false, octagon1,
											octagon2), octagon3),
							opt_oct_meet(man, false, octagon1,
									opt_oct_meet(man, false, octagon2,
											octagon3)))) {
						fclose(fp);
						return 1;
					}
				}
			}
		}
	}
	fclose(fp);
	return 0;
}

