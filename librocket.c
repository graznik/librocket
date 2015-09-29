#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "librocket.h"

#define UP    65
#define DOWN  66
#define RIGHT 67
#define LEFT  68
#define FIRE  '\n'

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
