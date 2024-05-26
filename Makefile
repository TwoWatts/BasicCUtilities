CC=gcc
AR=ar

INC=-I. \
    -I./data_structures/header_only \
    -I./data_structures/libraries
CFLAGS= $(INC) -L. -g

SRC_DIR=./data_structures/libraries
OBJ_DIR=obj

OBJ= \
$(OBJ_DIR)/cqueue.o \
$(OBJ_DIR)/stack.o

all: build

build: build_library build_header
	@$(CC) $(CFLAGS) main.c -lbcutils

build_library: $(OBJ)
	@$(AR) rcs libbcutils.a $(OBJ) 

build_header:
	@echo "#ifndef BCUTILS_H" >  bcutils.h
	@echo "#define BCUTILS_H" >> bcutils.h
	@echo "" >> bcutils.h

	@echo "" >> bcutils.h
	@echo "#endif /* BCUTILS_H */" >> bcutils.h

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

# Rule: Remove All Build Artifacts
clean:
	@rm -f ./run
	@rm -rf ./obj/*
	@rm -f ./core_*