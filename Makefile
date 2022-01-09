# Makefile
CFLAGS= -std=c++0x -Wall
COMPILATOR = g++ $(CFLAGS) -o $@

OBJ_PATH = ./obj/
SRC_PATH = ./src/
INC_PATH = ./include/

OBJ = main.o \
	Client.o \
	ArgsParser.o \
	CommunicationBase.o

TARGET = client

HPP = ArgsParser.hpp \
	CommunicationBase.hpp \
	Client.hpp

OBJ_FILES = $(patsubst %,$(OBJ_PATH)%,$(OBJ)) 
HEADERS = $(patsubst %,$(INC_PATH)%,$(HPP)) 

all: $(OBJ_PATH)main.o $(OBJ_PATH)Client.o $(OBJ_PATH)ArgsParser.o $(OBJ_PATH)CommunicationBase.o $(TARGET) 

$(OBJ_PATH)ArgsParser.o: $(SRC_PATH)ArgsParser.cpp $(INC_PATH)ArgsParser.hpp 
	$(COMPILATOR) -c $<

$(OBJ_PATH)CommunicationBase.o: $(SRC_PATH)CommunicationBase.cpp $(INC_PATH)CommunicationBase.hpp 
	$(COMPILATOR) -c $<

$(OBJ_PATH)Client.o: $(SRC_PATH)Client.cpp $(INC_PATH)ArgsParser.hpp $(INC_PATH)CommunicationBase.hpp $(INC_PATH)Client.hpp
	$(COMPILATOR) -c $<

$(OBJ_PATH)main.o: main.cpp $(INC_PATH)ArgsParser.hpp $(INC_PATH)CommunicationBase.hpp $(INC_PATH)Client.hpp
	$(COMPILATOR) -c $<

$(TARGET): $(OBJ_PATH)main.o $(OBJ_PATH)ArgsParser.o $(OBJ_PATH)CommunicationBase.o $(OBJ_PATH)Client.o
	$(COMPILATOR) $^

clean:
	rm -f $(OBJ_FILES)