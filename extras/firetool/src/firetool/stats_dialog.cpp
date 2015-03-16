#include <QtGui>
#include <QUrl>
#include <assert.h>
#include "stats_dialog.h"
#include "db.h"
#include "graph.h"
#include "../common/utils.h"
#include "../../firetool_config.h"


StatsDialog::StatsDialog(): QDialog(), updated_(false), mode_(MODE_TOP), pid_(0), pid_seccomp_(-1), pid_caps_(QString("")) {
	procView_ = new QTextBrowser;
	procView_->setOpenLinks(false);
	procView_->setOpenExternalLinks(false);
	procView_->setText("accumulating data...");
	
	connect(procView_,  SIGNAL(anchorClicked(const QUrl &)), this, SLOT(anchorClicked(const QUrl &)));

	QGridLayout *layout = new QGridLayout;
	layout->addWidget(procView_, 0, 0);
	setLayout(layout);
	resize(600, 500);
	setWindowTitle(tr("Firejail Tools&Stats"));
}

QString StatsDialog::header() {
	QString msg;
	if (mode_ == MODE_TOP) {
		msg += "<table><tr><td width=\"5\"></td><td>";
		msg += "<a href=\"about\">About</a>";
		msg += "</td></tr></table>";
	}
	else if (mode_ == MODE_PID) {
		msg += "<table><tr><td width=\"5\"></td><td>";
		msg += "<a href=\"top\">Home</a>";
		msg += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"about\">About</a>";
		msg += "</td></tr></table>";
	}
	else {
		msg += "<table><tr><td width=\"5\"></td><td>";
		msg += "<a href=\"back\">Back</a>";
		msg += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"top\">Home</a>";
		msg += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"about\">About</a>";
		msg += "</td></tr></table>";
	}
	
	if (mode_ == MODE_PID || mode_ == MODE_TREE) {
		msg += "<table><tr><td width=\"5\"></td><td>";
		msg += "<a href=\"shut\">Shutdown</a>";
		msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"join\">Join</a>";
		msg += " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"tree\">Process Tree</a>";
		msg += "</td></tr></table>";
	}


	return msg;
}

void StatsDialog::updateTop() {
	QString msg = header() + "<hr>";
	msg += "<table><tr><td width=\"5\"></td><td><b>Sandbox List</b></td></tr></table><br/>\n";
	msg += "<table><tr><td width=\"5\"></td><td width=\"60\">PID</td/><td width=\"60\">CPU(%)</td><td>Memory(KiB)&nbsp;&nbsp;</td><td>RX(KB/sec)&nbsp;&nbsp;</td><td>TX(KB/sec)&nbsp;&nbsp;</td><td>Command</td>\n";
	
	int cycle = Db::instance().getCycle();
	assert(cycle < DbPid::MAXCYCLE);
	DbPid *ptr = Db::instance().firstPid();
	
	
	while (ptr) {
		pid_t pid = ptr->getPid();
		const char *cmd = ptr->getCmd();
		if (cmd) {
			char *str;
			DbStorage *st = &ptr->data_[cycle];
			if (asprintf(&str, "<tr><td></td><td><a href=\"%d\">%d</a></td><td>%.02f</td><td>%d</td><td>%.02f</td><td>%.02f</td><td>%s</td></tr>",
				pid, pid, st->cpu_, (int) (st->rss_ + st->shared_),
				st->rx_, st->tx_, cmd) != -1) {
				
				msg += str;
				free(str);
			}
		}
		
		ptr = ptr->getNext();
	}
	
	msg += "</table>";		
	procView_->setHtml(msg);
}


void StatsDialog::updateTree() {
	QString msg = header();
	msg += "<hr><table><tr><td width=\"5\"></td><td>";
		
	char *str = 0;
	char *cmd;
	if (asprintf(&cmd, "firemon --tree --nowrap %d", pid_) != -1) {
		str = run_program(cmd);
		char *ptr = str;
		// htmlize!
		while (*ptr != 0) {
			if (*ptr == '\n') {
				*ptr = '\0';
				msg += QString(str) + "<br/>\n";
				ptr++;
				
				while (*ptr == ' ') {
					msg += "&nbsp;&nbsp;";
					ptr++;
				}	
				str = ptr;
				continue;
			}
			ptr++;
		}
	}		
	free(cmd);

	msg += "</td></tr></table><hr>" + header();
	procView_->setHtml(msg);
}

void StatsDialog::kernelSecuritySettings() {
	pid_seccomp_ = 0;
	pid_caps_ = QString("");
	
	char *cmd;
	if (asprintf(&cmd, "firemon --seccomp %d", pid_) == -1)
		return;
	char *str = run_program(cmd);
	if (str) {
		char *ptr = strstr(str, "Seccomp");
		if (!ptr) {
			free(cmd);
			return;
		}
		if (strstr(ptr, "2"))
			pid_seccomp_ = 1;
	}
	free(cmd);
	
	if (asprintf(&cmd, "firemon --caps %d", pid_) == -1)
		return;
	str = run_program(cmd);
	if (str) {
		char *ptr = strstr(str, "CapBnd:");
		if (!ptr) {
			free(cmd);
			return;
		}
		pid_caps_ = QString(ptr + 7);
	}
}
	
