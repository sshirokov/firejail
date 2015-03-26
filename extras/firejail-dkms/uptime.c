#include "firejail.h"


static int uptime_show(struct seq_file *m, void *v) {
	NsRule *ptr = find_sandbox(current->nsproxy);
	if (ptr) {
		unsigned long long delta;
		struct timespec uptime;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0))
		do_posix_clock_monotonic_gettime(&uptime);
		monotonic_to_bootbased(&uptime);
#else
		get_monotonic_boottime(&uptime);
#endif
		
		delta =  (unsigned long long) uptime.tv_sec - (unsigned long long) ptr->real_start_time.tv_sec;
		seq_printf(m, "%llu 0\n", delta);
	}
	else
		seq_printf(m, "0 0\n");
	return 0;
}

static int uptime_open(struct inode *inode, struct file *file) {
	return single_open(file, uptime_show, NULL);
}


const struct file_operations uptime_fops = {
	.owner     = THIS_MODULE,
	.open      = uptime_open,
	.read      = seq_read,
	.llseek    = seq_lseek,
	.release   = single_release,
};
