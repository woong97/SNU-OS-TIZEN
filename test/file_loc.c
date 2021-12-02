#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/gps.h>

#define SYSCALL_GET_GPS_LOCATION	399

int main(int argc, char *argv[]) {
	int retval;
	struct gps_location loc;

	// TODO
	char *path = "testpath";
	if ((retval = syscall(SYSCALL_GET_GPS_LOCATION, path, &loc)) < 0) {
		printf("sys_set_gps_location failed\n");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
