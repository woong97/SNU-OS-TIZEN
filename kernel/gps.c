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
#define MAX_FRAC		999999
#define DECIMAL_MAX		(MAX_FRAC + 1)
#define ACOS_POS		1
#define ACOS_NEG		0

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

struct gps_float AVG_EARTH_RADIUS = {
	.integer = 6371,
	.decimal = 0
};

struct gps_float PI = {
	.integer = 3,
	.decimal = 141593
};

struct gps_float HALF_PI = {
	.integer = 1,
	.decimal = 570796
};

// pi / 180
struct gps_float RADIAN_PI = {
	.integer = 0,
	.decimal = 17453
};

struct gps_float ONE = {
	.integer = 1,
	.decimal = 0
};

struct gps_float TWO = {
	.integer = 2,
	.decimal = 0
};

struct gps_float ZERO = {
	.integer = 0,
	.decimal = 0
};

struct gps_float HALF = {
	.integer = 0,
	.decimal = 500000
};

struct gps_float KM2M = {
	.integer = 1000,
	.decimal = 0
};

struct gps_float gps_haversine_distance(struct gps_location file_loc);

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
	struct gps_float ret;
	long long integer, decimal;

	decimal = (a->decimal + b->decimal) % DECIMAL_MAX;
	integer = a->integer + b->integer + (a->decimal + b->decimal) / DECIMAL_MAX;
	set_gps_float(&ret, integer, decimal);

	return ret;
}

// this function only can be called a, b are positive , a>=b
struct gps_float sub_gps_float(struct gps_float *a, struct gps_float *b)
{
	struct gps_float ret;
	long long integer, decimal;
	if (a->decimal < b->decimal) {
		decimal = DECIMAL_MAX + a->decimal - b->decimal;
		integer = a->integer - b->integer - 1;
	} else {
		decimal = a->decimal - b->decimal;
		integer = a->integer - b->integer;
	}
	set_gps_float(&ret, integer, decimal);
	return ret;
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

// In our struct, we cannot express -0.xxxx. because -0 = 0. 
// Thus dealing wiht negative number is meaningless. But I implement correctly for add, mul, sub, div for negative input
// in which the integer part is not zero.
struct gps_float div_gps_float(struct gps_float *a, struct gps_float *b)
{
	struct gps_float ret, tmp;
	long long quotient, remainder, multiplier;
	long long integer, decimal;
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

// a > b: return 1, a==b: return 0; a<b: return -1
int comp_gps_float(struct gps_float *a, struct gps_float *b)
{
	if (a->integer > b->integer) {
		return 1;
	} else if (a->integer < b->integer) {
		return -1;
	} else {
		if (a->decimal > b->decimal) {
			return 1;
		} else if (a->decimal < b->decimal) {
			return -1;
		} else {
			return 0;
		}
	}
}

struct gps_float power_gps_float(struct gps_float *x, int power)
{
	int i;
	struct gps_float ret = ONE;
	for (i = 0; i < power ; i++) {
		ret = mul_gps_float(&ret, x);
	}
	return ret;
}

struct gps_float factorial_gps_float(int num)
{
	struct gps_float ret = ONE;
	struct gps_float mul;
	int i;
	for (i = 1; i <= num; i++) {
		set_gps_float(&mul, i, 0);
		ret = mul_gps_float(&ret, &mul);
	}
	return ret;
}

struct gps_float degree2rad(struct gps_float *x)
{
	return mul_gps_float(x, &RADIAN_PI);
}
// return |sin(x)| = sin(|x|)

struct gps_float abs_sin_gps_float(struct gps_float *x)
{
	// int integer, decimal;
	struct gps_float ret;
	struct gps_float term;
	// int factorial;
	struct gps_float power_tmp, factorial_tmp;
	if (x->integer < 0)
		set_gps_float(x, -(x->integer), x->decimal);
	
	term = *x;
	ret = *x;
	
