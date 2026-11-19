EE_BIN = MOVTEST.ELF
EE_OBJS = main.o iso.o
EE_LIBS = -ldebug -lcdvd
EE_CFLAGS = -O2 -G0
all: $(EE_BIN)
clean:
	rm -f $(EE_BIN) $(EE_OBJS)
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
