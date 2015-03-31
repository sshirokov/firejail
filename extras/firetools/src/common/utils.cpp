#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#define MAXBUF (1024 * 1024) // 1MB output buffer
static char outbuf[MAXBUF + 1];

char *run_program(const char *prog) {
	// open pipe
	FILE *fp;
	fp = popen(prog, "r");
	if (fp == NULL)
		return NULL;

	// read pipe
	int len;
	char *ptr = outbuf;
	size_t size = MAXBUF;
	while ((len = fread(ptr, 1, size, fp)) > 0) {
		size -= len;
		ptr += len;
		if (size < 80)	// probably not enough room to read a new full line
			break;
	}
	*ptr = '\0';

	pclose(fp);
	return outbuf;
}

bool which(const char *prog) {
	char *cmd;
	if (asprintf(&cmd, "which %s", prog) == -1) {
		perror("asprintf");
		exit(1);
	}
	
	char *res = run_program(cmd);
	if (strstr(res, prog))
		return true;
	return false;
}
