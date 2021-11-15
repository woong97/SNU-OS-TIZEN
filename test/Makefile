CC=arm-linux-gnueabi-gcc
CFLAGS=-I../include

.PHONY=clean

TARGET=rotd selector trial
all: $(TARGET)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
