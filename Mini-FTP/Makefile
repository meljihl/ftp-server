#
# Makefile
# Xavier Delord
# 12.11.2002
#

CC = gcc
#CFLAGS = -g -v
CFLAGS = -g
LDFLAGS = -g
LDLIBS =  -lsocket -lnsl

EXE = miniftp miniftpd


all : $(EXE)

miniftp : miniftp.o common.o
	$(CC) $(LDFLAGS) miniftp.o common.o $(LOADLIBES) $(LDLIBS) -o $@
miniftpd : miniftpd.o common.o
	$(CC) $(LDFLAGS) miniftpd.o common.o $(LOADLIBES) $(LDLIBS) -o $@


miniftp.o : miniftp.c requests.h common.h
miniftpd.o : miniftpd.c requests.h common.h
common.o : common.c common.h

clean:
	-rm -f $(EXE) *.o *~
