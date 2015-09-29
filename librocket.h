#include <libusb.h>

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

int __winbond_control(struct rocket_launcher *self, unsigned char dir);
void init_winbond(struct rocket_launcher *winb, libusb_device_handle *handle);
libusb_device *get_device(libusb_device **devs, int cnt);
