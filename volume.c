#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <poll.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "wiringPi.h"

#define PIN_VOLUME_HI		0
#define PIN_VOLUME_LO		1

#define PIN_PROG_HI			3
#define PIN_PROG_LO			4

#define MPC_OUTPUT_FIFO		"/tmp/mpc_output_fifo"
#define MPC_CONTROL_TIMEOUT	5000

#define NSTATES	3
const int states[4] = {0, 1, -1, 2};

int clamp(int v, int minv, int maxv)
{
	return (v < minv ? minv : (v > maxv ? maxv : v));
}

int read_inc_encoder(const int *pins, int *pprev_state, int (*proc)(int))
{
	int code = digitalRead(pins[0])*2 + digitalRead(pins[1]);
	int state = states[code];
	int prev_state = *pprev_state;

	if(state != prev_state){
		if(state < 0)printf("woa! never seen this code (%d) before\n", code);

		if(state == (prev_state + 1) % NSTATES)
			proc(1);
		else if(state == (NSTATES + prev_state - 1) % NSTATES)
			proc(-1);
		else printf("missed something (%d -> %d\n", prev_state, state);
		*pprev_state = state;
	}
	return 0;
}

int volume = 0;
int prog_counter = 0;
int prog = 0;
struct mpd_connection *conn = 0;

int volume_changed(int d)
{
	int newvol = clamp(volume + d, 0, 100);
	if(newvol != volume){
		char command[256];
		volume = newvol;
		snprintf(command, sizeof(command), "mpc volume %d >%s", volume, MPC_OUTPUT_FIFO);
		system(command);
	}
	return 0;
}

int prog_changed(int d)
{
	int next_prog;

	prog_counter += d;
	next_prog = prog_counter / 10;

	if(next_prog < prog){
		system("mpc prev >" MPC_OUTPUT_FIFO);
	}
	else if(next_prog > prog){
		system("mpc next >" MPC_OUTPUT_FIFO);
	}
	else return 0;

	prog = next_prog;
	return 0;
}

int parse_mpc_output(const char *buf)
{
	return 0;
}

int mpc_control()
{
	int pid;

	pid = fork();
	if(pid == 0){
		char buf[1024];
		int bytesread;
		int fd;
		int ret;
		struct pollfd fds;

		ret = mkfifo(MPC_OUTPUT_FIFO, 0666);
		if(ret < 0 && errno != EEXIST){
			error(0, errno, "mpd_control() fail to create fifo\n");
			return -2;
		}
		fd = open(MPC_OUTPUT_FIFO, O_RDONLY);
		if(fd < 0){
			error(0, errno, "mpd_control() fail to open fifo\n");
			return -3;
		}

		do{
			fds.fd = fd;
			fds.events = POLLIN;
			fds.revents = 0;
			ret = poll(&fds, 1, -1);
			if(ret < 1){
				error(0, errno, "mpd_control() fail to poll\n");
				break;
			}
			bytesread = read(fd, buf, sizeof(buf));
			buf[bytesread] = 0;
			if(bytesread > 0){
				printf("mpd_control() %d bytes read\n", bytesread);
				parse_mpc_output(buf);
				puts(buf);
			}
			else usleep(1000);
		}while(1);
		close(fd);
	}
	else if(pid < 0){
		error(0, errno, "mpd_control() fail to create mpc process\n");
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int vol_pins[2] = {PIN_VOLUME_HI, PIN_VOLUME_LO};
	int prog_pins[2] = {PIN_PROG_HI, PIN_PROG_LO};
	const struct timespec st = {0, 100000};
	struct timespec st_rem;

	int vol_prev_state = -1;
	int prog_prev_state = -1;

	if(mpc_control()){
		return -1;
	}

	wiringPiSetup();
	pullUpDnControl(vol_pins[0], PUD_DOWN);
	pullUpDnControl(vol_pins[1], PUD_DOWN);
	pullUpDnControl(prog_pins[0], PUD_DOWN);
	pullUpDnControl(prog_pins[1], PUD_DOWN);
	pinMode(vol_pins[0], INPUT);
	pinMode(vol_pins[1], INPUT);
	pinMode(prog_pins[0], INPUT);
	pinMode(prog_pins[1], INPUT);

	do{
		read_inc_encoder(vol_pins, &vol_prev_state, volume_changed);
		read_inc_encoder(prog_pins, &prog_prev_state, prog_changed);

		nanosleep(&st, &st_rem);
	}while(1);

	return 0;
}
