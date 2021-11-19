#include <linux/list.h>

#define	MIN_DEGREE	0
#define MAX_DEGREE	359
#define END_DEGREE	360

#define RT_READ		0
#define RT_WRITE	1

typedef struct __rotation_range_descriptor {
	pid_t pid;
	int degree;
	int range;
	int rw_type;
	struct list_head list;
} rot_rd;