	power_tmp = power_gps_float(x, 3);
	factorial_tmp = factorial_gps_float(3);
	term = div_gps_float(&power_tmp, &factorial_tmp);		// x^3/3!
	if (comp_gps_float(&ret, &term) >= 0) {
		ret = sub_gps_float(&ret, &term);			// x-x^3/3!
		power_tmp = power_gps_float(x, 5);
		factorial_tmp = factorial_gps_float(5);
		term = div_gps_float(&power_tmp, &factorial_tmp);	// x^5/5!
		ret = add_gps_float(&ret, &term);			// x-x^3+x^5/5!

		power_tmp = power_gps_float(x, 7);
		factorial_tmp = factorial_gps_float(7);

		term = div_gps_float(&power_tmp, &factorial_tmp);	// x^7/7!
		if (comp_gps_float(&ret, &term) >= 0) {
			ret = sub_gps_float(&ret, &term);
		} else {
			ret = sub_gps_float(&term, &ret);
		}
	} else if(comp_gps_float(&ret, &term) == -1) {
		ret = sub_gps_float(&term, &ret);
		power_tmp = power_gps_float(x, 5);
		factorial_tmp = factorial_gps_float(5);
		term = div_gps_float(&power_tmp, &factorial_tmp);	// x^5/5!
		if (comp_gps_float(&ret, &term) >= 0) {
			ret = sub_gps_float(&ret, &term);		// originally negative
			
			power_tmp = power_gps_float(x, 7);
			factorial_tmp = factorial_gps_float(7);
			term = div_gps_float(&power_tmp, &factorial_tmp);	// x^7/7!
			ret = add_gps_float(&ret, &term);
		} else {
			ret = sub_gps_float(&term, &ret);		// originally positive
			power_tmp = power_gps_float(x, 7);
			factorial_tmp = factorial_gps_float(7);
			term = div_gps_float(&power_tmp, &factorial_tmp);	// x^7/7!
			if (comp_gps_float(&ret, &term) >= 0) {
				ret = sub_gps_float(&ret, &term);
			} else {
				ret = sub_gps_float(&term, &ret);
			}
		}

	}
	return ret;
}

struct gps_float abs_cos_gps_float(struct gps_float *x)
{
	struct gps_float ret;
	struct gps_float term = *x;
	struct gps_float power_tmp, factorial_tmp;

	ret = ONE;
	power_tmp = power_gps_float(x, 2);
	factorial_tmp = factorial_gps_float(2);
	term = div_gps_float(&power_tmp, &factorial_tmp);		// x^3/3!
	if (comp_gps_float(&ret, &term) >= 0) {
		ret = sub_gps_float(&ret, &term);			// x-x^3/3!
		power_tmp = power_gps_float(x, 4);
		factorial_tmp = factorial_gps_float(4);
		term = div_gps_float(&power_tmp, &factorial_tmp);	// x^5/5!
		ret = add_gps_float(&ret, &term);			// x-x^3+x^5/5!

		power_tmp = power_gps_float(x, 6);
		factorial_tmp = factorial_gps_float(6);
		term = div_gps_float(&power_tmp, &factorial_tmp);	// x^7/7!
		if (comp_gps_float(&ret, &term) >= 0) {
			ret = sub_gps_float(&ret, &term);
		} else {
			ret = sub_gps_float(&term, &ret);
		}
	} else if(comp_gps_float(&ret, &term) == -1) {

		ret = sub_gps_float(&term, &ret);
		power_tmp = power_gps_float(x, 4);
		factorial_tmp = factorial_gps_float(4);
		term = div_gps_float(&power_tmp, &factorial_tmp);	// x^5/5!

		if (comp_gps_float(&ret, &term) >= 0) {
			ret = sub_gps_float(&ret, &term);		// originally negative
			
			power_tmp = power_gps_float(x, 6);
			factorial_tmp = factorial_gps_float(6);
			term = div_gps_float(&power_tmp, &factorial_tmp);	// x^7/7!
			ret = add_gps_float(&ret, &term);
		} else {
			ret = sub_gps_float(&term, &ret);		// originally positive
			
			power_tmp = power_gps_float(x, 6);
			factorial_tmp = factorial_gps_float(6);
			term = div_gps_float(&power_tmp, &factorial_tmp);	// x^7/7!
			if (comp_gps_float(&ret, &term) >= 0) {
				ret = sub_gps_float(&ret, &term);
			} else {
				ret = sub_gps_float(&term, &ret);
			}
		}

	}

