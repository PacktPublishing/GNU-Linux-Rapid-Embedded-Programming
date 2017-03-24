TARGET = usb_sendrecv

CFLAGS := -Wall -O2 -D_GNU_SOURCE $(shell pkg-config libusb-1.0 --cflags)
LDLIBS := $(shell pkg-config libusb-1.0 --libs)

all: $(TARGET)

clean:
	rm -rf $(TARGET)

phony: all clean
