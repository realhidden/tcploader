ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC)
endif

include $(DEVKITPPC)/wii_rules

.PHONY = all

DEPSDIR		= 	.
CFLAGS		=	-g -Os -mrvl -Wall $(MACHDEP) $(INCLUDE) -I$(LIBOGC_INC)

LDFLAGS		=	-L$(LIBOGC_LIB) -logc -g $(MACHDEP) -mrvl -Wl,-Map,$(notdir $@).map,--section-start,.header=0x80a00000

LOADER_OBJS	=	main.o elf.o dol.o
LOADER_DEPENDS	=	$(LOADER_OBJS:.o=.d)

TCPLOAD_OBJS	=	loader.dol.o preloader.o dol.o
TCPLOAD_LDFLAGS	=	-L$(LIBOGC_LIB) -logc -g $(MACHDEP) -mrvl -Wl,-Map,$(notdir $@).map
TCPLOAD_DEPENDS :=	$(TCPLOAD_OBJS:.o=.d)

export LD	:=	$(CC)


all: tcpload.dol

clean:
	rm -f *.o *.d *.map *.s *.dol *.elf

tcpload.dol: tcpload.elf

loader.dol.o: loader.dol
loader.dol: loader.elf
loader.elf: $(LOADER_OBJS)

# FIXME: these aren't used in the first build..
-include $(LOADER_DEPENDS)
-include $(TCPLOAD_DEPENDS)

%.o: %.c
	$(CC) -MMD -MP -MF $*.d $(CFLAGS) -c $< -o $@

%.dol.o: %.dol
	bin2s $< > $@.s
	$(CC) -MMD -MP $(CFLAGS) -c $@.s -o $@

tcpload.elf: $(TCPLOAD_OBJS)
	$(LD)  $^ $(TCPLOAD_LDFLAGS) $(LIBPATHS) $(LIBS) -o $@

