CC=gcc

CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

ALL= server client

all: $(ALL)

client : client.o utils_v1.o header.h
	$(CC) $(CCFLAGS) -o client client.o utils_v1.o
client.o : client.c
	$(CC) $(CCFLAGS) -c client.c


server : server.o utils_v1.o tuile.o socket.o header.h 
	$(CC) $(CCFLAGS) -o server server.o utils_v1.o tuile.o socket.o
server.o : server.c
	$(CC) $(CCFLAGS) -c server.c


utils_v1.o: utils_v1.c utils_v1.h
	$(CC) $(CCFLAGS) -c utils_v1.c

tuile.o: tuile.c tuile.h utils_v1.h
	$(CC) $(CCFLAGS) -c tuile.c

socket.o: socket.c socket.h utils_v1.h
	$(CC) $(CCFLAGS) -c socket.c

clean:
	rm -f *.o
	rm -f $(ALL)	