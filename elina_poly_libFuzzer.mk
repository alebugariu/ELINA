INCLUDES_FOR_POLY=-I /home/elina/elina_auxiliary -I /home/elina/elina_linearize -I /home/elina/partitions_api
INCLUDES=-I /home/elina/elina_poly $(INCLUDES_FOR_POLY)
OBJS=*.o ../../*.o ../../../elina_auxiliary/*.o /home/elina/elina_linearize/*.o /home/elina/partitions_api/*.o
all: compile test
compile: 
	cd elina_auxiliary ; \
	clang -fsanitize-coverage=trace-pc-guard -DTHRESHOLD=0.75 -DNUM_DOUBLE -O0 -c -g *.c; \
	cd ../elina_linearize ; \
	clang -fsanitize-coverage=trace-pc-guard -DTHRESHOLD=0.75 -DNUM_DOUBLE -I ../elina_auxiliary -O0 -c -g *.c; \
	cd ../partitions_api ; \
	clang -fsanitize-coverage=trace-pc-guard -DTHRESHOLD=0.75 -DNUM_DOUBLE -O0 -c -g *.c; \
	cd ../elina_poly ; \
	clang -fsanitize-coverage=trace-pc-guard -DTHRESHOLD=0.75 -DNUM_DOUBLE $(INCLUDES_FOR_POLY) -O0 -c -g *.c; \
	rm -fr elina_test_poly.o; \
	cd ..        
 
test:
	cd elina_poly/tests/libFuzzer; \
	clang -fsanitize-coverage=trace-pc-guard -DTHRESHOLD=0.75 -DNUM_DOUBLE $(INCLUDES) -O0 -c -g test_poly.c; \
        number=$(start) ; while [ $${number} -le $(number) ] ; do \
		clang -lstdc++ -fsanitize=address -fsanitize-coverage=trace-pc-guard -DTHRESHOLD=0.75 -DNUM_DOUBLE -I /usr/local/include $(INCLUDES) $(OBJS) test_poly$${number}.c /home/libFuzzer.a -o test$${number} -lmpfr -lgmp; \
                startTime=`date +%s` ; \
		./test$${number} -max_len=10000 -detect_leaks=0 -rss_limit_mb=8192 -timeout=3600 -print_final_stats=1 MY_CORPUS/ SEED_CORPUS/; \
                endTime=`date +%s` ; \
                runtime=`expr $$endTime - $$startTime` ; \
                echo "Execution time: $$runtime sec for test_poly$$number\n" ; \
		number=`expr $$number + 1` ; \
    	done; \
        true
