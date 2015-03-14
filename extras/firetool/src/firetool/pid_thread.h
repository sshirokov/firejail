#ifndef PID_THREAD_H
#define PID_THREAD_H
#include <QImage>
#include <QMutex>
#include <QQueue>
#include <QThread>
#include <QWaitCondition>
#include <QStringList>

class PidThread : public QThread
{
Q_OBJECT

public:
	PidThread();
	~PidThread();

signals:
	void cycleReady(bool update);

protected:
	void run();
private:
	bool ending_;		
};

#endif