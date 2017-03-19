#include <time.h>
#include <klee/klee.h>
#include "opt_oct.h"
#include "opt_oct_internal.h"
#include "opt_oct_hmat.h"
#include <string.h>


elina_linexpr0_t * create_linexpr0(unsigned short int dim, int v1, int v2, int coeff1, int coeff2, char * octagonNumber){
	elina_coeff_t *cst, *coeff;
	elina_linexpr0_t * linexpr0 = elina_linexpr0_alloc(ELINA_LINEXPR_SPARSE,2);
	cst = &linexpr0->cst;

	char buffer_scalar[80] = "scalar value for octagon ";
	elina_int_t scalar_value;
	klee_make_symbolic(&scalar_value, sizeof(scalar_value), strcat(buffer_scalar, octagonNumber));
	//otherwise scalar_value is concretized to 0, because klee cannot deal with floats
	klee_assume(scalar_value >= dim & scalar_value < 10);
	elina_scalar_reinit(cst->val.scalar, ELINA_SCALAR_DOUBLE);
	cst->val.scalar->val.dbl = (double)scalar_value;

	elina_linterm_t * linterm = &linexpr0->p.linterm[0];
	linterm->dim = v1;
	coeff = &linterm->coeff;
	elina_scalar_reinit(coeff->val.scalar, ELINA_SCALAR_DOUBLE);
	coeff->val.scalar->val.dbl = (double)coeff1;

	linterm = &linexpr0->p.linterm[1];
	linterm->dim = v2;
	coeff = &linterm->coeff;
	elina_scalar_reinit(coeff->val.scalar, ELINA_SCALAR_DOUBLE);
	coeff->val.scalar->val.dbl = (double)coeff2;
	return linexpr0;
}

elina_lincons0_array_t create_constraints(unsigned short int dim, size_t nbcons, char * octagonNumber){
	size_t i;
	elina_lincons0_array_t  lincons0 = elina_lincons0_array_make(nbcons);
	char buffer_type[80] = "type for octagon ";
	char buffer_v1[80] = "v1 for octagon ";
	char buffer_v2[80] = "v2 for octagon ";
	char buffer_coeff1[80] = "coeff1 for octagon ";
	char buffer_coeff2[80] = "coeff2 for octagon ";
	for(i=0; i < nbcons; i++){
		elina_constyp_t type;
		klee_make_symbolic(&type, sizeof(type), strcat(buffer_type, octagonNumber));
		klee_assume(type == ELINA_CONS_SUPEQ);
		lincons0.p[i].constyp = type;
		int v1, v2;
		klee_make_symbolic(&v1, sizeof(v1), strcat(buffer_v1, octagonNumber));
		klee_make_symbolic(&v2, sizeof(v2), strcat(buffer_v2, octagonNumber));
		klee_assume(v1 < dim & v2 < dim & v1 != v2 & v1 >=0 & v2 >= 0);
		int coeff1, coeff2;
		klee_make_symbolic(&coeff1, sizeof(coeff1), strcat(buffer_coeff1, octagonNumber));
	    klee_make_symbolic(&coeff2, sizeof(coeff2), strcat(buffer_coeff2, octagonNumber));
        klee_assume(coeff1 == 1 | coeff1 == -1);
        klee_assume(coeff2 == 1 | coeff2 == -1);
		elina_linexpr0_t * linexpr0 = create_linexpr0(dim,v1,v2,coeff1,coeff2, octagonNumber);
		lincons0.p[i].linexpr0 = linexpr0;
	}
	return lincons0;
}

opt_oct_t* create_octagon(elina_manager_t* man, opt_oct_t * top, char * octagonNumber, unsigned short int dim, size_t nbcons) {
	elina_lincons0_array_t constraints = create_constraints(dim, nbcons, octagonNumber);
	//elina_lincons0_array_fprint(stdout,&constraints, NULL);
	opt_oct_t* octagon = opt_oct_meet_lincons_array(man, false, top, &constraints);
	char buffer[80] = "octagon ";

	klee_assume(!opt_oct_is_bottom(man, octagon));
	//printf("Created not bottom octagon %s!\n", octagonNumber);
	//elina_lincons0_array_fprint(stdout,&constraints,NULL);
	return octagon;
}

int main(int argc, char **argv){
	unsigned short int dim;
	size_t nbcons;
	klee_make_symbolic(&dim, sizeof(dim), "number of variables");
	klee_make_symbolic(&nbcons, sizeof(nbcons), "number of constraints");
	klee_assume(dim >= 3 & dim < 10);
	// otherwise klee_assume(v1 < dim & v2 < dim & v1 != v2 & v1 >=0 & v2 >= 0); is provably false, because the value of dim is concretized
	klee_assume(nbcons > 0 & nbcons < 10);

	elina_manager_t * man = opt_oct_manager_alloc();
	opt_oct_t * top = opt_oct_top(man, dim, 0);
	opt_oct_t * bottom = opt_oct_bottom(man, dim, 0);

	opt_oct_t* octagon1 = create_octagon(man, top, "1", dim, nbcons);
	klee_assert(opt_oct_is_leq(man, bottom, octagon1));
	klee_assert(opt_oct_is_leq(man, octagon1, top));

	//meet == glb, join == lub
	klee_assert(opt_oct_is_eq(man, opt_oct_join(man, false, bottom, octagon1),octagon1));
	klee_assert(opt_oct_is_eq(man, opt_oct_join(man, false, top, octagon1), top));

	opt_oct_t* octagon2 = create_octagon(man, top, "2", dim, nbcons);
	klee_assert(opt_oct_is_leq(man, octagon2 , opt_oct_join(man, false, octagon1, octagon2)));
	klee_assert(opt_oct_is_leq(man, opt_oct_meet(man, false, octagon1, octagon2), octagon1));
	return 0;
}
