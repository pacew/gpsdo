CFLAGS = -g -Wall `pkg-config --cflags libusb-1.0`
LIBS = `pkg-config --libs libusb-1.0` -lusb-1.0

all: swocat

swocat: swocat.o
	$(CC) $(CFLAGS) -o swocat swocat.o $(LIBS)

clean:
	rm -f swocat *.o
