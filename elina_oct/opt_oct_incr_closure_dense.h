/*
	Copyright 2016 Software Reliability Lab, ETH Zurich

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/


#ifndef __OPT_OCT_INCR_CLOSURE_DENSE_H_INCLUDED__
#define __OPT_OCT_INCR_CLOSURE_DENSE_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#include "opt_oct_hmat.h"

bool incremental_closure_opt_dense(opt_oct_mat_t *oo, int dim, int v, bool is_int);
double incremental_closure_calc_perf_dense(double cycles, int dim);

#ifdef __cplusplus
}
#endif

#endif
