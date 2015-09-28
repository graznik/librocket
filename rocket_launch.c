/* gcc -Wall -Werror -I/usr/include/libusb-1.0/ rocket_launch.c \
   -o rocket_launch -usb-1.0 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libusb.h>
#include <ncurses.h>

#define UP    65
#define DOWN  66
#define RIGHT 67
#define LEFT  68
#define FIRE  '\n'

/* Winbond PE-5858-919 */
#define WINBOND_VENDOR_ID 0x0416  /* Winbond */
#define WINBOND_DEVICE_ID 0x9391  /* PE-5858-919 */
#define WINBOND_HOLD       0x60
#define WINBOND_DOWN       0x01
#define WINBOND_UP         0x02
#define WINBOND_RIGHT      0x04
#define WINBOND_LEFT       0x08
#define WINBOND_FIRE       0x1f
#define WINBOND_LOWER_END  0xe1

struct rocket_launcher {
	unsigned char last_cmd;
	/* Direction commands (host -> device) */
	unsigned char cmd_hold;
	unsigned char cmd_up;
	unsigned char cmd_down;
	unsigned char cmd_left;
	unsigned char cmd_right;
	unsigned char cmd_fire;
	int (*control)(struct rocket_launcher *self, unsigned char dir);
	/* USB device handle */
	libusb_device_handle *handle;
};

static int __winbond_control(struct rocket_launcher *self, unsigned char dir)
{
	int cnt;
	unsigned char data[] = {0x5f, dir, 0xe0, 0xff, 0xfe};

	cnt = libusb_control_transfer(self->handle,
				      (uint8_t)0x21,
				      (uint8_t)0x09,
				      (uint16_t)0x0300,
				      (uint16_t)0x00,
				      data,
				      (uint16_t)0x05,
				      (unsigned int)0);
	if (cnt < 0) {
		fprintf(stderr, "%s\n", libusb_error_name(cnt));
		return -1;
	}

	return 0;
}

void init_winbond(struct rocket_launcher *winb, libusb_device_handle *handle)
{
	winb->last_cmd = (unsigned char)WINBOND_HOLD;
	winb->cmd_hold = (unsigned char)WINBOND_HOLD;
	winb->cmd_up = (unsigned char)WINBOND_UP;
	winb->cmd_down = (unsigned char)WINBOND_DOWN;
	winb->cmd_left = (unsigned char)WINBOND_LEFT;
	winb->cmd_right = (unsigned char)WINBOND_RIGHT;
	winb->cmd_fire = (unsigned char)WINBOND_FIRE;

	winb->control = &(__winbond_control);
	winb->handle = handle;
}

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
