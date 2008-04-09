.PHONY = all clean win32 linux

OUTPUT	=	sendelf
CC	=	gcc

OBJS	=	main.o
CFLAGS	=	-Wall -pedantic -g

WIN_OUTPUT	=	sendelf.exe
WIN_CC	=	i686-mingw32-gcc

WIN_OBJS	=	main.wo
WIN_CFLAGS	=	-Wall -pedantic -g
WIN_LDFLAGS	=	-lwsock32

all: win32 linux

win32: $(WIN_OUTPUT)

linux: $(OUTPUT)

clean:
	rm -f $(OBJS) $(WIN_OBJS) $(OUTPUT) $(WIN_OUTPUT)

$(OUTPUT): $(OBJS)
	gcc -o $@ $<

$(WIN_OUTPUT): $(WIN_OBJS)
	$(WIN_CC) -o $@ $< $(WIN_LDFLAGS)

%.wo: %.c
	$(WIN_CC) $(WIN_CFLAGS) -c -o $@ $<

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<
