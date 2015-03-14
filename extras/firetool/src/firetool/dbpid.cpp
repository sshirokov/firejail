#include "dbpid.h"
#include <assert.h>
#include <string.h>

DbPid::DbPid(pid_t pid): next_(0), pid_(pid), cmd_(0) {
	memset(data_, 0, sizeof(data_));
}

DbPid::~DbPid() {
	if (cmd_)
		delete cmd_;
		
	if (next_)
		delete next_;
}

void DbPid::setCmd(const char *cmd) {
	if (cmd == 0) {
		if (cmd_)
			delete cmd_;
		cmd_ = 0;
	}
	else {
		if (cmd_) {
			if (strcmp(cmd, cmd)) {
				delete cmd_;
				cmd_ = 0;
			}
		}	
		
		if (!cmd_) {
			cmd_ = new char[strlen(cmd) + 1];
			strcpy(cmd_, cmd);
		}
	}
}

void DbPid::add(DbPid *dbpid) {
	assert(dbpid);
	if (!next_) {
		next_ = dbpid;
		return;
	}
	
	next_->add(dbpid);
}

void DbPid::remove(DbPid *dbpid) {
	assert(dbpid);
	if (next_ == dbpid) {
		next_ = dbpid->next_;
		return;
	}
	
	if (next_)
		next_->remove(dbpid);
}
	
DbPid *DbPid::find(pid_t pid) {
	if (pid_ == pid) {
		return this;
	}
	
	if (next_) {
		return next_->find(pid);
	}
	
	return 0;	
}	

void DbPid::dbgprint() {
	printf("***\n");
	printf("*** PID %d, %s\n", pid_, cmd_);
	printf("***\n");
	
	for (int i = 0; i < MAXCYCLE; i++)
		data_[i].dbgprint(i);
	
	if (next_)
		next_->dbgprint();
}
	

