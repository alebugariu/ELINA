number ?= 24
start ?= 0
all:
	cd elina_oct/tests; \
	clang -I /home/klee/klee_src/include -emit-llvm -c -g *.c ; \
        number=$(start) ; while [ $${number} -le $(number) ] ; do \
		llvm-link ../../elina_auxiliary/*.bc ../../partitions_api/*.bc ../../elina_linearize/*.bc ../opt_oct_closure_comp_sparse.bc ../opt_oct_incr_closure_comp_sparse.bc ../opt_oct_closure_dense.bc ../opt_oct_incr_closure_dense.bc ../opt_oct_nary.bc ../opt_oct_resize.bc ../opt_oct_predicate.bc ../opt_oct_representation.bc ../opt_oct_transfer.bc ../opt_oct_hmat.bc ../opt_oct_closure_dense_scalar.bc ../opt_oct_incr_closure_dense_scalar.bc test_oct.bc test_oct$${number}.bc -o test$${number}.bc ; \
                klee -allow-external-sym-calls -sym-malloc-bound=128 -max-depth=6 test$${number}.bc ; \
		number=`expr $$number + 1` ; \
    	done; \
        true
