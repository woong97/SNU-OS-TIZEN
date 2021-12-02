CC=arm-linux-gnueabi-gcc
CFLAGS=-I../include

.PHONY=clean

TARGET=gpsupdate

all: $(TARGET)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
