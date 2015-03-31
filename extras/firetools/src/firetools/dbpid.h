#ifndef DBPID_H
#define DBPID_H
#include <sys/types.h>
#include <unistd.h>
#include "dbstorage.h"

class DbPid {
public:
	static const int MAXCYCLE = 48;
	DbStorage data_[MAXCYCLE];

	DbPid(pid_t pid);
	~DbPid();
	void setCmd(const char *cmd);
	const char *getCmd() {
		return cmd_;
	}

	void add(DbPid *dbpid);
	void remove(DbPid *dbpid);
	DbPid *find(pid_t pid);
	void dbgprint();
	DbPid *getNext() {
		return next_;
	}
	void resetNext() {
		next_ = 0;
	}
	pid_t getPid() {
		return pid_;
	}

private:	
	DbPid *next_;
	pid_t pid_;
	char *cmd_;
};

#endif