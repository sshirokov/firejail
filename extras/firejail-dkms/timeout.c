#include "firejail.h"

static struct task_struct *find_pid(pid_t pid) {
	struct task_struct *p;

	for_each_process(p) {
		if (p->pid == pid) {
			return p;
		}
	}
	
	return NULL;
}

//*****************************************************
// 1 second timer
//*****************************************************
void firejail_timeout(unsigned long dummy) {
	NsRule *ptr;
	
	// walk the rules list and disable rules if no process left in the namespace
	ptr = head.next;
	while (ptr) {
		if (ptr->active && find_pid(ptr->sandbox_pid) == NULL) {
			printk(KERN_INFO "firejail[%u]: cleanup sandbox.\n", ptr->sandbox_pid);
			ptr->active = 0;
		}
		ptr = ptr->next;
	}
	
	// restart timer
	mod_timer(&cleanup_timer, jiffies + HZ * CLEANUP_CNT);
}
