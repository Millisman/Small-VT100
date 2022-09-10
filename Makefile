CC=gcc
CFLAGS="-Wall"

debug:clean
	$(CC) $(CFLAGS) -g -o vt100_test main.c vt100_print.c
stable:clean
	$(CC) $(CFLAGS) -o vt100_test main.c vt100_print.c
clean:
	rm -vfr *~ vt100_test

