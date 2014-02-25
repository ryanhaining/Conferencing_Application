CC=gcc

.c.o:
	$(CC) -Wall -Wextra -pedantic -D_XOPEN_SOURCE=600 -std=c99 -g -c $?

# compile client and server
all: confclient confserver

# compile client only
confclient: confclient.o confutils.o
	gcc -g -D_XOPEN_SOURCE=600 -o confclient confclient.o  confutils.o

# compile server program
confserver: confserver.o confutils.o
	gcc -g -D_XOPEN_SOURCE=600 -o confserver confserver.o  confutils.o

SOURCE = confutils.c confclient.c confserver.c Readme.txt makefile
SUBMISSION=ytang4
submit: $(SOURCE)
	rm -rf $(SUBMISSION).tar.gz $(SUBMISSION)/
	mkdir $(SUBMISSION)
	cp $(SOURCE) $(SUBMISSION)
	tar -zcvf $(SUBMISSION).tar.gz $(SUBMISSION)/
