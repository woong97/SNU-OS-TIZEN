#include <linux/gps.h>
#include <linux/kernel.h>

long sys_set_gps_location(struct gps_location __user *loc)
{
	// TODO
	return 0;
}

long sys_get_gps_location(const char __user *pathname, struct gps_location __user *loc)
{
	// TODO
	return 0;
}
