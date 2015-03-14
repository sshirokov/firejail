#include <stdio.h>
#include <assert.h>
#include "db.h"

Db::Db(): cycle_(DbPid::MAXCYCLE - 1), pidlist_(0) {
	printf("hello world!\n");
}

void Db::newCycle() {
	if (++cycle_ >= DbPid::MAXCYCLE)
		cycle_ = 0;
}


DbPid *Db::findPid(pid_t pid) {
	if (!pidlist_) {
//printf("find in db.cpp, return null\n");	
		return 0;
	}
	
//printf("find in db.cpp, walk the list\n");	
	return pidlist_->find(pid);
}

DbPid *Db::newPid(pid_t pid) {
	assert(findPid(pid) == 0);
	
	DbPid *newpid = new DbPid(pid);
	if (!pidlist_)
		pidlist_ = newpid;
	else
		pidlist_->add(newpid);
		
	return newpid;
}

DbPid *Db::removePid(pid_t pid) {
	// find dbpid
	DbPid *dbpid = findPid(pid);
	if (!dbpid)
		return 0;
	
	// remove first element
	if (dbpid == pidlist_)
		pidlist_ = dbpid->getNext();
	else
		pidlist_->remove(dbpid);
	
	dbpid->resetNext();
	return dbpid;
}

void Db::dbgprint() {
	if (pidlist_)
		pidlist_->dbgprint();
}
