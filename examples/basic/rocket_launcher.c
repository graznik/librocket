#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "../../librocket.h"

#define UP    65
#define DOWN  66
#define RIGHT 67
#define LEFT  68
#define FIRE  '\n'

int main(int argc, char *argv[])
{
	struct rocket_launcher *rl = NULL;
	unsigned char cmd = 0, next_cmd = 0;

	initscr();
	noecho();

	rl = malloc(sizeof(struct rocket_launcher));
	init_launcher(rl);

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

	exit_launcher(rl);
	free(rl);

	return EXIT_SUCCESS;
}
