CC = gcc-10
CFLAGS = -pedantic -Wall -Wextra -Wshadow -Wdouble-promotion -Wundef -Wconversion -Wunused-parameter \
         -Wcast-align -Wcast-qual -Winit-self -Wpointer-arith -Wuninitialized -Wmissing-prototypes -g -o
EXECSERVER = mainServer
RM = rm -fv

all: $(EXECSERVER)

$(EXECSERVER): serveur.o mainServeur.c
	$(CC) $(CFLAGS) $@ $^

serveur.o: serveur.c
	$(CC) -c $(CFLAGS) $@ $<

clean:
	$(RM) *.o $(EXECSERVER)