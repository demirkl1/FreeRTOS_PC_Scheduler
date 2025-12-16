CC = gcc
CFLAGS = -I. -I./src -I./FreeRTOS/include -I./FreeRTOS/portable/ThirdParty/GCC/Posix \
         -DprojCOVERAGE_TEST=0 -D_WINDOWS_ -Wall -pthread -g

# Kaynak Dosyalar
SRC = src/main.c \
      src/scheduler.c \
      src/tasks.c \
      FreeRTOS/croutine.c \
      FreeRTOS/event_groups.c \
      FreeRTOS/list.c \
      FreeRTOS/queue.c \
      FreeRTOS/stream_buffer.c \
      FreeRTOS/tasks.c \
      FreeRTOS/timers.c \
      FreeRTOS/portable/MemMang/heap_3.c \
      FreeRTOS/portable/ThirdParty/GCC/Posix/port.c \
      FreeRTOS/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c

OBJ = $(SRC:.c=.o)
TARGET = freertos_sim

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)