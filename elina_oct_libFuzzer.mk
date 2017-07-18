INCLUDES=-I ../.. -I ../../../elina_auxiliary -I ../../../elina_linearize -I ../../../partitions_api
OBJS=*.o ../../*.o ../../../elina_auxiliary/*.o ../../../elina_linearize/*.o ../../../partitions_api/*.o
all: compile test
compile: 
	cd elina_auxiliary ; \
	clang -fsanitize=address -fsanitize-coverage=trace-pc-guard -O0 -c -g *.c; \
	cd ../elina_linearize ; \
	clang -fsanitize=address -fsanitize-coverage=trace-pc-guard -I ../elina_auxiliary -O0 -c -g *.c; \
	cd ../partitions_api ; \
	clang -fsanitize=address -fsanitize-coverage=trace-pc-guard -O0 -c -g *.c; \
	cd ../elina_oct ; \
	clang -fsanitize=address -fsanitize-coverage=trace-pc-guard -I ../elina_auxiliary -I ../elina_linearize -I ../partitions_api -O0 -c -g *.c; \
	rm -fr elina_test_oct.o; \
	cd ..        
 
test:
	cd elina_oct/tests/libFuzzer; \
	clang -fsanitize=address -fsanitize-coverage=trace-pc-guard $(INCLUDES) -O0 -c -g test_oct.c; \
        number=$(start) ; while [ $${number} -le $(number) ] ; do \
		clang -lstdc++ -fsanitize=address -fsanitize-coverage=trace-pc-guard -I /usr/local/include $(INCLUDES) $(OBJS) test_oct$${number}.c /home/libFuzzer.a -o test$${number} -lmpfr -lgmp; \
                startTime=`date +%s` ; \
		#./test$${number} -dict=/home/elina/dictionary.txt -max_len=10000 -detect_leaks=0 -rss_limit_mb=2000000000 -timeout=900000000000000 -print_final_stats=1; \
                endTime=`date +%s` ; \
                runtime=`expr $$endTime - $$startTime` ; \
                echo "Execution time: $$runtime sec for test_oct$$number\n" ; \
		number=`expr $$number + 1` ; \
    	done; \
        true
