#ifndef STATS_DIALOG_H
#define STATS_DIALOG_H
#include <QWidget>
#include <QDialog>
//class QTextEdit;
class QTextBrowser;
class QUrl;

class StatsDialog: public QDialog {
Q_OBJECT

public:
	StatsDialog();
	void reset() {
		updated_ = false;
	}

public slots:
	void cycleReady(bool update);
	void anchorClicked(const QUrl & link);

private:
	QString header();
	void kernelSecuritySettings();
	void updateTop();	
	void updatePid();	
	void updateTree();	
	void updateSeccomp();	

private:
	QTextBrowser *procView_;
	bool updated_;
#define MODE_TOP 0
#define MODE_PID 1	
#define MODE_TREE 2
#define MODE_SECCOMP 3
	int mode_;
	int pid_;	// pid value for mode 1
	int pid_seccomp_;
	QString pid_caps_;
};


#endif