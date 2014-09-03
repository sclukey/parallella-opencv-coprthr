CC=gcc

all: sobel_parallel sobel_series

sobel_series:
	$(CC) -o host/bin/sobel_series host/src/sobel_series.c `pkg-config --cflags --libs coprthr opencv`
	cp host/bin/sobel_series bin/sobel_series

sobel_parallel: device/bin/matvecmult.o
	$(CC) -o host/bin/sobel_parallel host/src/sobel_parallel.c device/bin/matvecmult.o `pkg-config --cflags --libs coprthr opencv`
	cp host/bin/sobel_parallel bin/sobel_parallel

device/bin/matvecmult.o:
	clcc -o device/bin/matvecmult.o -c device/src/matvecmult.cl

clean:
	rm -f device/bin/* host/bin/* bin/*
