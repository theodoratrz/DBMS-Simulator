CC = gcc

LDLIBS = -lcrypto

all: HP_main HT_main

HP_main: HP_main.o HP.o
	$(CC) -o HP_main HP_main.o HP.o BF_64.a -no-pie $(LDLIBS)

HT_main: HT_main.o HT.o
	$(CC) -o HT_main HT_main.o HT.o BF_64.a -no-pie $(LDLIBS)

clean:
	rm HP_main HT_main HP_main.o HT_main.o HP.o HT.o
