#ifndef	_LINUX_GPS_H_
#define _LINUX_GPS_H_
struct gps_location {
	int lat_integer;
	int lat_fractional;
	int lng_integer;
	int lng_fractional;
	int accuracy;
};

extern struct gps_location curr_loc;
extern struct mutex gps_lock;
#endif
