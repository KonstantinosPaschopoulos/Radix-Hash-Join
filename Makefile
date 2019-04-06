OBJS 	= join_main.o functions.o buffer.o
SOURCE	= join_main.c functions.c buffer.c
HEADER  = functions.h types.h buffer.h
OUT  	= ../project
CC	= gcc
FLAGS   = -c -Wall

all: $(OBJS)
	$(CC) $(OBJS) -o $(OUT)

join_main.o: join_main.c
	$(CC) $(FLAGS) join_main.c

functions.o: functions.c
	$(CC) $(FLAGS) functions.c

buffer.o: buffer.c
	$(CC) $(FLAGS) buffer.c

clean:
	rm -f $(OBJS) $(OUT)
