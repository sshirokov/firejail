#include "firejail.h"

static struct task_struct *find_pid(pid_t pid) {
	struct task_struct *p;

	rcu_read_lock();
	for_each_process(p) {
		if (p->pid == pid) {
			rcu_read_unlock();
			return p;
		}
	}
	rcu_read_unlock();
	
	return NULL;
}

//*****************************************************
// 1 second timer
//*****************************************************
void firejail_timeout(unsigned long dummy) {
	NsRule *ptr = head.next;
	
	// walk the rules list and disable rules if no process left in the namespace
	while (ptr) {
		if (ptr->active && find_pid(ptr->sandbox_pid) == NULL) {
			printk(KERN_INFO "firejail[%u]: cleanup sandbox.\n", ptr->sandbox_pid);
			ptr->active = 0;
		}
		ptr = ptr->next;
	}
	
	// restart timer
	mod_timer(&rate_timer, jiffies + HZ * 10);
}
