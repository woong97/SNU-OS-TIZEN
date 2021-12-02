#include <linux/gps.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
/*
struct gps_location {
	int lat_integer;
	int lat_fractional;
	int lng_integer;
	int lng_fractional;
	int accuracy;
};
*/
#define MAX_FRAC	999999
#define DECIMAL_MUL	(MAX_FRAC + 1)

DEFINE_MUTEX(gps_lock);

struct gps_location curr_loc = {
	.lat_integer = 0,
	.lat_fractional = 0,
	.lng_integer = 0,
	.lng_fractional = 0,
	.accuracy = 0
};

void print_curr_loc(void)
{
	printk("====== curr_loc fields =====\n");
	printk("* lat_integer\t= %d\n", curr_loc.lat_integer);
	printk("* lat_fractional\t= %d\n", curr_loc.lat_fractional);
	printk("* lng_integer\t= %d\n", curr_loc.lng_integer);
	printk("* lng_fractional\t= %d\n", curr_loc.lng_fractional);
	printk("* accuracy\t= %d\n", curr_loc.accuracy);
	printk("============================\n");
}

static int is_valid_input(struct gps_location *loc)
{
	int lat_integer = loc->lat_integer;
	int lat_fractional = loc->lat_fractional;
	int lng_integer = loc->lng_integer;
	int lng_fractional = loc->lng_fractional;
	int accuracy = loc->accuracy;
	long latitude = lat_integer * DECIMAL_MUL + lat_fractional; 
	long longtitude = lng_integer * DECIMAL_MUL + lng_fractional;
	if (lat_fractional < 0 || lng_fractional < 0 || lat_fractional > MAX_FRAC || lng_fractional > MAX_FRAC)
		return -1;
	printk("==decimal: %d\n", DECIMAL_MUL);

	if ((latitude < -90 * DECIMAL_MUL) || (latitude > 90 * DECIMAL_MUL))
		return -1;
	if ((longtitude < -180 * DECIMAL_MUL) || (longtitude > 180 * DECIMAL_MUL))
		return -1;

	if (accuracy < 0)
		return -1;
	return 0;
}

long sys_set_gps_location(struct gps_location __user *loc)
{
	// TODO
	struct gps_location buf_loc;
	if (loc == NULL)
		return -EINVAL;
	if (copy_from_user(&buf_loc, loc, sizeof(struct gps_location)))
		return -EFAULT;
	
	if (is_valid_input(&buf_loc) == -1)
		return -EINVAL;
	mutex_lock(&gps_lock);
	curr_loc.lat_integer = buf_loc.lat_integer;
	curr_loc.lat_fractional = buf_loc.lat_fractional;
	curr_loc.lng_integer = buf_loc.lng_integer;
	curr_loc.lng_fractional = buf_loc.lng_fractional;
	curr_loc.accuracy = buf_loc.accuracy;
	mutex_unlock(&gps_lock);
	print_curr_loc();
	return 0;
}

long sys_get_gps_location(const char __user *pathname, struct gps_location __user *loc)
{
	// TODO
	return 0;
}
