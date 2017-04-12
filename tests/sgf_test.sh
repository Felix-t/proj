gcc -c -o obj/sigfox.o src/sigfox.c -Iinc -Wall -pthread
gcc -c -o tests/obj/test.o tests/sgf_test.c -Iinc
gcc -o test tests/obj/test.o obj/sigfox.o -pthread
