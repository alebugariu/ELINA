all: compile test
compile: 
	cd elina_auxiliary ; \
	clang -I /home/klee/klee_src/include -emit-llvm -O0 -c -g *.c ; \
	cd ../elina_linearize ; \
	clang -I /home/klee/klee_src/include -emit-llvm -O0 -c -g *.c ; \
	cd ../partitions_api ; \
	clang -I /home/klee/klee_src/include -emit-llvm -O0 -c -g *.c ; \
	cd ../elina_oct ; \
	clang -I /home/klee/klee_src/include -emit-llvm -O0 -c -g *.c ; \
	cd ..       
 
test:
	cd elina_oct/tests/klee; \
        #rm -rf klee-*; \
	clang -I /home/klee/klee_src/include -emit-llvm -O0 -c -g *.c ; \
        number=$(start) ; while [ $${number} -le $(number) ] ; do \
		llvm-link ../../../elina_auxiliary/*.bc ../../../partitions_api/*.bc ../../../elina_linearize/*.bc ../../opt_oct_closure_comp_sparse.bc ../../opt_oct_incr_closure_comp_sparse.bc ../../opt_oct_closure_dense.bc ../../opt_oct_incr_closure_dense.bc ../../opt_oct_nary.bc ../../opt_oct_resize.bc ../../opt_oct_predicate.bc ../../opt_oct_representation.bc ../../opt_oct_transfer.bc ../../opt_oct_hmat.bc ../../opt_oct_closure_dense_scalar.bc ../../opt_oct_incr_closure_dense_scalar.bc test_oct.bc test_oct$${number}.bc -o test$${number}.bc ; \
                startTime=`date +%s` ; \
                klee -allow-external-sym-calls -max-forks=300 -solver-backend=stp -sym-malloc-bound=128 test$${number}.bc ; \
                endTime=`date +%s` ; \
                runtime=`expr $$endTime - $$startTime` ; \
                echo "Execution time: $$runtime sec for test_oct$$number\n" ; \
		number=`expr $$number + 1` ; \
    	done; \
        true
