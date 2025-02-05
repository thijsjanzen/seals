CFLAGS = -std=c++17 -O3 -ffast-math -march=native
OBJS = main.o

all: ${OBJS}
	g++ ${CFLAGS} ${OBJS} -o run_model

main.o: main.cpp
	g++ ${CFLAGS} -c main.cpp

clean:
	rm -f run_model ${OBJS}
