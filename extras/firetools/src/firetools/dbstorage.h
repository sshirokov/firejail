#ifndef DBSTORAGE_H
#define DBSTORAGE_H
#include <stdio.h>
#include <assert.h>

struct DbStorage {
	float cpu_;
	float rss_;
	float shared_;
	float rx_;
	float tx_;
	bool network_disabled_;
	
	DbStorage(): cpu_(0), rss_(0), shared_(0), rx_(0), tx_(0), network_disabled_(true) {}
	
	void dbgprint(int cycle) {
		printf("%d: %.2f, %.2f, %.2f, %.2f, %.2f\n",
			cycle, cpu_, rss_, shared_, rx_, tx_);
	}
	
	float get(int id) {
		switch (id) {
			case 0:
				return cpu_;
			case 1:
				return rss_ + shared_;
			case 2:
				return rx_;
			case 3:
				return tx_;
			default:
				assert(0);
				return 0;
		}
		return 0;
	}
}; 

#endif
