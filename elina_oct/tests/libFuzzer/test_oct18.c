#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	FILE *fp;
	fp = fopen("out18.txt", "w+");

	int dim = create_dimension(fp);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	if (create_pool(man, top, bottom, dim, data, dataSize, &dataIndex, fp)) {

		opt_oct_t* octagon1;
		if (get_octagon_from_pool(&octagon1, man, top, bottom, dim, data,
				dataSize, &dataIndex, fp)) {
			opt_oct_t* octagon2;
			if (get_octagon_from_pool(&octagon2, man, top, bottom, dim, data,
					dataSize, &dataIndex, fp)) {

				//meet == glb, join == lub
				//x meet y <= y
				if (!opt_oct_is_leq(man,
						opt_oct_meet(man, DESTRUCTIVE, octagon1, octagon2),
						octagon2)) {
					opt_oct_free(man, top);
					opt_oct_free(man, bottom);
					opt_oct_free(man, octagon1);
					opt_oct_free(man, octagon2);
					elina_manager_free(man);
					fclose(fp);
					return 1;
				}
				opt_oct_free(man, octagon2);
			}
			opt_oct_free(man, octagon1);
		}
		opt_oct_free(man, top);
		opt_oct_free(man, bottom);
		elina_manager_free(man);
	}
	fclose(fp);
	return 0;
}

