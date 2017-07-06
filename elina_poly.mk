all: compile test
compile: 
	cd elina_auxiliary ; \
	clang -I /home/klee/klee_src/include -emit-llvm -O0 -c -g *.c ; \
	cd ../elina_linearize ; \
	clang -I /home/klee/klee_src/include -emit-llvm -O0 -c -g *.c ; \
	cd ../partitions_api ; \
	clang -I /home/klee/klee_src/include -emit-llvm -O0 -c -g *.c ; \
	cd ../elina_poly ; \
	clang -I /home/klee/klee_src/include -emit-llvm -O0 -c -g *.c ; \
	rm -f elina_test_poly.bc ; \
	cd ..       
 
test:
	cd elina_poly/tests; \
        #rm -rf klee-*; \
	clang -I /home/klee/klee_src/include -I ../ -emit-llvm -O0 -c -g *.c ; \
        number=$(start) ; while [ $${number} -le $(number) ] ; do \
		llvm-link ../../elina_auxiliary/*.bc ../../partitions_api/*.bc ../../elina_linearize/*.bc ../*.bc test_poly.bc test_poly$${number}.bc -o test$${number}.bc ; \
                startTime=`date +%s` ; \
                klee -load=/usr/local/lib/libgmp.so.10.3.2 -allow-external-sym-calls -max-forks=300 -check-overshift=false -solver-backend=stp -sym-malloc-bound=256 test$${number}.bc ; \
                endTime=`date +%s` ; \
                runtime=`expr $$endTime - $$startTime` ; \
                echo "Execution time: $$runtime sec for test_poly$$number\n" ; \
		number=`expr $$number + 1` ; \
    	done; \
        true
