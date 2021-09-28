#include <linux/kernel.h>
#include <linux/prinfo.h>
#include <linux/unistd.h>
//#include <uapi/asm-generic/errno-base.h>
#include <linux/uaccess.h>

asmlinkage int sys_ptree(struct prinfo *buf, int *nr){
	int tmp_nr;
	struct prinfo tmp_prinfo;

	if (buf == NULL || nr == NULL)
		return -EINVAL;

	// need to check: access_ok just check availabilty of scope of address
	// So we will use copy_from_user
	// need check: copy_from_user is the most powerful among
	// copy_from user, get_user, access_ok
	if (copy_from_user(&tmp_nr, nr, sizeof(tmp_nr)))
		return -EFAULT;
	if (copy_from_user(&tmp_prinfo, buf, sizeof(tmp_prinfo)))
		return -EFAULT;
	if (tmp_nr < 1) {
		return -EINVAL;
	}
	return *nr;

}

/*
 * asmlinkabe: for i386 architecture
 * printk: <linux/printk.h>
 * why not <linux/prinfo.h>?
 */
