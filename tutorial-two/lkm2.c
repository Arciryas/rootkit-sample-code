/*lkm2.c*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <asm/cacheflush.h>
#include <asm/page.h>
#include <asm/current.h>

#include "sys_map_addr.h"

unsigned long *sys_call_table = (unsigned long *) SYS_CALL_TABLE;

asmlinkage long (*real_mkdir)(const char __user *pathname,
				umode_t mode);
asmlinkage long fake_mkdir(const char __user *pathname, umode_t mode)
{
	printk("Arciryas:mkdir-%s\n", pathname);
	
	return (*real_mkdir)(pathname, mode);
}

static int lkm_init(void)
{
	write_cr0(read_cr0() & (~0x10000));
	real_mkdir = (void *)sys_call_table[__NR_mkdir];
	sys_call_table[__NR_mkdir] = fake_mkdir;
	write_cr0(read_cr0() | 0x10000);

	printk("Arciryas:module loaded\n");

	return 0;
}

static void lkm_exit(void)
{
	write_cr0(read_cr0() & (~0x10000));	
	sys_call_table[__NR_mkdir] = real_mkdir;
	write_cr0(read_cr0() | 0x10000);
	printk("Arciryas:module removed\n");
}

module_init(lkm_init);
module_exit(lkm_exit);
