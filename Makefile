CC = g++ -std=c++14

all:
	$(CC) main.cpp -O0 -g3 -lglut -lGLU -lGL -I. -o cgprog

clean:
	rm cgprog

