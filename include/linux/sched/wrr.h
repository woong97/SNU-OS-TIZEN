/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCHED_WRR_H
#define _LINUX_SCHED_WRR_H

#include <linux/sched.h>

struct task_struct;

/*
 *  * default timeslice is 100 msecs (used only for SCHED_RR tasks).
 *   * Timeslices get refilled after they expire.
 *    */
#define WRR_TIMESLICE		(100 * HZ / 1000)
#define WRR_DEFAULT_WEIGHT	10

#endif
