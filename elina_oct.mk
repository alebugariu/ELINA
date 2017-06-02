number ?= 24
start ?= 0

all: compile test
compile: 
	clang -I /home/klee/klee_src/include -emit-llvm -c -g elina_auxiliary/*.c; \
	clang -I /home/klee/klee_src/include -emit-llvm -c -g elina_linearize/*.c; \
	clang -I /home/klee/klee_src/include -emit-llvm -c -g partitions_api/*.c; \
	clang -I /home/klee/klee_src/include -emit-llvm -c -g elina_oct/*.c; \

 

test:
	cd elina_oct/tests; \
        rm -rf klee-*; \
	clang -I /home/klee/klee_src/include -emit-llvm -c -g *.c ; \
        number=$(start) ; while [ $${number} -le $(number) ] ; do \
		llvm-link ../../elina_auxiliary/*.bc ../../partitions_api/*.bc ../../elina_linearize/*.bc ../opt_oct_closure_comp_sparse.bc ../opt_oct_incr_closure_comp_sparse.bc ../opt_oct_closure_dense.bc ../opt_oct_incr_closure_dense.bc ../opt_oct_nary.bc ../opt_oct_resize.bc ../opt_oct_predicate.bc ../opt_oct_representation.bc ../opt_oct_transfer.bc ../opt_oct_hmat.bc ../opt_oct_closure_dense_scalar.bc ../opt_oct_incr_closure_dense_scalar.bc test_oct.bc test_oct$${number}.bc -o test$${number}.bc ; \
                startTime=`date +%s` ; \
                klee -allow-external-sym-calls -sym-malloc-bound=128 -max-depth=6 test$${number}.bc ; \
                endTime=`date +%s` ; \
                runtime=`expr $$endTime - $$startTime` ; \
                echo "Execution time: $$runtime sec for test_oct$$number\n" ; \
		number=`expr $$number + 1` ; \
    	done; \
        true
