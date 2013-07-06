IDIR = /opt/local/include
LDIR = /opt/local/lib
CFLAGS = -O7 -I$(IDIR)
CC = gcc
LIBOPTS =
LIBS = -L$(LDIR) -lgdal -lm

default: all


r_fdim_boxcount:main.o boxcount.o
	$(CC) $(CFLAGS) $(LIBOPTS) -o r.fdim.boxcount main.o boxcount.o $(LIBS)

boxcount.o:boxcount.c Makefile
	$(CC) $(CFLAGS) -c boxcount.c

main.o:main.c Makefile
	$(CC) $(CFLAGS) -c main.c

all: r_fdim_boxcount
