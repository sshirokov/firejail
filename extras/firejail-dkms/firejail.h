#ifndef FIREJAIL_LKM_H
#define FIREJAIL_LKM_H
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/proc_fs.h>
#include <linux/tracepoint.h>
#include <asm/syscall.h>
#include <linux/un.h>

// one second timer
extern struct timer_list cleanup_timer;
#define CLEANUP_CNT 5 	// clean the list every 60 seconds

// rules
typedef struct nsrule_t {
	struct nsrule_t *next;	// linked list
	
	struct nsproxy *nsproxy;		// namespace proxy pointer
	struct net *net;			// net structure for current sandbox
	pid_t sandbox_pid;			// pid of the control process for the sandbox
	struct timespec real_start_time;	// time when the sandbox was registered

	unsigned active : 1;	// rule active flag; inactive rules are reused or deallocated
	unsigned net_active : 1;	// network shaping is active
	
	// unix sockets connections that will kill the process
#define MAX_UNIX_PATH 8
	char *unix_path[MAX_UNIX_PATH];
	int unix_path_len[MAX_UNIX_PATH];
} NsRule;
extern NsRule head;

static inline NsRule *find_sandbox(struct nsproxy *nsproxy) {
	NsRule *ptr;

	ptr = head.next;
	while (ptr) {
		if (ptr->nsproxy == nsproxy)
			break;
		ptr = ptr->next;
	}
	
	if (ptr == NULL || ptr->active == 0)
		return NULL;
	return ptr;
}

static inline NsRule *find_sandbox_pid(pid_t pid) {
	NsRule *ptr;

	ptr = head.next;
	while (ptr) {
		if (ptr->sandbox_pid == pid)
			break;
		ptr = ptr->next;
	}
	if (ptr == NULL || ptr->active == 0)
		return NULL;
	return ptr;
}
	
static inline NsRule *find_sandbox_net(struct net *net) {
	NsRule *ptr;

	ptr = head.next;
	while (ptr) {
		if (ptr->net == net)
			break;
		ptr = ptr->next;
	}
	if (ptr == NULL || ptr->active == 0 || ptr->net_active == 0)
		return NULL;
	return ptr;
}	

// timeout.c
void firejail_timeout(unsigned long dummy);

// split.c
#define CMD_MAX_SIZE 2048
#define SARG_MAX 30
extern int sargc;
extern char *sargv[SARG_MAX];
extern void split_command(char *cmd);

// syscall.c
void syscall_probe_open(struct pt_regs *regs, long id);
void syscall_probe_socket(struct pt_regs *regs, long id);
void syscall_probe_connect(struct pt_regs *regs, long id, NsRule *rule);

// uptime.c
extern const struct file_operations uptime_fops;

// utils.c
struct task_struct *get_sandbox(void);
NsRule *find_or_create_rule(void);

#endif