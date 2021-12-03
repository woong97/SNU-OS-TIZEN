#include <linux/gps.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/namei.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
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
#define DECIMAL_MAX	(MAX_FRAC + 1)

DEFINE_MUTEX(gps_lock);

struct gps_location curr_loc = {
	.lat_integer = 0,
	.lat_fractional = 0,
	.lng_integer = 0,
	.lng_fractional = 0,
	.accuracy = 0
};

struct gps_float {
	long long integer;
	long long decimal;
};

void set_gps_float(struct gps_float *gf, long long integer, long long decimal)
{
	gf->integer = integer;
	gf->decimal = decimal;
}

// This function an be called only if a and b are positive
struct gps_float __pure_sub_gps_float(struct gps_float *a, struct gps_float *b)
{
	struct gps_float ret;
	long long integer, decimal;
	if ((a->integer > b->integer) || (a->integer == b->integer && a->decimal >= b->decimal)) {
		if (a->decimal < b->decimal) {
			decimal = DECIMAL_MAX + a->decimal - b->decimal;
			integer = a->integer - b->integer - 1;
		} else {
			decimal = a->decimal - b->decimal;
			integer = a->integer - b->integer;
		}
		set_gps_float(&ret, integer, decimal);
	} else {
		ret = __pure_sub_gps_float(b, a);
		set_gps_float(&ret, -(ret.integer), ret.decimal);
	}
	return ret;
}


struct gps_float add_gps_float(struct gps_float *a, struct gps_float *b)
{
	struct gps_float ret, tmp;
	long long integer, decimal;
	if ((a->integer >= 0 && b->integer >= 0)) {
		decimal = (a->decimal + b->decimal) % DECIMAL_MAX;
		integer = a->integer + b->integer + (a->decimal + b->decimal) / DECIMAL_MAX;
		set_gps_float(&ret, integer, decimal);
	} else if (a->integer >= 0 && b->integer < 0) {
		set_gps_float(&tmp, -(b->integer), b->decimal);
		ret = __pure_sub_gps_float(a, &tmp);
	} else if (a->integer < 0 && b->integer >=0) {
		set_gps_float(&tmp, -(a->integer), a->decimal);
		ret = __pure_sub_gps_float(b, &tmp);
	} else if (a->integer < 0 && b->integer < 0) {
		decimal = (a->decimal + b->decimal) % DECIMAL_MAX;
		integer = -((-a->integer) + (-b->integer) + (a->decimal + b->decimal) / DECIMAL_MAX); 
		set_gps_float(&ret, integer, decimal);
	}
	return ret;
}

struct gps_float sub_gps_float(struct gps_float *a, struct gps_float *b)
{
	struct gps_float tmp;
	set_gps_float(&tmp, -(b->integer), b->decimal);
	return add_gps_float(a, &tmp);
}

struct gps_float mul_gps_float(struct gps_float *a, struct gps_float *b)
{
	struct gps_float ret, tmp;
	long long integer = 0;
	long long decimal = 0;
	long long rem;

	if(a->integer >= 0 && b->integer >= 0) {
		integer += (a->integer * b->integer);
		integer += ((a->integer * b->decimal) / DECIMAL_MAX);
		integer += ((a->decimal * b->integer) / DECIMAL_MAX);

		decimal +=  ((a->integer * b->decimal) % DECIMAL_MAX);
		decimal += ((a->decimal * b->integer) % DECIMAL_MAX);
		decimal += ((a->decimal * b->decimal) / DECIMAL_MAX);

		rem = ((a->decimal * b->decimal) % DECIMAL_MAX);
		if (rem >= DECIMAL_MAX / 2)
			decimal += 1;
		integer = integer + decimal / DECIMAL_MAX;
		decimal = decimal % DECIMAL_MAX;

		set_gps_float(&ret, integer, decimal);
	} else if (a->integer >= 0 && b->integer < 0 ) {
		set_gps_float(&tmp, -(b->integer), b->decimal);
		ret = mul_gps_float(a, &tmp);
		set_gps_float(&ret, -(ret.integer), ret.decimal);
	} else if (a->integer < 0 && b->integer >=0) {
		set_gps_float(&tmp, -(a->integer), a->decimal);
		ret = mul_gps_float(b, &tmp);
		set_gps_float(&ret, -(ret.integer), ret.decimal);
	} else if (a->integer < 0 && b->integer < 0) {
		set_gps_float(&tmp, -(a->integer), a->decimal);
		ret = mul_gps_float(b, &tmp);
	}
	return ret;
}

/* param: a
 * return: 1/a
 */
struct gps_float inverse_gps_float(struct gps_float *a)
{
	struct gps_float ret, tmp;
	int quotient, remainder;
	int integer;
	int decimal;
	int multiplier;
	long long dividend;
	int i;
	
	if (a->integer >=0 ) {
		long long divisor = a->integer * DECIMAL_MAX + a->decimal;
		integer = DECIMAL_MAX / divisor;
		remainder = DECIMAL_MAX % divisor;
		decimal = 0;
		multiplier = DECIMAL_MAX / 10;
		for (i = 0; i <= 6; i++) {
			dividend = remainder * 10;
			quotient = dividend / divisor;
			if (i < 6)
				decimal = decimal + multiplier * quotient;
			remainder = dividend % divisor;
			multiplier /= 10;
		}
		if (quotient >= 5)
			decimal += 1;
		set_gps_float(&ret, integer, decimal);
	} else {
		set_gps_float(&tmp, -(a->integer), a->decimal);
		ret = inverse_gps_float(&tmp);
		set_gps_float(&ret, -(ret.integer), ret.decimal);
	}
	return ret;
}

