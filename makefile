CC      = gcc
CFLAGS  = -g
RM      = rm -f

default: all

all: proj2

Hello: proj2.c
	$(CC) $(CFLAGS) -o proj2 proj2.c
clean veryclean:
	$(RM) proj2
