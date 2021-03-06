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
#include "firemon.h"
#define MAXBUF 4098

static void print_cgroup(int pid) {
	char *file;
	if (asprintf(&file, "/proc/%d/cgroup", pid) == -1) {
		errExit("asprintf");
		exit(1);
	}

	FILE *fp = fopen(file, "r");
	if (!fp) {
		printf("  Error: cannot open %s\n", file);
		free(file);
		return;
	}
	
	char buf[MAXBUF];
	if (fgets(buf, MAXBUF, fp)) {
		printf("  %s", buf);
		fflush(0);
	}

	fclose(fp);
	free(file);
}
			
void cgroup(pid_t pid) {
	if (getuid() == 0)
		firemon_drop_privs();
	
	pid_read(pid);
	
	// print processes
	int i;
	for (i = 0; i < MAX_PIDS; i++) {
		if (pids[i].level == 1) {
			pid_print_list(i, 0);
			int child = find_child(i);
			if (child != -1)
				print_cgroup(child);
		}
	}
}

