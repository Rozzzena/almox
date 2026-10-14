CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2
TARGET = almox.exe
SRCS = main.c almoxarifado.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c almoxarifado.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /f /q $(OBJS) $(TARGET) 2>nul || rm -f $(OBJS) $(TARGET)

.PHONY: all clean
