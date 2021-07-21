/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2018-2019 Sultan Alsawaf <sultan@kerneltoast.com>.
 */
#ifndef _B1C1_INIT_H_
#define _B1C1_INIT_H_

#include <linux/init.h>

/* Selected and ordered according to /vendor/etc/init.insmod.cfg */
enum b1c1_init_id {
	B1C1_WLAN,
	B1C1_INIT_MAX
};

#if defined(CONFIG_BOARD_B1C1)
int register_b1c1_init_fn(int (*fn)(void), enum b1c1_init_id id) __init;

/* The function passed to b1c1_init CANNOT be in the .init section! */
#define b1c1_init(fn, id) \
static int __init fn##_init(void)		\
{						\
	return register_b1c1_init_fn(fn, id);	\
}						\
early_initcall(fn##_init)
#else
#define b1c1_init(fn, id) module_init(fn)
#endif

#endif /* _B1C1_INIT_H_ */
