#
# Student makefile for Cache Lab
# Note: requires a 64-bit x86-64 system 
#
CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -m64

all: csim csim_new
	# Generate a handin tar file each time you compile
	-tar -cvf ${USER}-handin.tar  csim.c csim_new.c 

csim: csim.c cachelab.c cachelab.h
	$(CC) $(CFLAGS) -o csim csim.c cachelab.c -lm 

csim_new: csim_new.c cachelab.c cachelab.h
	$(CC) $(CFLAGS) -o csim_new csim_new.c cachelab.c -lm 

# Clean the src dirctory
#
clean:
	rm -rf *.o
	rm -f *.tar
	rm -f csim
	rm -f csim_new
	rm -f tracegen
	rm -f trace.all trace.f*
	rm -f .csim_results .marker
