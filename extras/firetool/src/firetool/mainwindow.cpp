#include <QtGui>
//#include <QPixmap>
#include "mainwindow.h"
#include "../common/utils.h"
#include "applications.h"
#include "pid_thread.h"
#include "stats_dialog.h"

MainWindow::MainWindow(QWidget *parent): QWidget(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint) {
	active_index_ = -1;
	animation_id_ = 0;
	stats_ = new StatsDialog();
	connect(this, SIGNAL(cycleReadySignal(bool)), stats_, SLOT(cycleReady(bool)));

	
	if (!which("firejail")) {
		QMessageBox::warning(this, tr("Firejail Tools"),
			tr("<br/><b>Firejail</b> sandbox not found.<br/><br/><br/>"));
		exit(1);
	}
	applications_init();
	createTrayActions();
	createLocalActions();
	thread_ = new PidThread();
	connect(thread_, SIGNAL(cycleReady(bool)), this, SLOT(cycleReady(bool)));

	setContextMenuPolicy(Qt::ActionsContextMenu);
	setToolTip(tr("Double click on an icon to open an application.\n"
		"Drag the launcher with the left mouse button.\n"
		"Use the right mouse button to open a context menu."));
	setWindowTitle(tr("Firejail Tools"));
}

void MainWindow::cycleReady(bool update) {
	if (stats_->isVisible()) {
		emit cycleReadySignal(update);
	}
}

void MainWindow::edit() {
	printf("edit not impelemnted\n");
}


void MainWindow::run() {
	int index = active_index_;
	if (index != -1) {
		if (index == 0) {
			stats_->reset();
			stats_->show();
		}
		else
			system(applist[index].exec_.toStdString().c_str());
	}
		
	animation_id_ = AFRAMES;
	QTimer::singleShot(0, this, SLOT(update()));
}


void MainWindow::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		int x = event->pos().x();
		int y = event->pos().y();
		if (x >= MARGIN * 2 + 54 && x <= MARGIN * 2 + 68 &&
			   y >= 4 && y <= 15) {
			hide();
			stats_->hide();
		}
		dragPosition = event->globalPos() - frameGeometry().topLeft();
		event->accept();
		active_index_ = -1;
	}

	else if (event->button() == Qt::RightButton) {
		active_index_ = applications_get_index(event->pos());
		if (active_index_ == -1) {
			qedit_->setDisabled(true);
			qrun_->setDisabled(true);
		}
		else {
			qedit_->setDisabled(false);
			qrun_->setDisabled(false);
		}
	}
}


void MainWindow::mouseMoveEvent(QMouseEvent *event) {
	if (event->buttons() & Qt::LeftButton) {
		move(event->globalPos() - dragPosition);
		event->accept();
	}
}


void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		QPoint pos = event->pos();
		int index = applications_get_index(pos);
		if (index != -1) {
			if (index == 0) {
				stats_->reset();
				stats_->show();
			}
			else
				system(applist[index].exec_.toStdString().c_str());
			event->accept();
			animation_id_ = AFRAMES;
			active_index_ = index;
			QTimer::singleShot(0, this, SLOT(update()));
		}
	}
}


void MainWindow::paintEvent(QPaintEvent *) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	QSize sz = sizeHint();
	painter.fillRect(QRect(0, 0, sz.width(), sz.height()), QBrush(QColor(255, 20, 20)));


	int nelem = applist.count();

	for (int i = 0; i < nelem; i++) {
		QIcon icon(applist[i].icon_);	
		int sz = 64 ;
		if (active_index_ == i) {
			int id = animation_id_;
			if (id == AFRAMES)
				id = AFRAMES / 2;
			sz -= id * 3;
		}
		QPixmap pixmap = icon.pixmap(QSize(sz, sz), QIcon::Normal, QIcon::On);
		painter.drawPixmap(QPoint(MARGIN * 2 + (64 - sz) / 2, MARGIN *2 + i * 64 + TOP + (64 - sz) / 2), pixmap);
	}

	// vertical bars
	QPen pen1(Qt::black);
	painter.setPen(pen1);
	painter.drawLine(MARGIN * 2 + 20, MARGIN * 2 + TOP, MARGIN * 2 + 20, MARGIN * 2 + nelem * 64 + TOP);
	painter.drawLine(MARGIN * 2 + 64 - 20, MARGIN * 2 + TOP, MARGIN * 2 + 64 - 20, MARGIN * 2 + nelem * 64 + TOP);

	// horizontal bars
	int i;
	for (i = 0; i < nelem - 1; i++) {
		painter.drawLine(MARGIN * 2, MARGIN * 2 + 64 * (i + 1) - 1 + TOP,
			MARGIN * 2 + 64, MARGIN * 2 + 64 * (i + 1) - 1 + TOP);

	}

	// close button
	painter.fillRect(QRect(MARGIN * 2 + 56, 8, 12, 3), QBrush(Qt::white));
	

	painter.setFont(QFont("Sans", TOP, QFont::Normal));
	QPen pen2(Qt::white);
	painter.setPen(pen2);
	painter.drawText(MARGIN * 2, TOP + MARGIN / 2, "Firejail");

	if (animation_id_ > 0) {
		animation_id_--;
		QTimer::singleShot(ADELAY, this, SLOT(update()));
	}
	
	
}