	return ret;

}


///input : if input is negative, use this function
//arccos x return always positive
struct gps_float acos_gps_float(struct gps_float *x, int acos_flag)
{
	struct gps_float ret;
	struct gps_float term = *x;
	struct gps_float power_tmp, factor_tmp;
	struct gps_float mul;
	if (x->integer < 0)
		set_gps_float(x, -(x->integer), x->decimal);

	ret = HALF_PI;
	
	power_tmp = power_gps_float(x, 1);
	set_gps_float(&factor_tmp, 1, 0);
	term = div_gps_float(&power_tmp, &factor_tmp);		// x
	
	// pi/2 - x	
	if (acos_flag == ACOS_POS) {
		ret = sub_gps_float(&ret, &term);	
	} else {
		ret = add_gps_float(&ret, &term);
	}
	
	power_tmp = power_gps_float(x, 3);			// x^3
	set_gps_float(&factor_tmp, 6, 0);
	term = div_gps_float(&power_tmp, &factor_tmp);		// x^3/6

	// pi/2 - x - x^3/6
	if (acos_flag == ACOS_POS) {
		ret = sub_gps_float(&ret, &term);		
	} else {
		ret = add_gps_float(&ret, &term);
	}

	power_tmp = power_gps_float(x, 5);			// x^5
	set_gps_float(&factor_tmp, 40, 0);			// 40
	set_gps_float(&mul, 3, 0);				// 3
	factor_tmp = div_gps_float(&factor_tmp, &mul);		// 40/3
	term = div_gps_float(&power_tmp, &factor_tmp);		// 3x^5/40

	// pi/2 - x - x^3/6 - 3x^5/40
	if (acos_flag == ACOS_POS) {
		ret = sub_gps_float(&ret, &term);		
	} else {
		ret = add_gps_float(&ret, &term);
	}

	power_tmp = power_gps_float(x, 7);			// x^7
	set_gps_float(&factor_tmp, 112, 0);			// 112
	set_gps_float(&mul, 5, 0);				//5
	factor_tmp = div_gps_float(&factor_tmp, &mul);		// 112/5
	term = div_gps_float(&power_tmp, &factor_tmp);		// 5x^7/112

	// pi/2 - x - x^3/6 - 3x^5/40 - 5x^7/112
	if (acos_flag == ACOS_POS) {
		ret = sub_gps_float(&ret, &term);		
	} else {
		ret = add_gps_float(&ret, &term);
	}

	power_tmp = power_gps_float(x, 9);			// x^9
	set_gps_float(&factor_tmp, 1152, 0);			// 1152
	set_gps_float(&mul, 35, 0);				//35
	factor_tmp = div_gps_float(&factor_tmp, &mul);		// 1152/35
	term = div_gps_float(&power_tmp, &factor_tmp);		// 35x^9/1152

	// pi/2 - x - x^3/6 - 3x^5/40 - 5x^7/112 - 35x^9/1152
	if (acos_flag == ACOS_POS) {
		ret = sub_gps_float(&ret, &term);		
	} else {
		ret = add_gps_float(&ret, &term);
	}

	power_tmp = power_gps_float(x, 11);			// x^9
	set_gps_float(&factor_tmp, 2816, 0);			// 2816
	set_gps_float(&mul, 63, 0);				//63
	factor_tmp = div_gps_float(&factor_tmp, &mul);		// 2816/63
	term = div_gps_float(&power_tmp, &factor_tmp);		// 63x^11/2816

	if (acos_flag == ACOS_POS) {
		ret = sub_gps_float(&ret, &term);		
	} else {
		ret = add_gps_float(&ret, &term);
	}

