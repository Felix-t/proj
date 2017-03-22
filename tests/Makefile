CC = gcc
OBJS = ads1256_test.o

ads1256_test: $(OBJS)
	$(CC) ads1256_test.c -o ads1256_test -lbcm2835

ads1256_test.o: ads1256_test.c ads1256_test.h

clean: 
	$(RM) ads1256_test *.o
