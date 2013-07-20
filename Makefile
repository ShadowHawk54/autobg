INC=./include
LIB=./lib
SRC=./src
BIN=./bin
DOC=./doc
TEST=./test

EXEC_NAME=autobg

SOURCES=$(wildcard $(SRC)/*.c)

TEST_SOURCES=$(wildcard $(TEST)/*.c)

TARGET=$(BIN)/$(EXEC_NAME)

CFLAGS+=-std=c99 -Wall -Werror -I$(INC) -L$(LIB) -lop -O2
LD=/usr/bin/gcc
LDFLAGS+= -lc

all: prepare
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

doc:
	@mkdir -p $(DOC)
	@$(shell doxygen)

prepare:
	@mkdir -p $(BIN)