	power_tmp = power_gps_float(x, 13);			// x^13
	set_gps_float(&factor_tmp, 13312, 0);			// 13312
	set_gps_float(&mul, 231, 0);				// 231
	factor_tmp = div_gps_float(&factor_tmp, &mul);		// 13312/231
	term = div_gps_float(&power_tmp, &factor_tmp);		// 231x^13/13312

	if (acos_flag == ACOS_POS) {
		ret = sub_gps_float(&ret, &term);		
	} else {
		ret = add_gps_float(&ret, &term);
	}
	
	power_tmp = power_gps_float(x, 15);			// x^15
	set_gps_float(&factor_tmp, 10240, 0);			// 10240
	set_gps_float(&mul, 143, 0);				// 143
	factor_tmp = div_gps_float(&factor_tmp, &mul);		// 10240/143
	term = div_gps_float(&power_tmp, &factor_tmp);		// 143x^15/10240

	if (acos_flag == ACOS_POS) {
		ret = sub_gps_float(&ret, &term);		
	} else {
		ret = add_gps_float(&ret, &term);
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
	struct gps_float rad;
	set_gps_float(&a, integer_x, decimal_x);
	set_gps_float(&b, integer_y, decimal_y);
	
	printk("Input a: %lld.%lld, b: %lld.%lld\n", a.integer, a.decimal, b.integer, b.decimal);
	/*
	c = add_gps_float(&a, &b);
	printk("add = %lld.%lld\n", c.integer, c.decimal);

	c = sub_gps_float(&a, &b);
	printk("sub = %lld.%lld\n", c.integer, c.decimal);

	c = mul_gps_float(&a, &b);
	printk("mul = %lld.%lld\n", c.integer, c.decimal);
	
	c = div_gps_float(&a, &b);
	printk("div = %lld.%lld\n", c.integer, c.decimal);
	*/
	/*
	rad = degree2rad(&a);
	printk("rad = %lld.%lld\n", rad.integer,rad.decimal);
	c = abs_sin_gps_float(&rad);
	printk("sin = %lld.%lld\n", c.integer, c.decimal);
	
	c = abs_cos_gps_float(&rad);
	printk("cos = %lld.%lld\n", c.integer, c.decimal);
	*/
	c = acos_gps_float(&a, ACOS_POS);
	printk("acos positive case = %lld.%lld\n", c.integer, c.decimal);
	
	c = acos_gps_float(&a, ACOS_NEG);
	printk("acos negative case = %lld.%lld\n", c.integer, c.decimal);
	
	struct gps_location my = {
		.lat_integer=a.integer,
		.lat_fractional=a.decimal,
		.lng_integer=b.integer,
		.lng_fractional=b.decimal
	};

	c = gps_haversine_distance(my);
	printk("distance: %lld.%lld\n", c.integer, c.decimal);
}

static int is_valid_input(struct gps_location *loc)
{
	int lat_integer = loc->lat_integer;
	int lat_fractional = loc->lat_fractional;
	int lng_integer = loc->lng_integer;
	int lng_fractional = loc->lng_fractional;
	int accuracy = loc->accuracy;
	long long latitude = lat_integer * DECIMAL_MAX + lat_fractional; 
	long long longtitude = lng_integer * DECIMAL_MAX + lng_fractional;
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
	// print_curr_loc();
	/*
	test_cal(90, 0, 42, 0);
	test_cal(-60, 0, 1, 1);
	test_cal(135, 0, 1, 1);
	test_cal(45, 0, 1, 1);
	
	test_cal(48, 856700, 2, 350800);
	test_cal(45, 759722, 123, 100000);
	*/
	return 0;
}

struct gps_float gps_haversine_distance(struct gps_location file_loc)
{
	struct gps_float lat1, lng1;			// file gps position
	struct gps_float lat2, lng2;			// current gps position
	struct gps_float lat, lng;			// difference of src and target position
	struct gps_float cos_lat1, cos_lat2;
	struct gps_float harv;
	struct gps_float tmp;
	struct gps_float distance;

