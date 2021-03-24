#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <libusb.h>
#include <stdint.h>
#include <limits.h>
#include <termios.h>
#include <signal.h>
#include <ctype.h>

#define VID       (0x1d50)
#define PID       (0x6018)
#define INTERFACE (5)
#define ENDPOINT  (0x85)

#define TRANSFER_SIZE (64)

int vflag;

void
dump (void *buf, int n)
{
	int i;
	int j;
	int c;

	for (i = 0; i < n; i += 16) {
		printf ("%04x: ", i);
		for (j = 0; j < 16; j++) {
			if (i+j < n)
				printf ("%02x ", ((unsigned char *)buf)[i+j]);
			else
				printf ("   ");
		}
		printf ("  ");
		for (j = 0; j < 16; j++) {
			c = ((unsigned char *)buf)[i+j] & 0x7f;
			if (i+j >= n)
				putchar (' ');
			else if (c < ' ' || c == 0x7f)
				putchar ('.');
			else
				putchar (c);
		}
		printf ("\n");

	}
}


libusb_device_handle *handle;
libusb_device *dev;

int state;


int
do_connect (void)
{
	if (handle)
		libusb_close(handle);
	if ((handle = libusb_open_device_with_vid_pid(NULL,VID,PID)) == NULL){
		printf ("libusbopen failed ... need to be root?\n");
		return (-1);
	}

	if ((dev = libusb_get_device(handle)) == NULL) {
		printf ("get_device failed\n");
		return (-1);
	}

	if (libusb_claim_interface (handle, INTERFACE) < 0) {
		printf ("claim failed\n");
		return (-1);
	}

	return (0);
}

int want_message;

void
process_swit (int chan, uint32_t val)
{
	printf ("%d; 0x%02x\n", chan, val);
}


/*

https://developer.arm.com/documentation/ddi0314/h/instrumentation-trace-macrocell/about-the-instrumentation-trace-macrocell/trace-packet-format
*/

int lastc;

void
soak (void)
{
	unsigned char rbuf[TRANSFER_SIZE];
	int size;
	int rc;
	int connection_ok = 0;
	int i;
	int c;

	while ((rc = libusb_bulk_transfer(handle, 
					  ENDPOINT, 
					  rbuf, 
					  TRANSFER_SIZE, 
					  &size, 
					  1000)) == 0) {
		if (size > 0) {
			connection_ok = 1;
			want_message = 1;

			if (vflag)
				dump (rbuf, size);

			for (i = 0; i < size; i++) {
				c = rbuf[i] & 0xff;
				if (lastc == 1) {
					if ((' ' <= c && c <= '~')
					    || isspace (c)) {
						putchar (c);
					} else {
						printf ("[%02x]", c);
					}
					fflush (stdout);
				}
				lastc = c;
			}
		}
	}

	if (connection_ok)
		printf ("libusb err %s\n", libusb_error_name(rc));
}

int
main (int argc, char **argv)
{
	int c;

	while ((c = getopt (argc, argv, "v")) != EOF) {
		switch (c) {
		case 'v':
			vflag = 1;
			break;
		}
	}

	if (libusb_init (NULL) < 0) {
		fprintf (stderr, "libusb_init error\n");
		exit (1);
	}

	want_message = 1;
	while (1) {
		if (want_message) {
			printf ("connecting...\n");
			want_message = 0;
		}
		if (do_connect () < 0) {
			usleep (100 * 1000);
			continue;
		}
			
		soak ();
	}
}
