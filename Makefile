CFLAGS=-Wall -Werror -I/usr/include/libusb-1.0/ -lusb-1.0 -lncurses

all: rocket_launch

clean:
	-@rm *~ rocket_launch 2>/dev/null || true
