# macro

CC	= gcc
RM	= rm -f

TARGET	= fire

# objs
OBJS	= main.o
SRC	= main.c

# rule
all:	$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -lpthread -lwiringPi -o $(TARGET)

$(OBJS): $(SRC)
	$(CC) -g -c $*.c

depend:
	makedepend $(SRC)

clean:
	$(RM) $(TARGET) $(OBJS) core

