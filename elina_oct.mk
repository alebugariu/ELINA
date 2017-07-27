INCLUDES=-I /home/elina/elina_oct -I /home/elina/elina_auxiliary -I /home/elina/elina_linearize -I /home/elina/partitions_api 
OBJS= /home/elina/elina_oct/*.o /home/elina/elina_auxiliary/*.o /home/elina/elina_linearize/*.o /home/elina/partitions_api/*.o
all: compile test
compile: 
	cd elina_auxiliary ; \
	clang -O0 -c -g -DTHRESHOLD=0.75 -Wall -DNUM_DOUBLE *.c; \
	cd ../elina_linearize ; \
	clang -I ../elina_auxiliary -O0 -Wall -c -g -DTHRESHOLD=0.75 -DNUM_DOUBLE  *.c; \
	cd ../partitions_api ; \
	clang  -O0 -c -g -DTHRESHOLD=0.75 -Wall -DNUM_DOUBLE *.c; \
	cd ../elina_oct ; \
	clang -I ../elina_auxiliary -I ../elina_linearize -I ../partitions_api -O0 -c -g -DTHRESHOLD=0.75 -Wall -DNUM_DOUBLE *.c; \
	rm -fr elina_test_oct.o; \
	cd ..        
 
test:
	cd elina_oct/tests/libFuzzer ; \
	clang -O0 -c -g -DTHRESHOLD=0.75 -DNUM_DOUBLE $(INCLUDES) test_oct.c ;\
	cd failing_tests; \
        number=$(start) ; while [ $${number} -le $(number) ] ; do \
		clang -lstdc++ -DTHRESHOLD=0.75 -g -Wall -DNUM_DOUBLE $(INCLUDES) $(OBJS) test$${number}.c /home/libFuzzer.a -o test$${number} -lmpfr -lgmp -lm; \
                startTime=`date +%s` ; \
		./test$${number}; \
                endTime=`date +%s` ; \
                runtime=`expr $$endTime - $$startTime` ; \
                echo "Execution time: $$runtime sec for test_oct$$number\n" ; \
		number=`expr $$number + 1` ; \
    	done; \
        true
