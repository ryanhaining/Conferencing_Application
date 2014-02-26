CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g -D_XOPEN_SOURCE=600

SERVER = confserver
CLIENT = confclient
UTILS = confutils

SUBMISSION = ytang4
SOURCE = $(SERVER).c $(CLIENT).c $(UTILS).c makefile README

# compile client and server
all: $(CLIENT) $(SERVER) 

# compile client only
$(CLIENT): $(CLIENT).o $(UTILS).o

# compile server program
$(SERVER): $(SERVER).o $(UTILS).o

submit: $(SOURCE)
	rm -rf $(SUBMISSION).tar.gz $(SUBMISSION)/
	mkdir $(SUBMISSION)
	cp $(SOURCE) $(SUBMISSION)
	tar -zcvf $(SUBMISSION).tar.gz $(SUBMISSION)/

clean:
	rm -f *.o $(CLIENT) $(SERVER) $(SUBMISSION).tar.gz
	rm -rf $(SUBMISSION)/
