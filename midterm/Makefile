compile: link
	gcc client.o -o client -lm 
	gcc serverY.o -o serverY -lm -pthread 
	gcc serverZ.o -o serverZ -lm -pthread -lrt 
link:
	gcc -c client.c 
	gcc -c serverY.c 
	gcc -c serverZ.c 
clean:
	rm *.o
	rm client
	rm serverY
	rm serverZ
