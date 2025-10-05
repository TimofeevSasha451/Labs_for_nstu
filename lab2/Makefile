CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC
LDFLAGS_STATIC = -static
LDFLAGS_LIB = -shared
LDFLAGS_MAIN = -ldl

TARGET_STATIC = main_s 
TARGET_DYNAMIC = main_d 
LIB_STATIC = libdll.a
LIB_DYNAMIC = libdll.so
SOURCE_LIB = dll.c
MAIN_STATIC = main_s.c
MAIN_DYNAMIC = main_d.c
HEADER = dll.h

$(TARGET_STATIC): $(SOURCE_LIB) $(HEADER) $(MAIN_STATIC)
	$(CC) $(CFLAGS) -c $(SOURCE_LIB) -o dll.o
	ar rcs $(LIB_STATIC) dll.o
	$(CC) $(CFLAGS) -c $(MAIN_STATIC) -o main1.o
	$(CC) main1.o $(LIB_STATIC) $(LDFLAGS_STATIC) -o $(TARGET_STATIC)

$(TARGET_DYNAMIC): $(SOURCE_LIB) $(HEADER) $(MAIN_DYNAMIC)
	$(CC) $(CFLAGS) $(LDFLAGS_LIB) $(SOURCE_LIB) -o $(LIB_DYNAMIC)
	$(CC) $(CFLAGS) $(MAIN_DYNAMIC) $(LDFLAGS_MAIN) -o $(TARGET_DYNAMIC)

all: $(TARGET_STATIC) $(TARGET_DYNAMIC)

run: all
	./$(TARGET_STATIC) file.txt 9
	./$(TARGET_DYNAMIC) file.txt 9

# Очистка
clean:
	rm -f *.o $(LIB_STATIC) $(LIB_DYNAMIC) $(TARGET_STATIC) $(TARGET_DYNAMIC)

.PHONY: all clean
