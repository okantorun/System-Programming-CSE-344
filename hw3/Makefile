compile: link
	gcc hw3unnamed.o -o hw3unnamed -pthread 
	gcc hw3named.o -o hw3named -pthread 
link:
	gcc -c hw3unnamed.c -Wall
	gcc -c hw3named.c -Wall
	
clean:
	rm *.o
	rm hw3unnamed
	rm hw3named
