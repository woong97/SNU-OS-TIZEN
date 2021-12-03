CC=arm-linux-gnueabi-gcc
CFLAGS=-I../include

.PHONY=clean

TARGET=gpsupdate file_loc

all: $(TARGET)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
