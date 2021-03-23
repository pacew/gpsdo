CFLAGS = -g -Wall `pkg-config --cflags libusb-1.0`
LIBS = `pkg-config --libs libusb-1.0` -lusb-1.0

all: swocat swolisten

swolisten: swolisten.o
	$(CC) $(CFLAGS) -o swolisten swolisten.o $(LIBS)

swocat: swocat.o
	$(CC) $(CFLAGS) -o swocat swocat.o $(LIBS)

clean:
	rm -f swolisten *.o
