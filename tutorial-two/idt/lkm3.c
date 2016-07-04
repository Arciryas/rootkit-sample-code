/*lkm3.c*/

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

	
unsigned long *sys_call_table;

struct 
{
	unsigned short size;
	unsigned int addr;
}__attribute__((packed)) idtr;

struct
{
	unsigned short offset_1;
	unsigned short selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short offset_2;
}__attribute__((packed)) idt;

unsigned long  *find_sys_call_table(void)
{
	unsigned int sys_call_off;
	char *p;
	int i;
	unsigned int ret;

	asm("sidt %0":"=m"(idtr));
	printk("Arciryas:idt table-0x%x\n", idtr.addr);

	memcpy(&idt, idtr.addr+8*0x80, sizeof(idt));
	sys_call_off = ((idt.offset_2<<16) | idt.offset_1);

	p = sys_call_off;
	for(i=0; i<100; i++)
	{
		if(p[i]=='\xff' && p[i+1]=='\x14' && p[i+2]=='\x85')
			ret = *(unsigned int *)(p+i+3);
	}
	printk("Arciryas:sys_call_table-0x%x\n", ret);
	return (unsigned long**)ret;
}

asmlinkage long (*real_mkdir)(const char __user *pathname,
				umode_t mode);
asmlinkage long fake_mkdir(const char __user *pathname, umode_t mode)
{
	printk("Arciryas:mkdir-%s\n", pathname);
	
	return (*real_mkdir)(pathname, mode);
}

static int lkm_init(void)
{
	sys_call_table = find_sys_call_table();

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
