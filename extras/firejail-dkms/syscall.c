#include "firejail.h"
#include <linux/in.h>
#include <linux/inet.h>
#include <net/sock.h>

void syscall_probe_connect(struct pt_regs *regs, long id, NsRule *rule) {
	unsigned long val;
	int fd;
	struct sockaddr __user *usraddr;
	struct sockaddr_storage addr;
	memset(&addr, 0, sizeof(addr));

	syscall_get_arguments(current, regs, 0, 1, &val);
	fd = (int) val;
	if (fd > 0) {
		unsigned len = 0;
		syscall_get_arguments(current, regs, 1, 1, &val);
		usraddr = (struct sockaddr __user *)val;

		syscall_get_arguments(current, regs, 2, 1, &val);
		len = (unsigned) val;

		if (usraddr && len) {
			sa_family_t family ;

			if (__copy_from_user_inatomic((char *) &addr, (const char *)usraddr, len) < 0) {
				printk(KERN_INFO "firejail %u: ERROR %d\n", current->pid, __LINE__);
			}

			family = ((struct sockaddr *) &addr)->sa_family;
			if (family == AF_INET) {
//				struct sockaddr_in *saddr4 = (struct sockaddr_in *) &addr;
//				char *a = (char *) &saddr4->sin_addr.s_addr;
//				printk(KERN_INFO "firejail[%u]: connect AF_INET %u.%u.%u.%u\n", rule->sandbox_pid,
//					a[0] & 0xff, a[1] & 0xff, a[2] & 0xff, a[3] & 0xff);
			}

			else if (family == AF_INET6) {
//				printk(KERN_INFO "firejail[%u]: connect AF_INET6\n", rule->sandbox_pid);
			}

			else if (family == AF_UNIX) {
				int i;
				struct sockaddr_un *a = (struct sockaddr_un *) &addr;
				if (a->sun_path[0]) {
					printk(KERN_INFO "firejail[%u]: connect AF_UNIX #%s#\n", rule->sandbox_pid, a->sun_path);
				}
				else {
					printk(KERN_INFO "firejail[%u]: connect AF_UNIX %s\n", rule->sandbox_pid, a->sun_path + 1);
				}

				for (i = 0; i < MAX_UNIX_PATH; i++) {
					if (rule->unix_path_len[i] && rule->unix_path[i]) {
						if ((a->sun_path[0] && strncmp(a->sun_path, rule->unix_path[i], rule->unix_path_len[i]) == 0) ||
						(a->sun_path[0] == '\0' && strncmp(a->sun_path + 1, rule->unix_path[i], rule->unix_path_len[i]) == 0)) {

							printk(KERN_INFO "firejail[%u]: process %u killed, rule \"no connect unix %s\"\n",
								rule->sandbox_pid, current->pid, rule->unix_path[i]);
							do_exit(SIGSYS);
						}
					}
					else
						break;
				}
			} // end AF_UNIX
		}
	}
}
