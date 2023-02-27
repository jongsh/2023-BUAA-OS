work: ./code/main.c ./code/fibo.c ./include/fibo.h
	gcc -I ./include/ -c ./code/fibo.c
	gcc -I ./include/ -c ./code/main.c
	gcc main.o fibo.o -o fibo
	mv main.o ./code
	mv fibo.o ./code

clean: ./code
	
	rm ./code/main.o
	rm ./code/fibo.o