void MainWindow::resizeEvent(QResizeEvent * /* event */) {
	int nelem = applist.count();
	
	// margins
	QRegion m1(0, 0, 64 + MARGIN * 4, TOP + nelem * 64 + MARGIN * 4);
	QRegion m2(MARGIN, MARGIN + TOP, 64 + MARGIN * 2, nelem * 64 + MARGIN * 2);
	QRegion m3(MARGIN * 2, MARGIN *2 + TOP, 64, nelem * 64);
	
	QRegion all = m1.subtracted(m2);
	all = all.united(m3);
	
	setMask(all);
}


QSize MainWindow::sizeHint() const
{
	int nelem = applist.count();
	return QSize(64 + MARGIN * 4, nelem * 64 + MARGIN * 4 + TOP);
}


bool MainWindow::event(QEvent *event) {
	if (event->type() == QEvent::ToolTip) {
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
		
		int index = applications_get_index(helpEvent->pos());
		if (index == -1) {
			int x = helpEvent->pos().x();
			int y = helpEvent->pos().y();
			
			if (x >= MARGIN * 2 + 54 && x <= MARGIN * 2 + 68 &&
			   y >= 4 && y <= 15) {
			   	QToolTip::showText(helpEvent->globalPos(), QString("minimize"));
			   	return true;
			}
			else
				QToolTip::hideText();
		}
		else {
			QToolTip::showText(helpEvent->globalPos(), applist[index].name_);
			return true;
		}
	}
	return QWidget::event(event);
}

void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason reason) {
	if (reason == QSystemTrayIcon::Context)
		return;
	if (reason == QSystemTrayIcon::DoubleClick)
		return;
	if (reason == QSystemTrayIcon::MiddleClick)
		return;

	if (isVisible()) {
		hide();
		stats_->hide();
	}
	else {
		show();
		stats_->hide();
	}
}


void MainWindow::createTrayActions() {
	minimizeAction = new QAction(tr("Mi&nimize"), this);
	connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
	connect(minimizeAction, SIGNAL(triggered()), stats_, SLOT(hide()));

	restoreAction = new QAction(tr("&Restore"), this);
	connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
//	connect(restoreAction, SIGNAL(triggered()), stats_, SLOT(show()));

	quitAction = new QAction(tr("&Quit"), this);
	connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void MainWindow::createLocalActions() {
	QAction *qminimize = new QAction(tr("&Minimize"), this);
	connect(qminimize, SIGNAL(triggered()), this, SLOT(hide()));
	connect(qminimize, SIGNAL(triggered()), stats_, SLOT(hide()));
	addAction(qminimize);

	QAction *separator1 = new QAction(this);
	separator1->setSeparator(true);
	addAction(separator1);

	qrun_ = new QAction(tr("&Run"), this);
	connect(qrun_, SIGNAL(triggered()), this, SLOT(run()));
	addAction(qrun_);

	qedit_ = new QAction(tr("&Edit"), this);
	connect(qedit_, SIGNAL(triggered()), this, SLOT(edit()));
	addAction(qedit_);

	QAction *separator2 = new QAction(this);
	separator2->setSeparator(true);
	addAction(separator2);

	QAction *qquit = new QAction(tr("&Quit"), this);
	//	qa->setShortcut(tr("Ctrl+Q"));
	connect(qquit, SIGNAL(triggered()), qApp, SLOT(quit()));
	addAction(qquit);
}
