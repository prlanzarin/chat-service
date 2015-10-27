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

all: client server 

#todo deps
client: $(SRC_DIR)/client.c *.o 
	$(CC) $(DEBUG) -o $(BIN_DIR)/client $(SRC_DIR)/client.c $(BIN_DIR)/*.o $(CFLAGS) 

#todo deps	
server: $(SRC_DIR)/server.c *.o 
	$(CC) $(DEBUG) -o $(BIN_DIR)/server $(SRC_DIR)/server.c $(BIN_DIR)/*.o $(CFLAGS) -pthread

usr.o: $(INC_DIR)/usr.h $(SRC_DIR)/usr.c 
	mkdir -p $(BIN_DIR) && $(CC) $(DEBUG) -c -o $(BIN_DIR)/usr.o $(SRC_DIR)/usr.c $(CFLAGS) 

list.o: $(INC_DIR)/list.h $(SRC_DIR)/list.c 
	mkdir -p $(BIN_DIR) && $(CC) $(DEBUG) -c -o $(BIN_DIR)/list.o $(SRC_DIR)/list.c $(CFLAGS) 

utils.o: $(INC_DIR)/utils.h $(SRC_DIR)/utils.c matrix.o
	mkdir -p $(BIN_DIR) && $(CC) $(DEBUG) -c -o $(BIN_DIR)/utils.o $(SRC_DIR)/utils.c $(CFLAGS) 

clean:
	rm -rf $(BIN_DIR)/*.o $(EXE) $(SRC_DIR)/*~ $(INC_DIR)/*~ *~

rebuild: clean all
