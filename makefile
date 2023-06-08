CC = gcc
CFLAGS = -I.
LDFLAGS = -L. -Wl,-rpath=.

all: proactor server client

proactor: st_proactor.c st_proactor.h
	$(CC) -c -fPIC st_proactor.c -o st_proactor.o $(CFLAGS)
	$(CC) -shared -o libst_proactor.so st_proactor.o

server: selectserver.c
	$(CC) -o server selectserver.c -lst_proactor -lpthread $(CFLAGS) $(LDFLAGS)

client: client.c
	$(CC) -o client client.c $(CFLAGS)

clean:
	rm -f *.o *.so server client
