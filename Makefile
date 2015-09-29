INCLUDES  = $(shell pkg-config --cflags libusb-1.0)
INCLUDES += $(shell pkg-config --cflags ncurses)
LIBS  = $(shell pkg-config --libs libusb-1.0)
LIBS += $(shell pkg-config --libs ncurses)

CFLAGS  = -Wall -Werror $(INCLUDES)
LDFLAGS = $(LIBS)

.PHONY: all clean

all: rocket_launch

clean:
	-@rm -f *~ rocket_launch
