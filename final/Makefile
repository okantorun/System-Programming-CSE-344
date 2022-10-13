compile: link
	gcc client.o -o client -pthread -g
	gcc server.o data-structures.o -o server -pthread -g
	gcc servant.o data-structures.o -o servant -pthread -g
link:
	gcc -c client.c -pthread  -g -Wall
	gcc -c server.c -pthread  -g -Wall
	gcc -c servant.c -pthread  -g -Wall
	gcc -c data-structures.c -pthread  -g -Wall
	
clean:
	rm *.o
	rm client
	rm server
	rm servant
	
