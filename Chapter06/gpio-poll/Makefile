TARGET = gpio-poll
CFLAGS = -Wall -O2 -D_GNU_SOURCE

MACHINE = $(shell awk '/Hardware/ { print $$3 }' < /proc/cpuinfo)
ifeq ($(MACHINE),Atmel)
  CFLAGS += -D_ATMEL_GPIOS
endif

all: $(TARGET)

clean:
	rm -rf $(TARGET)
