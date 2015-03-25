#include "firejail.h"

#define PROC_FILE_NAME	"firejail"
#define PROC_UPTIME		"firejail-uptime"
struct timer_list cleanup_timer;		  // periodic cycle timer

//MODULE_LICENSE("GPLv2");
MODULE_LICENSE("GPL");

static struct nsproxy *main_ns;	// host nsproxy
static struct net *main_net;		// host net structure

NsRule head;
static atomic_t firejail_available = ATOMIC_INIT(1);

#if !(LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0))
static struct tracepoint *tp_sysenter;
static void set_tracepoint(struct tracepoint *tp, void *priv) {
//printk(KERN_INFO "trace point %s\n", tp->name);
	if (strcmp(tp->name, "sys_enter") == 0)
		tp_sysenter = tp;
}
#endif

//*****************************************************
// Syscall filtering
//*****************************************************
static void syscall_probe(void *__data, struct pt_regs *regs, long id) {
	NsRule *ptr;
	// printk(KERN_INFO "firejail %u: syscall %d\n", current->pid, (int) id);
	
	if (current->nsproxy == main_ns)
		return;
	if (id != __NR_connect)
		return;
	if (!regs)
		return;
		
#ifdef CONFIG_X86_64
	// skip events for 32bit processes running on 64bit kernels
	if (unlikely(test_tsk_thread_flag(current, TIF_IA32)))
		return;
#endif

	ptr = find_rule(current->nsproxy);
	if (ptr) {
//printk(KERN_INFO "syscall proxy %p\n", current->nsproxy);
		syscall_probe_connect(regs, id, ptr);
	}
}


//*****************************************************
// proc interface
//*****************************************************
// first seq call - return NULL to end the sequence
static void *firejail_seq_start(struct seq_file *s, loff_t *pos) {
	/* beginning a new sequence ? */
	if (*pos == 0) {
		return &head;
	}
	else {
		/* no => it's the end of the sequence, return end to stop reading */
		*pos = 0;
		return NULL;
	}
}


// next seq call - return NULL to end the sequence
static void *firejail_seq_next(struct seq_file *s, void *v, loff_t *pos) {
	NsRule *ptr = (NsRule *) v;
	return ptr->next;
}


// stop seq - called at the end of the sequence
static void firejail_seq_stop(struct seq_file *s, void *v) {
	/* nothing to do, we use a static value in start() */
}


// seq show - called for each sequence
static int firejail_seq_show(struct seq_file *s, void *v) {
	NsRule *ptr = (NsRule *) v;
//	if (ptr->active == 0)
//		return 0;
	if (ptr->nsproxy == NULL) {
		int active = 0;
		int inactive = 0;
		ptr = head.next;
		while (ptr) {
			if (ptr->active)
				active++;
			else
				inactive++;
			ptr = ptr->next;
		}
				
		seq_printf(s, "Kernel rules: %d active, %d inactive\n", active, inactive);
	}
		
	else if (current->nsproxy == main_ns && ptr->active){
		int i;
		seq_printf(s, "sandbox pid %d, nsproxy %p\n", ptr->sandbox_pid, ptr->nsproxy);
		for (i = 0; i < MAX_UNIX_PATH; i++) {
			if (ptr->unix_path[i])
				seq_printf(s, "    no connect unix %s\n", ptr->unix_path[i]);
			else
				break;
		}
	}
	
	else if (current->nsproxy == ptr->nsproxy) {
		seq_printf(s, "\tThis namespace is protected\n");
	}
	
	return 0;
}


static struct seq_operations firemon_seq_ops = {
	.start = firejail_seq_start,
	.next  = firejail_seq_next,
	.stop  = firejail_seq_stop,
	.show  = firejail_seq_show
};

static int firemon_open(struct inode *inode, struct file *file) {
	if (!atomic_dec_and_test (&firejail_available)) {
		atomic_inc(&firejail_available);
//		printk(KERN_INFO "firejail: /proc/firejail busy\n");
		return -EBUSY; /* already open */
	}
	return seq_open(file, &firemon_seq_ops);
};

static int firemon_release(struct inode *inode, struct file *filp) {
	atomic_inc(&firejail_available); /* release the device */
	return seq_release(inode, filp);
}
	
