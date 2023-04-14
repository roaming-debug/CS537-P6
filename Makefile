run: compile
	./psort /nobackup/test.img /nobackup/output 16
compile: psort.c
	gcc psort.c -o psort -Wall -pthread  -g
