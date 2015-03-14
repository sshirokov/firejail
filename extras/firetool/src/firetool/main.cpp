#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>

#include <stdio.h>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
	// initialize resources
	Q_INIT_RESOURCE(firetool);

	QApplication app(argc, argv);
	MainWindow fc;
	fc.show();
	
	QSystemTrayIcon icon(QIcon(":resources/firetool.png"));
	icon.show();	
	QMenu *trayIconMenu = new QMenu(&fc);
	trayIconMenu->addAction(fc.minimizeAction);
	trayIconMenu->addAction(fc.restoreAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(fc.quitAction);
	icon.setContextMenu(trayIconMenu);

	icon.connect(&icon, SIGNAL(activated(QSystemTrayIcon: :ActivationReason)), &fc, SLOT(trayActivated(QSystemTrayIcon: :ActivationReason)));
	
	// direct all errror to /dev/null to work around this qt bug:
	//      https://bugreports.qt.io/browse/QTBUG-43270
	freopen( "/dev/null", "w", stderr );
	
	return app.exec();
}

