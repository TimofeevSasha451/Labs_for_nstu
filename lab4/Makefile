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


$(TARGET_DYNAMIC): $(SOURCE_LIB) $(HEADER) $(MAIN_DYNAMIC)
	$(CC) $(CFLAGS) $(LDFLAGS_LIB) $(SOURCE_LIB) -o $(LIB_DYNAMIC)
	$(CC) $(CFLAGS) $(MAIN_DYNAMIC) $(LDFLAGS_MAIN) -o $(TARGET_DYNAMIC)

parent:
	gcc parent.c -o parent

all: parent $(TARGET_DYNAMIC)

run: all
	./parent file.txt 9

clean:
	rm -f *.o  $(LIB_DYNAMIC)  $(TARGET_DYNAMIC) parent

.PHONY: all clean
