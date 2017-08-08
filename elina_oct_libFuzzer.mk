LIBS=-lmpfr -lgmp -lelinalinearize -lelinaux -loptoct -lpartitions

all: compile test
compile: 
	make CC=clang CFLAGS="-fsanitize-coverage=trace-pc-guard -fPIC -O0"
	sudo make install         
 
test:
	cd elina_oct/tests/libFuzzer; \
    number=$(start) ; while [ $${number} -le $(number) ] ; do \
		clang -lstdc++ -fsanitize=address -fsanitize-coverage=trace-pc-guard -fPIC -O0 -DTHRESHOLD=0.75 -DNUM_DOUBLE \
				-I /usr/local/include test_oct.c test_oct$${number}.c /home/libFuzzer.a -o test$${number} $(LIBS); \
        startTime=`date +%s` ; \
		./test$${number} -max_len=10000 -detect_leaks=0 -rss_limit_mb=8192 -timeout=3600 -print_final_stats=1 MY_CORPUS/ SEED_CORPUS/; \
        endTime=`date +%s` ; \
        runtime=`expr $$endTime - $$startTime` ; \
        echo "Execution time: $$runtime sec for test_oct$$number\n" ; \
		number=`expr $$number + 1` ; \
    	done; \
        true
