INCLUDES_FOR_POLY=-I /home/elina/elina_auxiliary -I /home/elina/elina_linearize -I /home/elina/partitions_api
INCLUDES=-I /home/elina/elina_poly $(INCLUDES_FOR_POLY)
OBJS=/home/elina/elina_poly/*.o /home/elina/elina_auxiliary/*.o /home/elina/elina_linearize/*.o /home/elina/partitions_api/*.o
all: compile test
compile: 
	cd elina_auxiliary ; \
	clang -DTHRESHOLD=0.75 -DNUM_DOUBLE -O0 -c -g *.c; \
	cd ../elina_linearize ; \
	clang -DTHRESHOLD=0.75 -DNUM_DOUBLE -I ../elina_auxiliary -O0 -c -g *.c; \
	cd ../partitions_api ; \
	clang -DTHRESHOLD=0.75 -DNUM_DOUBLE -O0 -c -g *.c; \
	cd ../elina_poly ; \
	clang -DTHRESHOLD=0.75 -DNUM_DOUBLE $(INCLUDES_FOR_POLY) -O0 -c -g *.c; \
	rm -fr elina_test_poly.o; \
	cd ..        
 
test:
	cd elina_poly/tests/libFuzzer; \
	clang -DTHRESHOLD=0.75 -DNUM_DOUBLE $(INCLUDES) -O0 -c -g test_poly.c; \
        cd failing_tests; \
        number=$(start) ; while [ $${number} -le $(number) ] ; do \
		clang -lstdc++ -DTHRESHOLD=0.75 -DNUM_DOUBLE -I /usr/local/include $(INCLUDES) $(OBJS) test$${number}.c -o test$${number} -lmpfr -lgmp -lm; \
                startTime=`date +%s` ; \
		./test$${number}; \
                endTime=`date +%s` ; \
                runtime=`expr $$endTime - $$startTime` ; \
                echo "Execution time: $$runtime sec for test_poly$$number\n" ; \
		number=`expr $$number + 1` ; \
    	done; \
        true
