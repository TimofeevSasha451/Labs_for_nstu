CC = gcc
CFLAGS =  -fPIC
LDFLAGS_STATIC = -static
LDFLAGS_LIB = -shared
LDFLAGS_MAIN = -ldl

TARGET_DYNAMIC = main_d
LIB_DYNAMIC = libdll.so
SOURCE_LIB = dll.c
MAIN_DYNAMIC = main_d.c
HEADER = dll.h


$(TARGET_DYNAMIC): $(SOURCE_LIB) $(HEADER)
	$(CC) $(CFLAGS) $(LDFLAGS_LIB) $(SOURCE_LIB) -o $(LIB_DYNAMIC)

all: main $(TARGET_DYNAMIC)

main: main.c
	gcc main.c -o main -lrt -pthread -ldl


run: all
	./main file.txt file_1.txt 9

clean:
	rm -f *.o  $(LIB_DYNAMIC) main 

.PHONY: all clean
