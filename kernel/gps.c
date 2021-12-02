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

DEFINE_MUTEX(gps_lock);
static struct gps_location curr_loc = {
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

long sys_set_gps_location(struct gps_location __user *loc)
{
	// TODO
	struct gps_location buf_loc;
	if (loc == NULL)
		return -EINVAL;
	if (copy_from_user(&buf_loc, loc, sizeof(struct gps_location)))
		return -EFAULT;
	print_curr_loc();	
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
