test: compile
	echo "small test"
	for n in `seq 1 16`; do\
		./psort test/test-small/test-small-in.txt /nobackup/output $$n;\
		test/rcheck /nobackup/output test/test-small/test-small-out.txt;\
	done;
	echo "\n"

	echo "small2 test"
	for n in `seq 1 16`; do\
		./psort test/test-small2/test-small2-in.txt /nobackup/output $$n;\
		test/rcheck /nobackup/output test/test-small2/test-small2-out.txt;\
	done;
	echo "\n"

	echo "medium test"
	for n in `seq 1 16`; do\
		./psort test/test-medium/test-medium-in.txt /nobackup/output $$n;\
		test/rcheck /nobackup/output test/test-medium/test-medium-out.txt;\
	done;
	echo "\n"

	echo "medium2 test"
	for n in `seq 1 16`; do\
		./psort test/test-medium2/test-medium2-in.txt /nobackup/output $$n;\
		test/rcheck /nobackup/output test/test-medium2/test-medium2-out.txt;\
	done;
	echo "\n"

	echo "large test"
	for n in `seq 1 16`; do\
		./psort test/test-large/test-large.in /nobackup/output $$n;\
		test/rcheck /nobackup/output test/test-large/test-large.out;\
	done;
	echo "\n"

	echo "large2 test"
	for n in `seq 1 16`; do\
		./psort test/test-large2/test-large2.in /nobackup/output $$n;\
		test/rcheck /nobackup/output test/test-large2/test-large2.out;\
	done;
	echo "\n"
run: compile
	./psort /nobackup/test.img /nobackup/output 16
compile: psort.c
	gcc psort.c -o psort -Wall -Werror -pthread -O
