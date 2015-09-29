#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <libusb.h>
#include "../../librocket.h"

#define UP    65
#define DOWN  66
#define RIGHT 67
#define LEFT  68
#define FIRE  '\n'

libusb_device *get_device(libusb_device **devs, int cnt)
{
	int err, i;
	libusb_device *dev;
	struct libusb_device_descriptor dev_desc;

	for (i = 0; i < cnt; i++) {
		dev = devs[i];
		err = libusb_get_device_descriptor(dev, &dev_desc);
		if (err) {
			fprintf(stderr, "%s\n", libusb_error_name(err));
			return NULL;
		}

		if ((dev_desc.idVendor == WINBOND_VENDOR_ID) &&
		    (dev_desc.idProduct == WINBOND_DEVICE_ID))
			return dev;
		else
			fprintf(stdout, "Ignoring 0x%04x 0x%04x\n",
				dev_desc.idVendor,
				dev_desc.idProduct);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	libusb_context *ctx;
	libusb_device **devs;
	libusb_device *dev;
	libusb_device_handle *handle;
	struct rocket_launcher *rl = NULL;
	ssize_t cnt;
	uint8_t bus, addr;
	int err = 0;
	int retval;
	unsigned char cmd = 0, next_cmd = 0;

	err = libusb_init(&ctx);
	if (err) {
		fprintf(stderr, "%s\n", libusb_error_name(err));
		return EXIT_FAILURE;
	}

	cnt = libusb_get_device_list(ctx, &devs);

	dev = get_device(devs, cnt);
	if (!dev) {
		fprintf(stderr, "Error: Unable to detect device\n");
		retval = EXIT_FAILURE;
		goto err_free;
	}

	bus = libusb_get_bus_number(dev);
	addr = libusb_get_device_address(dev);

	fprintf(stdout, "Bus %03d Device %03d\n", bus, addr);

	err = libusb_open(dev, &handle);
	if (err) {
		fprintf(stderr, "%s\n", libusb_error_name(err));
		retval = EXIT_FAILURE;
		goto err_free;
	}

	err = libusb_kernel_driver_active(handle, 0);
	if (err) {
		err = libusb_detach_kernel_driver(handle, 0);
		if (err) {
			fprintf(stderr, "%s\n", libusb_error_name(err));
			retval = EXIT_FAILURE;
			goto err_close;
		}
	}

	initscr();
	noecho();

	rl = malloc(sizeof(struct rocket_launcher));
	/* FIXME: Only Winbond device supported at the moment */
	init_winbond(rl, handle);

	while (1) {
		next_cmd = (unsigned char)getch();
		switch (next_cmd) {
		case UP:
			if (rl->last_cmd == rl->cmd_down)
				cmd = rl->cmd_hold;
			else
				cmd = rl->cmd_up;
			break;
		case DOWN:
			if (rl->last_cmd == rl->cmd_up)
				cmd = rl->cmd_hold;
			else
				cmd = rl->cmd_down;
			break;
		case LEFT:
			if (rl->last_cmd == rl->cmd_right)
				cmd = rl->cmd_hold;
			else
				cmd = rl->cmd_left;
			break;
		case RIGHT:
			if (rl->last_cmd == rl->cmd_left)
				cmd = rl->cmd_hold;
			else
				cmd = rl->cmd_right;
			break;
		case FIRE:
			cmd = rl->cmd_fire;
			break;
		}
		rl->last_cmd = cmd;
		rl->control(rl, cmd);
	}

	retval = EXIT_SUCCESS;

 err_close:
	libusb_close(handle);
 err_free:
	libusb_free_device_list(devs, 1);

	libusb_exit(ctx);
	free(rl);

	return retval;
}
