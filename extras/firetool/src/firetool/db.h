#ifndef DB_H
#define DB_H

#include "dbpid.h"


class Db {
public:
	static Db& instance() {
		static Db myinstance;
		return myinstance;
	}
	
	void newCycle();
	int getCycle() {
		return cycle_;
	}
	DbPid *firstPid() {
		return pidlist_;
	}
	DbPid *newPid(pid_t pid);
	DbPid *findPid(pid_t pid);
	DbPid *removePid(pid_t pid);
	void dbgprint();
	
private:
	Db();
	Db(Db const&);
	void operator=(Db const&);

private:
	int cycle_;
	DbPid *pidlist_;
};

#endif