	struct gps_float lval, rval;
	
	set_gps_float(&lat1, file_loc.lat_integer, file_loc.lat_fractional);
	set_gps_float(&lng1, file_loc.lng_integer, file_loc.lng_fractional);
	set_gps_float(&lat2, curr_loc.lat_integer, curr_loc.lat_fractional);
	set_gps_float(&lng2, curr_loc.lng_integer, curr_loc.lng_fractional);

	lat1 = degree2rad(&lat1);
	lng1 = degree2rad(&lng1);
	lat2 = degree2rad(&lat2);
	lng2 = degree2rad(&lng2);
	
	if (comp_gps_float(&lat2, &lat1) >= 0) {
		lat = sub_gps_float(&lat2, &lat1);
	} else {
		lat = sub_gps_float(&lat1, &lat2);
	}
	
	if (comp_gps_float(&lng2, &lng1) >= 0) {
		lng = sub_gps_float(&lng2, &lng1);
	} else {
		lng = sub_gps_float(&lng1, &lng2);
	}
	
	lval = mul_gps_float(&lat, &HALF);		// lat * 0.5
	lval = abs_sin_gps_float(&lval);		// sin(lat * 0.5)
	lval = mul_gps_float(&lval, &lval);		// sin(lat * 0.5) ** 2;
	
	rval = mul_gps_float(&lng, &HALF);		// lng * 0.5
	rval = abs_sin_gps_float(&rval);		// sin(lng * 0.5)
	rval = mul_gps_float(&rval, &rval);		// sin(lng * 0.5) ** 2;

	// fortunaltey latitued is always in [-90, 90] that means cosine vlaue is positive!
	cos_lat1 = abs_cos_gps_float(&lat1);		// cos(lat1)
	cos_lat2 = abs_cos_gps_float(&lat2);		// cos(lat2)

	rval = mul_gps_float(&rval, &cos_lat1); 	// sin(lng * 0.5) ** 2 * cos(lat1);
	rval = mul_gps_float(&rval, &cos_lat2);		// sin(lng * 0.5) ** 2 * cos(lat1) * cos(lat2);
	
	harv = add_gps_float(&lval, &rval);		// sin(lng * 0.5) ** 2 + cos(lat1) * cos(lat2) * sin(lng * 0.5) ** 2
	tmp = mul_gps_float(&harv, &TWO);		// 2 * harv
	if (comp_gps_float(&ONE, &tmp) >= 0) {
		tmp = sub_gps_float(&ONE, &tmp);	// 1-2*harv is positive
		tmp = acos_gps_float(&tmp, ACOS_POS);		// acos(1-2*harv); use ACOS_POS as flag
	} else {
		tmp = sub_gps_float(&tmp, &ONE);	// 1-2*harv is negative. so we calculate 2*harv - 1
		tmp = acos_gps_float(&tmp, ACOS_NEG);		// since 1-2*harv is originally negative, use ACOS_NEG as flag

	}
	
	distance = mul_gps_float(&AVG_EARTH_RADIUS, &tmp);	// R * acos(1 - 2*harv)
	distance = mul_gps_float(&distance, &KM2M);		// transform km to m
	return distance;
}

/// Permission denied: return -1, Succeed: return 0
int gps_distance_permission(struct inode *inode)
{
	struct gps_location file_loc;
	struct gps_float distance, accuracy;
	if (inode->i_op->get_gps_location) {
		inode->i_op->get_gps_location(inode, &file_loc);
	}
	distance = gps_haversine_distance(file_loc);
	set_gps_float(&accuracy, curr_loc.accuracy + file_loc.accuracy, 0);

	if (comp_gps_float(&accuracy, &distance) >= 0) {
		return 0;
	} else {
		return -1;
	}
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

	if(copy_to_user(loc, &kernel_buf, sizeof(struct gps_location) != 0))
		return -EFAULT;
	return 0;
}
