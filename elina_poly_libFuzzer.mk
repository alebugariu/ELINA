LIBS=-lmpfr -lgmp -lelinalinearize -lelinaux -loptpoly -lpartitions

all: compile test
compile: 
	make CC=clang CFLAGS="-fsanitize-coverage=trace-pc-guard -fPIC -O0"
	sudo make install   
 
test:
	cd elina_poly/tests/libFuzzer; \
        number=$(start) ; while [ $${number} -le $(number) ] ; do \
        rm MY_CORPUS/* ; \
		clang -lstdc++ -fsanitize-coverage=trace-pc-guard -fPIC -O0 -DTHRESHOLD=0.75 -DNUM_DOUBLE \
                      -I /usr/local/include test_poly.c test_poly$${number}.c /home/libFuzzer.a -otest$${number} $(LIBS) ; \
                      startTime=`date +%s%N` ; \
 		LD_LIBRARY_PATH=/usr/local/lib/ ./test$${number} -max_len=10000 -detect_leaks=0 -rss_limit_mb=0 -max_total_time=10800 -timeout=3600 -seed=1046527 -print_final_stats=1 MY_CORPUS/ SEED_CORPUS/; \
                endTime=`date +%s%N` ; \
                runtime=$$((($$endTime - $$startTime)/1000000)) ; \
               echo "Execution time: $$runtime millisec for test_poly$$number\n" ; \
		number=`expr $$number + 1` ; \
    	done; \
        true
