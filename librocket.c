#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "librocket.h"

int init_launcher(struct rocket_launcher *rl)
{
	int err = 0;
	ssize_t cnt;

	err = libusb_init(&(rl->ctx));
	if (err) {
		fprintf(stderr, "%s\n", libusb_error_name(err));
		return -1;
	}

	cnt = libusb_get_device_list(rl->ctx, &(rl->devs));

	err = __init_usb_device(rl, cnt);
	if (err) {
		fprintf(stderr, "Error: Unable to detect device\n");
		goto err_free;
	}

	err = libusb_open(rl->dev, &(rl->handle));
	if (err) {
		fprintf(stderr, "%s\n", libusb_error_name(err));
		goto err_free;
	}

	err = libusb_kernel_driver_active(rl->handle, 0);
	if (err) {
		err = libusb_detach_kernel_driver(rl->handle, 0);
		if (err) {
			fprintf(stderr, "%s\n", libusb_error_name(err));
			goto err_close;
		}
	}

	return 0;

 err_close:
	libusb_close(rl->handle);
 err_free:
	libusb_free_device_list(rl->devs, 1);

	libusb_exit(rl->ctx);

	return -1;
}

void exit_launcher(struct rocket_launcher *rl)
{
	libusb_close(rl->handle);
	libusb_free_device_list(rl->devs, 1);
	libusb_exit(rl->ctx);
}

int __init_usb_device(struct rocket_launcher *rl, ssize_t cnt)
{
	int err, i;
	libusb_device *dev;
	struct libusb_device_descriptor dev_desc;

	for (i = 0; i < cnt; i++) {
		dev = rl->devs[i];
		err = libusb_get_device_descriptor(dev, &dev_desc);
		if (err) {
			fprintf(stderr, "%s\n", libusb_error_name(err));
			return -1;
		}

		if ((dev_desc.idVendor == WINBOND_VENDOR_ID) &&
		    (dev_desc.idProduct == WINBOND_DEVICE_ID)) {
			fprintf(stdout, "Found Winbond PE-5858-919\n");
			__init_winbond(rl, dev);
			return 0;
		} else
			fprintf(stdout, "Ignoring 0x%04x 0x%04x\n",
				dev_desc.idVendor,
				dev_desc.idProduct);
	}
	return -1;
}

void __init_winbond(struct rocket_launcher *winb, libusb_device *dev)
{
	winb->last_cmd = (unsigned char)WINBOND_HOLD;
	winb->cmd_hold = (unsigned char)WINBOND_HOLD;
	winb->cmd_up = (unsigned char)WINBOND_UP;
	winb->cmd_down = (unsigned char)WINBOND_DOWN;
	winb->cmd_left = (unsigned char)WINBOND_LEFT;
	winb->cmd_right = (unsigned char)WINBOND_RIGHT;
	winb->cmd_fire = (unsigned char)WINBOND_FIRE;
	winb->control = &(__winbond_control);
	winb->dev = dev;
}

int __winbond_control(struct rocket_launcher *self, unsigned char dir)
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
