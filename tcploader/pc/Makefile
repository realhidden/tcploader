.PHONY = all clean win32 linux

OUTPUT	=	sendelf
CC	=	gcc

OBJS	=	main.o
CFLAGS	=	-Wall -pedantic -g

all: $(OUTPUT)

clean:
	rm -f $(OBJS) $(WIN_OBJS) $(OUTPUT) $(WIN_OUTPUT)

$(OUTPUT): $(OBJS)
	gcc -o $@ $<

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<
