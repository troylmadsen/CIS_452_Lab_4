filename = server
objects = server.o

server: $(objects)
	clang -o ${filename} -Wall -lpthread $(objects)

server.o: server.c
	clang -c -o server.o -Wall server.c

.PHONY: clean run debug
clean:
	-rm $(filename) $(objects)

run: $(filename)
	./$(filename)

debug:
	clang -c -o server.o -Wall -g -lpthread server.c
	clang -o $(filename) -Wall -g server.o
