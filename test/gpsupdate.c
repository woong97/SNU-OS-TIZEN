#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/gps.h>

#define	SYSCALL_SET_GPS_LOCATION	398

void parse_input(struct gps_location *loc, char *argv[]) {
	loc->lat_integer = atoi(argv[1]);
	loc->lat_fractional = atoi(argv[2]);
	loc->lng_integer = atoi(argv[3]);
	loc->lng_fractional = atoi(argv[4]);
	loc->accuracy = atoi(argv[5]);

	printf("Your input lat_integer: %d\n", loc->lat_integer);
	printf("Your input lat_fractional: %d\n", loc->lat_fractional);
	printf("Your input lng_integer: %d\n", loc->lng_integer);
	printf("Your input lng_fractional: %d\n", loc->lng_fractional);
	printf("Your input accuracy: %d\n", loc->accuracy);
}


int main(int argc, char *argv[]) {
	struct gps_location loc;
	int retval;
	
	if (argc != 6) {
		printf("Input should be './gpsupdate <lat_integer> <lat_fractional> <lng_integer> <lng_fractional> <accuracy>\n");
		exit(EXIT_FAILURE);
	}
	parse_input(&loc, argv);

	if ((retval = syscall(SYSCALL_SET_GPS_LOCATION, &loc)) < 0) {
		printf("sys_set_gps_location failed\n");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
