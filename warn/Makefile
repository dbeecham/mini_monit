main: main.o
	gcc -o main main.o -lnats -lrt -g -O0

main.o: main.c
	gcc -c -o main.o main.c -g -O0

main.c: main.rl
	ragel -G2 main.rl -o main.c
