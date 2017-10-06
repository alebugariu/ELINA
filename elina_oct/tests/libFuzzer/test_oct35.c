#include <time.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include "test_oct.h"
#include <string.h>
#include <stdio.h>

extern int LLVMFuzzerTestOneInput(const long *data, size_t dataSize) {
	unsigned int dataIndex = 0;
	int dim;
	FILE *fp;
	fp = fopen("out35.txt", "w+");

	if (make_fuzzable_dimension(&dim, data, dataSize, &dataIndex, fp)) {

		elina_manager_t * man = opt_oct_manager_alloc();
		opt_oct_t * top = opt_oct_top(man, dim, 0);
		opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

		opt_oct_t* octagon1;
		if (create_octagon(&octagon1, man, top, dim, data, dataSize, &dataIndex,
				fp)) {

			//meet == glb, join == lub
			//x narrowing bottom == bottom
			if (!opt_oct_is_bottom(man,
					opt_oct_narrowing(man, octagon1, bottom))) {
				opt_oct_free(man, top);
				opt_oct_free(man, bottom);
				opt_oct_free(man, octagon1);
				elina_manager_free(man);
				fclose(fp);
				return 1;
			}
		}
	}
	fclose(fp);
	return 0;
}

