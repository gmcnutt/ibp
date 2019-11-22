sources := 
objects = $(sources:.c=.o)
CFLAGS += -Werror -Wfatal-errors
CFLAGS += -std=c99
CFLAGS += -fPIC

ifeq ($(OPTIMIZE), true)
	CFLAGS += -O2
else
	CFLAGS += -Wall -g
endif

all: demo

demo: demo.o $(objects)
	$(CC) $(CFLAGS) $< $(objects) $(LDFLAGS) -lSDL2 -lSDL2_image -o $@ -Wl,-rpath=/usr/local/lib

clean:
	rm -f *.o $(sdl2lib) $(isolib) demo
