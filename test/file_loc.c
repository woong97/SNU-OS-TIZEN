#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/gps.h>

#define SYSCALL_GET_GPS_LOCATION	399

int main(int argc, char *argv[]) {
	int retval;
	struct gps_location loc;
	if (argc != 2) {
		printf("Input shuld be ./file_loc <filepath>\n");
		exit(EXIT_FAILURE);
	}

	if ((retval = syscall(SYSCALL_GET_GPS_LOCATION, argv[1], &loc)) < 0) {
		printf("sys_set_gps_location failed\n");
		exit(EXIT_FAILURE);
	}
	
	printf("File %s location\n", argv[1]);
	printf("=> latitude : %d.%06d\n", loc.lat_integer, loc.lat_fractional);
	printf("=> longtitude : %d.%06d\n", loc.lng_integer, loc.lng_fractional);
	printf("-> accuracy : %d\n\n", loc.accuracy);

	printf("Google Maps Link: https://www.google.co.kr/maps/search/%d.%06d+%d.%06d\n",
			loc.lat_integer, loc.lat_fractional, loc.lng_integer, loc.lng_fractional);

	return EXIT_SUCCESS;
}
