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

public slots:
	void cycleReady();
	void anchorClicked(const QUrl & link);

private:
	QString header();
	void kernelSecuritySettings();
	void updateTop();	
	void updatePid();	
	void updateTree();	
	void updateSeccomp();	
	void updateDns();	

private:
	QTextBrowser *procView_;
	bool updated_;
#define MODE_TOP 0
#define MODE_PID 1	
#define MODE_TREE 2
#define MODE_SECCOMP 3
#define MODE_DNS 4
	int mode_;
	int pid_;	// pid value for mode 1
	int pid_seccomp_;
	QString pid_caps_;
};


#endif