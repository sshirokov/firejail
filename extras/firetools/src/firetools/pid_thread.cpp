#include <QtGui>
#include <QElapsedTimer>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "pid_thread.h"
#include "../common/pid.h"
#include <db.h>

bool data_ready = false;


PidThread::PidThread(): ending_(false) {
	start();
}


PidThread::~PidThread() {
	ending_ = true;
}

static void store(int pid, int interval, int clocktick) {
	assert(pid < MAX_PIDS);
	if (pids[pid].cmd == 0)	{ // process shutting down
		// remove dbpid
		DbPid *dbpid = Db::instance().removePid(pid);
		if (dbpid)
			delete dbpid;
		return;
	}
		
	DbPid *dbpid = Db::instance().findPid(pid);
	
	if (!dbpid) {
		dbpid = Db::instance().newPid(pid);
	}
	assert(dbpid);
	if (!dbpid)
		return;

	int cycle = Db::instance().getCycle();
	
	// store the data in database
	DbStorage *st = &dbpid->data_[cycle];
	st->cpu_ = (float) ((pids[pid].utime + pids[pid].stime) * 100) / (interval * clocktick);
	st->rss_ = pids[pid].rss;
	st->shared_ =  pids[pid].shared;
	st->rx_ = ((float) pids[pid].rx) /( interval * 1000);
	st->tx_ = ((float) pids[pid].tx) /( interval * 1000);
	if (strstr(pids[pid].cmd, "--net"))
		st->network_disabled_ = false;
	else
		st->network_disabled_ = true;
	
	dbpid->setCmd(pids[pid].cmd);
}

void PidThread::run() {
	// memory page size clicks per second
	int pgsz = getpagesize();
	int clocktick = sysconf(_SC_CLK_TCK);

	
	
	while (1) {
		if (ending_)
			break;

		// initialize process table
		pid_read(0);
		
		// start cpu and network measurements
		unsigned utime;
		unsigned stime;
		unsigned long long rx;
		unsigned long long tx;
		for (int i = 0; i < MAX_PIDS; i++) {
			if (pids[i].level == 1) {
				// cpu
				pid_get_cpu_sandbox(i, &utime, &stime);
				pids[i].utime = utime;
				pids[i].stime = stime;

				pid_get_netstats_sandbox(i, &rx, &tx);
				pids[i].rx = rx;
				pids[i].tx = tx;
			}
		}
		
		// sleep 5 seconds
		msleep(4500);
		data_ready = false;
		msleep(500);
		
		// start a new database cycle
		Db::instance().newCycle();
		
		// read the cpu time again, memory
		for (int i = 0; i < MAX_PIDS; i++) {
			if (pids[i].level == 1) {
				if (pids[i].zombie)
					continue;
				pids[i].cmd = pid_proc_cmdline(i);
				
				// cpu time
				pid_get_cpu_sandbox(i, &utime, &stime);
				if (pids[i].utime <= utime)
					pids[i].utime = utime - pids[i].utime;
				else
					pids[i].utime = 0;
					
				if (pids[i].stime <= stime)
					pids[i].stime = stime - pids[i].stime;
				else
					pids[i].stime = 0;
				
				// memory
				unsigned rss;
				unsigned shared;
				pid_get_mem_sandbox(i, &rss, &shared);
				pids[i].rss = rss * pgsz / 1024;
				pids[i].shared = shared * pgsz / 1024;
				
				// network
				if (pids[i].cmd && strstr(pids[i].cmd, "--net") != 0) {
					pid_get_netstats_sandbox(i, &rx, &tx);
					if (rx >= pids[i].rx)
						pids[i].rx = rx - pids[i].rx;
					else
						pids[i].rx = 0;
					
					if (tx > pids[i].tx)
						pids[i].tx = tx - pids[i].tx;
					else
						pids[i].tx = 0;
				}
				else {
					pids[i].rx = 0;
					pids[i].tx = 0;
				}
				
				store(i, 5, clocktick);
				
				if (pids[i].cmd)
					free(pids[i].cmd);
			}
		}
		
//		Db::instance().dbgprint();
		emit cycleReady();
		data_ready = true;

	}
}