static ssize_t firejail_write(struct file *file, const char *buffer, size_t len, loff_t * off) {
	NsRule *ptr;
	char *cmd_buffer;
	unsigned long cmd_len;

	// extract the command
	if (len > CMD_MAX_SIZE)
		cmd_len = CMD_MAX_SIZE;
	else
		cmd_len = len;
	cmd_buffer = kmalloc(cmd_len + 1, GFP_KERNEL);
	if(copy_from_user(cmd_buffer, buffer, cmd_len))
		goto errout;
	cmd_buffer[cmd_len] = '\0';

	split_command(cmd_buffer);
	if (sargc == 0) {
		printk(KERN_INFO "firejail: invalid command\n");
		goto errout;
	}
//	for (i = 0; i < sargc; i++) {
//		printk(KERN_INFO "firejail: #%s#\n", sargv[i]);
//	}

	if (sargc == 4 &&
	    strcmp(sargv[0], "no") == 0 &&
	    strcmp(sargv[1], "connect") == 0 &&
	    strcmp(sargv[2], "unix") == 0) {
		int path_len;
		char *path;

		if (current->nsproxy == main_ns){
			printk(KERN_INFO "firejail: cannot create the sandbox from this namespace\n");
			goto errout;
		}

		// extract path
		path_len = strlen(sargv[3]);
		path =  kmalloc(path_len + 1, GFP_KERNEL);
	    	strcpy(path, sargv[3]);

		// create the rule
		ptr = find_or_create_rule();
		if (ptr) {
			int i;
			// empty path?
			for (i = 0; i < MAX_UNIX_PATH; i++) {
				if (ptr->unix_path[i] == NULL)
					break;
			}									
			if (i == MAX_UNIX_PATH) {
				kfree(path);
				printk(KERN_INFO "firejail: unix path limit reached\n");
				goto errout;
			}
			else {
				ptr->unix_path[i] = path;
				ptr->unix_path_len[i] = path_len;
				printk(KERN_INFO "firejail: sandbox %d, no connect unix %s.\n",
					 ptr->sandbox_pid, path);
			}
		}
		else {
			printk(KERN_INFO "firejail: cannot create sandbox\n");
			goto errout;
		}
	}

	else if (sargc == 1 && strcmp(sargv[0], "register") == 0) {
		if (current->nsproxy == main_ns){
			printk(KERN_INFO "firejail: cannot register the sandbox from this namespace\n");
			goto errout;
		}

		ptr = find_or_create_rule();
		if (!ptr) {
			printk(KERN_INFO "firejail: cannot create sandbox\n");
			goto errout;
		}
		
		
		ptr->net = get_net(ptr->nsproxy->net_ns);	
printk(KERN_INFO "net %p, %d\n", ptr->net, current->pid);		



#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0))
		do_posix_clock_monotonic_gettime(&ptr->real_start_time);
		monotonic_to_bootbased(&ptr->real_start_time);
#else
		get_monotonic_boottime(&ptr->real_start_time);
#endif
//		printk(KERN_INFO "firejail: new sandbox registered, pid %d, nsproxy %p\n", ptr->sandbox_pid, ptr->nsproxy);
	}
	
	else if (sargc == 1 && strcmp(sargv[0], "release") == 0) {
		NsRule *ptr = find_sandbox_pid(current->pid);
		if (ptr)
			ptr->active = 0;
	}
	
	else {
		printk(KERN_INFO "firejail: invalid command\n");
		goto errout;
	}
		
	kfree(cmd_buffer);
	return cmd_len;

errout:	
	kfree(cmd_buffer);
	return -EFAULT;
}


static struct file_operations firejail_fops = {
	.owner   = THIS_MODULE,
	.open    = firemon_open,
	.read    = seq_read,
	.write   = firejail_write,
	.llseek  = seq_lseek,
//	.release = seq_release
	.release = firemon_release
};

//*****************************************************
// module init/exit
//*****************************************************

//extern struct nsproxy init_nsproxy;
static int __init init_main(void) {
	int ret;
	printk(KERN_INFO "firejail: initializing module.\n");

	// initialize locals
	main_ns = current->nsproxy;
	main_net = get_net(current->nsproxy->net_ns);	
printk(KERN_INFO "main_net %p\n", main_net);	


	memset(&head, 0, sizeof(head));
	head.active = 1;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0))
	ret = tracepoint_probe_register("sys_enter", syscall_probe, NULL);
#else
	for_each_kernel_tracepoint(set_tracepoint, NULL);
	if (tp_sysenter == NULL) {
		printk(KERN_INFO "firejail: failed to find tracepoints.\n");
		return 1;
	}
	ret = tracepoint_probe_register(tp_sysenter, syscall_probe, NULL);
#endif
	if (ret) {
		printk(KERN_INFO "firejail: failed initializing syscall_enter_probe.\n");
		return 1;
	}


	if (proc_create(PROC_FILE_NAME, 0, NULL, &firejail_fops) == NULL) {
		printk(KERN_INFO "firejail: failed initializing proc file.\n");
		goto errout1;
	}

	if (proc_create(PROC_UPTIME, 0, NULL, &uptime_fops) == NULL) {
		printk(KERN_INFO "firejail: failed initializing proc file.\n");
		goto errout2;
	}

	setup_timer(&cleanup_timer, firejail_timeout, 0);
	mod_timer(&cleanup_timer, jiffies + HZ * CLEANUP_CNT);
	printk(KERN_INFO "firejail: module initialized, main nsproxy %p.\n", main_ns);

	return 0;
	
errout2:
	remove_proc_entry(PROC_FILE_NAME, NULL);
errout1:
printk(KERN_INFO "firejail: here %d\n", __LINE__);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0))
	tracepoint_probe_unregister("sys_enter", syscall_probe, NULL);
#else
	tracepoint_probe_unregister(tp_sysenter, syscall_probe, NULL);
#endif
	return 1;
}


static void __exit cleanup_main(void) {
	printk(KERN_INFO "firejail: removing module.\n");
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0))
	tracepoint_probe_unregister("sys_enter", syscall_probe, NULL);
#else
	tracepoint_probe_unregister(tp_sysenter, syscall_probe, NULL);
#endif
	remove_proc_entry(PROC_FILE_NAME, NULL);
	remove_proc_entry(PROC_UPTIME, NULL);
	del_timer_sync(&cleanup_timer);
}


module_init(init_main);
module_exit(cleanup_main);

#if 0
struct net   -    http://lxr.free-electrons.com/source/include/net/net_namespace.h#L44
struct net *get_net_ns_by_pid(pid_t pid);


struct net_device { -  http://lxr.free-electrons.com/source/include/linux/netdevice.h#L1492
...
#ifdef CONFIG_NET_NS
	struct net              *nd_net;
#endif
...
}
// use this to extract struct net *, it takes care of #ifdef CONFIG_NET_NS
struct net *dev_net(const struct net_device *dev)

#endif
