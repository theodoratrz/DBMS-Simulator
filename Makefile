CC = gcc

LDLIBS = -lcrypto

all: HP_main SHT_main

HP_main: HP_main.o HP.o
	$(CC) -o HP_main HP_main.o HP.o BF_64.a -no-pie $(LDLIBS)

SHT_main: SHT_main.o SHT.o HT.o
	$(CC) -o SHT_main SHT_main.o SHT.o HT.o BF_64.a -no-pie $(LDLIBS)

clean:
	rm HP_main SHT_main HP_main.o HP.o HT.o SHT_main.o SHT.o