// In our struct, we cannot express -0.xxxx. because -0 = 0. 
// Thus dealing wiht negative number is meaningless. But I implement correctly for add, mul, sub, div for negative input
// in which the integer part is not zero.
struct gps_float div_gps_float(struct gps_float *a, struct gps_float *b)
{
	struct gps_float ret, tmp;
	int quotient, remainder, multiplier;
	int integer, decimal;
	long long dividend, divisor;
	int i;
	
	if (a->integer >=0 && b->integer >=0) {
		dividend = a->integer * DECIMAL_MAX + a->decimal;
		divisor = b->integer * DECIMAL_MAX + b->decimal;
		integer = dividend / divisor;
		remainder = dividend % divisor;
		decimal = 0;
		multiplier = DECIMAL_MAX / 10;
		for (i = 0; i <= 6; i++) {
			dividend = remainder * 10;
			quotient = dividend / divisor;
			if (i < 6)
				decimal = decimal + multiplier * quotient;
			remainder = dividend % divisor;
			multiplier /= 10;
		}
		if (quotient >= 5)
			decimal += 1;
		set_gps_float(&ret, integer, decimal);
	} else if (a->integer >= 0 && b->integer < 0 ) {
		set_gps_float(&tmp, -(b->integer), b->decimal);
		ret = div_gps_float(a, &tmp);
		set_gps_float(&ret, -(ret.integer), ret.decimal);
	} else if (a->integer < 0 && b->integer >= 0) {
		set_gps_float(&tmp, -(a->integer), a->decimal);
		ret = div_gps_float(&tmp, b);
		set_gps_float(&ret, -(ret.integer), ret.decimal);
	} else {
		set_gps_float(&tmp, -(a->integer), a->decimal);
		ret = div_gps_float(&tmp, b);
		set_gps_float(&ret, -(ret.integer), ret.decimal);
	}
	return ret;
}


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

void test_cal(int integer_x, int decimal_x, int integer_y, int decimal_y)
{
	printk("===== Test Calcualtion =====\n");
	struct gps_float a, b, c;
	set_gps_float(&a, integer_x, decimal_x);
	set_gps_float(&b, integer_y, decimal_y);
	printk("Input a: %lld.%lld, b: %lld.%lld\n", a.integer, a.decimal, b.integer, b.decimal);
	c = add_gps_float(&a, &b);
	printk("add = %lld.%lld\n", c.integer, c.decimal);

	c = sub_gps_float(&a, &b);
	printk("sub = %lld.%lld\n", c.integer, c.decimal);

	c = mul_gps_float(&a, &b);
	printk("mul = %lld.%lld\n", c.integer, c.decimal);
	
	c = div_gps_float(&a, &b);
	printk("div = %lld.%lld\n", c.integer, c.decimal);
}

static int is_valid_input(struct gps_location *loc)
{
	int lat_integer = loc->lat_integer;
	int lat_fractional = loc->lat_fractional;
	int lng_integer = loc->lng_integer;
	int lng_fractional = loc->lng_fractional;
	int accuracy = loc->accuracy;
	long latitude = lat_integer * DECIMAL_MAX + lat_fractional; 
	long longtitude = lng_integer * DECIMAL_MAX + lng_fractional;
	if (lat_fractional < 0 || lng_fractional < 0 || lat_fractional > MAX_FRAC || lng_fractional > MAX_FRAC)
		return -1;
	if ((latitude < -90 * DECIMAL_MAX) || (latitude > 90 * DECIMAL_MAX))
		return -1;
	if ((longtitude < -180 * DECIMAL_MAX) || (longtitude > 180 * DECIMAL_MAX))
		return -1;

	if (accuracy < 0)
		return -1;
	return 0;
}

long sys_set_gps_location(struct gps_location __user *loc)
{
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

/// Permission denied: return -1, Succeed: return 0
int gps_distance_permission(struct inode *inode)
{
	// TODO
	return 0;
}

long sys_get_gps_location(const char __user *pathname, struct gps_location __user *loc)
{
	struct gps_location kernel_buf;
	struct inode *inode;
	struct path path;
	int error;
	unsigned int lookup_flags = LOOKUP_FOLLOW|LOOKUP_AUTOMOUNT;
	if (pathname == NULL || loc == NULL)
		return -EINVAL;
	
	error = user_path_at(AT_FDCWD, pathname, lookup_flags, &path);
	if (error)
		return error;
	inode = path.dentry->d_inode;

	if (inode_permission(inode, MAY_READ)) {
		printk("Permission Denied\n");
		return -EACCES;
	}
	
	if (inode->i_op->get_gps_location) {
		inode->i_op->get_gps_location(inode, &kernel_buf);
	} else {
		return -EOPNOTSUPP;
	}

	if(copy_to_user(loc, kernel_buf, sizeof(struct gps_location) != 0))
		return -EFAULT;
	return 0;
}
