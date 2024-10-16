# I am a comment, and I want to say that the variable CC will be
# the compiler to use.
CC=gcc
# Hey!, I am comment number 2. I want to say that CFLAGS will be the
# options I'll pass to the compiler.
CFLAGS=-c -g
LIBS=-pthread

all: myport portmaster monitor vessel

myport: myport.o
	$(CC) myport.o -o myport $(LIBS)
portmaster: portmaster.o
	$(CC) portmaster.o -o portmaster $(LIBS)
monitor: monitor.o
	$(CC) monitor.o -o monitor $(LIBS)
vessel: vessel.o
	$(CC) vessel.o -o vessel $(LIBS)
 
myport.o: myport.c
	$(CC) $(CFLAGS) myport.c $(LIBS)

portmaster.o: portmaster.c
	$(CC) $(CFLAGS) portmaster.c $(LIBS)

monitor.o: monitor.c
	$(CC) $(CFLAGS) monitor.c $(LIBS)

vessel.o: vessel.c
	$(CC) $(CFLAGS) vessel.c $(LIBS)

clean:
	rm  myport*
	rm  portmaster*
	rm  monitor*
	rm  vessel*

