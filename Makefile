P = ibp
OBJECTS = demo.o
CFLAGS = `pkg-config --cflags sdl2 SDL2_image` -Wall -g -std=c99
CFLAGS += -Werror -Wfatal-errors
CFLAGS += -fPIC
LDLIBS = `pkg-config --libs sdl2 SDL2_image`

ifeq ($(OPTIMIZE), true)
	CFLAGS += -O3
endif

$(P): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -f $(P) $(OBJECTS)
