#ifndef APPLICATIONS_H
#define APPLICATIONS_H
#include <QList>
#include <QString>
#include <QIcon>

#define TOP 10
#define MARGIN 5
#define AFRAMES 6		// animation frames
#define ADELAY 20		// animation delay

struct Application {
	QString name_;
	QString exec_;
	QString icon_;
	Application(const char *name, const char *exec, const char *icon):
		name_(name), exec_(exec), icon_(icon) {
		
		exec_ += " &";
	};
};

extern QList<Application> applist;
void applications_init();
int applications_get_index(QPoint pos);
QIcon getIcon(QString name);

#endif