void StatsDialog::updatePid() {
	QString msg = "";

	int cycle = Db::instance().getCycle();
	assert(cycle < DbPid::MAXCYCLE);
	DbPid *ptr = Db::instance().findPid(pid_);
	if (!ptr) {
		msg += "Process not found!<br/>";
		msg += "<hr>" + header();
		procView_->setHtml(msg);
		mode_ = MODE_TOP;
		return;
	}

	const char *cmd = ptr->getCmd();
	if (!cmd) {
		msg += "Process not found!<br/>";
		msg += "<hr>" + header();
		procView_->setHtml(msg);
		mode_ = MODE_TOP;
		return;
	}
	
	DbStorage *st = &ptr->data_[cycle];

	msg += header() + "<hr>";
	msg += "<table><tr><td width=\"5\"></td><td>Command: " + QString(cmd) + "</td></tr></table><br/>";

	msg += "<table>";
	msg += QString("<tr><td width=\"5\"></td><td>PID: ") + QString::number(pid_) + "</td>";
	if (st->network_disabled_)
		msg += "<td>RX: unknown</td></tr>";
	else
		msg += QString("<td>RX: ") + QString::number(st->rx_) + " KB/sec</td></tr>";
	
	msg += QString("<tr><td></td><td>CPU: ") + QString::number(st->cpu_) + "%</td>";
	if (st->network_disabled_)
		msg += "<td>TX: unknown</td></tr>";
	else
		msg += QString("<td>TX: ") + QString::number(st->tx_) + " KB/sec</td></tr>";
	
	// init seccomp and caps
	if (pid_seccomp_ == -1)
		kernelSecuritySettings();

	msg += QString("<tr><td></td><td>Memory: ") + QString::number((int) (st->rss_ + st->shared_)) + " KiB&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>";
	msg += QString("<td>Seccomp: ");
	if (pid_seccomp_)
		msg += "enabled";
	else
		msg += "disabled";
	msg += "</td></tr>";

	msg += QString("<tr><td></td><td>RSS " + QString::number((int) st->rss_) + ", shared " + QString::number((int) st->shared_)) + "</td>";	
	msg += QString("<td>Capabilities: ") + pid_caps_ + "</td></tr>";	
	
	// graphs
	msg += "<tr></tr>";
	msg += "<tr><td></td><td>"+ graph(0, ptr, cycle) + "</td><td>" + graph(1, ptr, cycle) + "</td></tr>";
	if (st->network_disabled_ == false)
		msg += "<tr><td></td><td>"+ graph(2, ptr, cycle) + "</td><td>" + graph(3, ptr, cycle) + "</td></tr>";

	msg += QString("</table><br/>");
	msg += "<hr>" + header();
	procView_->setHtml(msg);
}

void StatsDialog::cycleReady(bool update) {
	if (isVisible()) {
		if (updated_ == true && update ==false)
			return;
		updated_ = true;

		if (mode_ == MODE_TOP)
			updateTop();
		else if (mode_ == MODE_PID)
			updatePid();
		else if (mode_ == MODE_TREE)
			updateTree();
	}
}

void StatsDialog::anchorClicked(const QUrl & link) {
	QString linkstr = link.toString();
	
	if (linkstr == "top") {
		mode_ = MODE_TOP;
		updated_ = false;
	}
	else if (linkstr == "back") {
		if (mode_ == MODE_PID)
			mode_ = MODE_TOP;
		else if (mode_ == MODE_TREE)
			mode_ = MODE_PID;
		else if (mode_ == MODE_TOP);
		else
			assert(0);
		updated_ = false;
	}
	else if (linkstr == "tree") {
		mode_ = MODE_TREE;
		updated_ = false;
	}
	else if (linkstr == "shut") {
		QMessageBox msgBox;
		msgBox.setText(QString("Are you sure you want to shutdown PID ") + QString::number(pid_) + "?\n");
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		int rv = msgBox.exec();
		if (rv == QMessageBox::Ok) {
			// shutdown sandbox
			QApplication::setOverrideCursor(Qt::WaitCursor);
			char *cmd;
			if (asprintf(&cmd, "firejail --shutdown=%d", pid_) != -1) {
				int rv = system(cmd);
				(void) rv;
				free(cmd);
			}
			QApplication::restoreOverrideCursor();	
			mode_ = MODE_TOP;
		}
		updated_ = false;
	}
	else if (linkstr == "join") {
		// join the process in a new xterm
		char *cmd;
		if (asprintf(&cmd, "xterm -T firejail-%d -e firejail --join=%d&", pid_, pid_) != -1) {
			int rv = system(cmd);
			(void) rv;
			free(cmd);
		}
		updated_ = false;
	}
	else if (linkstr == "about") {
		QString msg = "<table cellpadding=\"10\"><tr><td><img src=\":/resources/firetool.png\"></td>";
		msg += "<td>Firetool " + tr("version") + " " + PACKAGE_VERSION + "<br/><br/>";
		msg += tr(
			"Firejail  is  a  SUID sandbox program that reduces the risk of security "
			"breaches by restricting the running environment of  untrusted  applications "
			"using Linux namespaces and seccomp-bpf. Firetool is the graphical "
			"user interface of Firejail. Firejail and Firetool are released "
			"under GPL v2 license.<br/><br/>");
		msg += "Copyright (C) 2014 Firejail Authors<br/><br/>";
		msg += QString(PACKAGE_URL) + "</td></tr></table><br/><br/>";
	
		QMessageBox::about(this, tr("About"), msg);
		
	}
	else {
		pid_ = linkstr.toInt();
		pid_seccomp_ = -1;
		pid_caps_ = -1;
		mode_ = MODE_PID;
		updated_ = false;
	}	
}
	
