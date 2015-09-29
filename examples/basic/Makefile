INCLUDES  = $(shell pkg-config --cflags libusb-1.0)
INCLUDES += $(shell pkg-config --cflags ncurses)
LIBS  = $(shell pkg-config --libs libusb-1.0)
LIBS += $(shell pkg-config --libs ncurses)

CFLAGS  = -Wall -Werror $(INCLUDES) ../../librocket.c
LDFLAGS = $(LIBS)

.PHONY: all clean

all: rocket_launcher

clean:
	-@rm -f *~ rocket_launcher
