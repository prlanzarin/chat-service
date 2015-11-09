#
# Makefile: chat-service
#

CC=gcc
CFLAGS =-Wall # -O3
DEBUG="-g" # tratar como opção do usuário 
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
EXE =./bin/client ./bin/server 

all: server client

server: $(SRC_DIR)/server.c usr.o room.o list.o session.o message.o
	$(CC) $(DEBUG) -o $(BIN_DIR)/server $(SRC_DIR)/server.c $(BIN_DIR)/*.o $(CFLAGS) -pthread

client: $(SRC_DIR)/client.c usr.o room.o list.o session.o message.o ui.o
	$(CC) $(DEBUG) -o $(BIN_DIR)/client $(SRC_DIR)/client.c $(BIN_DIR)/*.o $(CFLAGS) -pthread -lncurses

session.o: $(INC_DIR)/session.h $(SRC_DIR)/session.c
	mkdir -p $(BIN_DIR) && $(CC) $(DEBUG) -c -o $(BIN_DIR)/session.o $(SRC_DIR)/session.c $(CFLAGS)

usr.o: $(INC_DIR)/usr.h $(SRC_DIR)/usr.c 
	mkdir -p $(BIN_DIR) && $(CC) $(DEBUG) -c -o $(BIN_DIR)/usr.o $(SRC_DIR)/usr.c $(CFLAGS) 

room.o: $(INC_DIR)/room.h $(SRC_DIR)/room.c 
	mkdir -p $(BIN_DIR) && $(CC) $(DEBUG) -c -o $(BIN_DIR)/room.o $(SRC_DIR)/room.c $(CFLAGS) 

message.o: $(INC_DIR)/message.h $(SRC_DIR)/message.c
	mkdir -p $(BIN_DIR) && $(CC) $(DEBUG) -c -o $(BIN_DIR)/message.o $(SRC_DIR)/message.c $(CFLAGS)

ui.o: $(INC_DIR)/ui.h $(SRC_DIR)/ui.c
	mkdir -p $(BIN_DIR) && $(CC) $(DEBUG) -o $(BIN_DIR)/ui.o -c $(SRC_DIR)/ui.c $(CFLAGS) -lncurses

list.o: $(INC_DIR)/list.h $(SRC_DIR)/list.c 
	mkdir -p $(BIN_DIR) && $(CC) $(DEBUG) -c -o $(BIN_DIR)/list.o $(SRC_DIR)/list.c $(CFLAGS) 

# TODO UTILS MODULE
utils.o: $(INC_DIR)/utils.h $(SRC_DIR)/utils.c
	mkdir -p $(BIN_DIR) && $(CC) $(DEBUG) -c -o $(BIN_DIR)/utils.o $(SRC_DIR)/utils.c $(CFLAGS) 

clean:
	rm -rf $(BIN_DIR)/*.o $(EXE) $(SRC_DIR)/*~ $(INC_DIR)/*~ *~

rebuild: clean all
