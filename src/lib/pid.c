/*
 * Copyright (C) 2014, 2015 netblue30 (netblue30@yahoo.com)
 *
 * This file is part of firejail project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "../include/common.h"
#include "../include/pid.h"
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <dirent.h>
       
#define PIDS_BUFLEN 4096
//Process pids[MAX_PIDS];
Process *pids = NULL;
int MAX_PIDS=32769;
#define PIDS_BUFLEN 4096

// get the memory associated with this pid
void pid_getmem(unsigned pid, unsigned *rss, unsigned *shared) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/statm", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return;
	}
	free(file);
	
	unsigned a, b, c;
	if (3 != fscanf(fp, "%u %u %u", &a, &b, &c)) {
		fclose(fp);
		return;
	}
	*rss += b;
	*shared += c;
	fclose(fp);
}


void pid_get_cpu_time(unsigned pid, unsigned *utime, unsigned *stime) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/stat", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return;
	}
	free(file);
	
	char line[PIDS_BUFLEN];
	if (fgets(line, PIDS_BUFLEN - 1, fp)) {
		char *ptr = line;
		// jump 13 fields
		int i;
		for (i = 0; i < 13; i++) {
			while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				goto myexit;
			ptr++;
		}
		if (2 != sscanf(ptr, "%u %u", utime, stime))
			goto myexit;
	}

myexit:	
	fclose(fp);
}

unsigned long long pid_get_start_time(unsigned pid) {
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/stat", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return 0;
	}
	free(file);
	
	char line[PIDS_BUFLEN];
	unsigned long long retval = 0;
	if (fgets(line, PIDS_BUFLEN - 1, fp)) {
		char *ptr = line;
		// jump 21 fields
		int i;
		for (i = 0; i < 21; i++) {
			while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
				ptr++;
			if (*ptr == '\0')
				goto myexit;
			ptr++;
		}
		if (1 != sscanf(ptr, "%llu", &retval))
			goto myexit;
	}
	
myexit:
	fclose(fp);
	return retval;
}

char *pid_get_user_name(uid_t uid) {
	struct passwd *pw = getpwuid(uid);
	if (pw)
		return strdup(pw->pw_name);
	return NULL;
}

uid_t pid_get_uid(pid_t pid) {
	uid_t rv = 0;
	
	// open stat file
	char *file;
	if (asprintf(&file, "/proc/%u/status", pid) == -1) {
		perror("asprintf");
		exit(1);
	}
	FILE *fp = fopen(file, "r");
	if (!fp) {
		free(file);
		return 0;
	}

	// look for firejail executable name
	char buf[PIDS_BUFLEN];
	while (fgets(buf, PIDS_BUFLEN - 1, fp)) {
		if (strncmp(buf, "Uid:", 4) == 0) {
			char *ptr = buf + 5;
			while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
				ptr++;
			}
			if (*ptr == '\0')
				goto doexit;
				
			rv = atoi(ptr);
			break; // break regardless!
		}
	}
doexit:	
	fclose(fp);
	free(file);
	return rv;
}

static void print_elem(unsigned index, int nowrap) {
	// get terminal size
	struct winsize sz;
	int col = 0;
	if (isatty(STDIN_FILENO)) {
		if (!ioctl(0, TIOCGWINSZ, &sz))
			col  = sz.ws_col;
	}

	// indent
	char indent[(pids[index].level - 1) * 2 + 1];
	memset(indent, ' ', sizeof(indent));
	indent[(pids[index].level - 1) * 2] = '\0';

	// get data
	uid_t uid = pids[index].uid;
	char *cmd = pid_proc_cmdline(index);
	char *user = pid_get_user_name(uid);
	char *allocated = user;
	if (user ==NULL)
		user = "";
	if (cmd) {
		if (col < 4 || nowrap) 
			printf("%s%u:%s:%s\n", indent, index, user, cmd);
		else {
			char *out;
			if (asprintf(&out, "%s%u:%s:%s\n", indent, index, user, cmd) == -1)
				errExit("asprintf");
			int len = strlen(out);
			if (len > col) {
				out[col] = '\0';
				out[col - 1] = '\n';
			}
			printf("%s", out);
			free(out);
		}
				
		free(cmd);
	}
	else {
		if (pids[index].zombie)
			printf("%s%u: (zombie)\n", indent, index);
		else
			printf("%s%u:\n", indent, index);
	}
	if (allocated)
		free(allocated);
}

// recursivity!!!
void pid_print_tree(unsigned index, unsigned parent, int nowrap) {
	print_elem(index, nowrap);
	
	int i;
	for (i = index + 1; i < MAX_PIDS; i++) {
		if (pids[i].parent == index)
			pid_print_tree(i, index, nowrap);
	}
}

void pid_print_list(unsigned index, int nowrap) {
	print_elem(index, nowrap);
}

// recursivity!!!
void pid_store_cpu(unsigned index, unsigned parent, unsigned *utime, unsigned *stime) {
	if (pids[index].level == 1) {
		*utime = 0;
		*stime = 0;
	}
	
	unsigned utmp = 0;
	unsigned stmp = 0;
	pid_get_cpu_time(index, &utmp, &stmp);
	*utime += utmp;
	*stime += stmp;
	
	int i;
	for (i = index + 1; i < MAX_PIDS; i++) {
		if (pids[i].parent == index)
			pid_store_cpu(i, index, utime, stime);
	}

	if (pids[index].level == 1) {
		pids[index].utime = *utime;
		pids[index].stime = *stime;
	}
}

// mon_pid: pid of sandbox to be monitored, 0 if all sandboxes are included
void pid_read(pid_t mon_pid) {
	if (pids == NULL) {
		FILE *fp = fopen("/proc/sys/kernel/pid_max", "r");
		if (fp) {
			int val;
			if (fscanf(fp, "%d", &val) == 1) {
				if (val >= MAX_PIDS)
					MAX_PIDS = val + 1;
			}
			fclose(fp);
		}
		pids = malloc(sizeof(Process) * MAX_PIDS);
		if (pids == NULL)
			errExit("malloc");
	}
	memset(pids, 0, sizeof(Process) * MAX_PIDS);
	pid_t mypid = getpid();

	DIR *dir;
	if (!(dir = opendir("/proc"))) {
		// sleep 2 seconds and try again
		sleep(2);
		if (!(dir = opendir("/proc"))) {
			fprintf(stderr, "Error: cannot open /proc directory\n");
			exit(1);
		}
	}
	
	pid_t child = -1;
	struct dirent *entry;
	char *end;
	while (child < 0 && (entry = readdir(dir))) {
		pid_t pid = strtol(entry->d_name, &end, 10);
		pid %= MAX_PIDS;
		if (end == entry->d_name || *end)
			continue;
		if (pid == mypid)
			continue;
		
		// open stat file
		char *file;
		if (asprintf(&file, "/proc/%u/status", pid) == -1) {
			perror("asprintf");
			exit(1);
		}
		FILE *fp = fopen(file, "r");
		if (!fp) {
			free(file);
			continue;
		}

		// look for firejail executable name
		char buf[PIDS_BUFLEN];
		while (fgets(buf, PIDS_BUFLEN - 1, fp)) {
			if (strncmp(buf, "Name:", 5) == 0) {
				char *ptr = buf + 5;
				while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
					ptr++;
				}
				if (*ptr == '\0') {
					fprintf(stderr, "Error: cannot read /proc file\n");
					exit(1);
				}

				if (mon_pid == 0 && strncmp(ptr, "firejail", 8) == 0) {
					pids[pid].level = 1;
				}
				else if (mon_pid == pid && strncmp(ptr, "firejail", 8) == 0) {
					pids[pid].level = 1;
				}
//				else if (mon_pid == 0 && strncmp(ptr, "lxc-execute", 11) == 0) {
//					pids[pid].level = 1;
//				}
//				else if (mon_pid == pid && strncmp(ptr, "lxc-execute", 11) == 0) {
//					pids[pid].level = 1;
//				}
				else
					pids[pid].level = -1;
			}
			if (strncmp(buf, "State:", 6) == 0) {
				if (strstr(buf, "(zombie)"))
					pids[pid].zombie = 1;
			}
			else if (strncmp(buf, "PPid:", 5) == 0) {
				char *ptr = buf + 5;
				while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
					ptr++;
				}
				if (*ptr == '\0') {
					fprintf(stderr, "Error: cannot read /proc file\n");
					exit(1);
				}
				unsigned parent = atoi(ptr);
				parent %= MAX_PIDS;
				if (pids[parent].level > 0) {
					pids[pid].level = pids[parent].level + 1;
					pids[pid].parent = parent;
				}
			}
			else if (strncmp(buf, "Uid:", 4) == 0) {
				if (pids[pid].level > 0) {
					char *ptr = buf + 5;
					while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t')) {
						ptr++;
					}
					if (*ptr == '\0') {
						fprintf(stderr, "Error: cannot read /proc file\n");
						exit(1);
					}
					pids[pid].uid = atoi(ptr);
				}
				break;
			}
		}
		fclose(fp);
		free(file);
	}
	closedir(dir);
}
