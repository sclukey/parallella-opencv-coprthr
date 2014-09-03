CC=gcc

all: sobel_parallel sobel_series

sobel_series:
	$(CC) -o host/bin/sobel_series host/src/sobel_series.c `pkg-config --cflags --libs coprthr opencv`
	cp host/bin/sobel_series bin/sobel_series

sobel_parallel: device/bin/sobel_kernel.o
	$(CC) -o host/bin/sobel_parallel host/src/sobel_parallel.c device/bin/sobel_kernel.o `pkg-config --cflags --libs coprthr opencv`
	cp host/bin/sobel_parallel bin/sobel_parallel

device/bin/sobel_kernel.o:
	clcc -o device/bin/sobel_kernel.o -c device/src/sobel_kernel.c

clean:
	rm -f device/bin/* host/bin/* bin/*
