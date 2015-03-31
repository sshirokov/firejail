#include <QtGui>
#include <QUrl>
#include <assert.h>
#include "graph.h"
#include "dbpid.h"

static QByteArray byteArray[4];
static const char *id_label[4] = {
	"CPU (%)",
	"Memory (KiB)",
	"RX (KB/sec)",
	"TX (KB/sec)"
};

QString graph(int id, DbPid *dbpid, int cycle) {
	assert(id < 4);
	assert(dbpid);
	assert(cycle < DbPid::MAXCYCLE);
	int maxcycle = DbPid::MAXCYCLE;
	int i;	
	int j;
	
	// set pixmap
#define TOPMARGIN 20	
	QPixmap *pixmap = new QPixmap((maxcycle - 1) * 4 + 100, TOPMARGIN + 100 + 30);
	QPainter *paint = new QPainter(pixmap);
	paint->fillRect(0, 0, (maxcycle - 1) * 4 + 100, TOPMARGIN + 100 + 30, Qt::white);
	paint->setPen(Qt::black);
	paint->drawRect(0, TOPMARGIN, (maxcycle - 1) * 4, 100);
	paint->setPen(QColor(80, 80, 80, 128));
	paint->drawLine(0, TOPMARGIN + 25, (maxcycle - 1) * 4, TOPMARGIN + 25);
	paint->drawLine(0, TOPMARGIN + 50, (maxcycle - 1) * 4, TOPMARGIN + 50);
	paint->drawLine(0, TOPMARGIN + 75, (maxcycle - 1) * 4, TOPMARGIN + 75);
	paint->drawLine((maxcycle - 1) * 1, TOPMARGIN, (maxcycle - 1) * 1, TOPMARGIN + 100);
	paint->drawLine((maxcycle - 1) * 2, TOPMARGIN, (maxcycle - 1) * 2, TOPMARGIN + 100);
	paint->drawLine((maxcycle - 1) * 3, TOPMARGIN, (maxcycle - 1) * 3, TOPMARGIN + 100);
	
	// extract maximum value
	float maxval = 0;
	for (i = 0; i < maxcycle; i++) {
		float val = dbpid->data_[i].get(id);
		if (val > maxval)
			maxval = val;
	}

	// adjust maxval
	maxval = qCeil(maxval);
	if (maxval < 2)
		maxval = 2;
	else if (maxval < 5)
		maxval = 5;
	else if (maxval < 10)
		maxval = 10;
	else if (maxval < 20)
		maxval = 20;
	else if (maxval < 50)
		maxval = 50;
	else if (maxval < 100)
		maxval = 100;
	else if (maxval < 200)
		maxval = 200;
	else if (maxval < 500)
		maxval = 500;
	else if (maxval < 1000)
		maxval = 1000;
	else if (maxval < 2000)
		maxval = 2000;
	else if (maxval < 5000)
		maxval = 5000;
	else if (maxval < 10000)
		maxval = 10000;
	else if (maxval < 20000)
		maxval = 20000;
	else if (maxval < 50000)
		maxval = 50000;
	else if (maxval < 100000)
		maxval = 100000;
	else if (maxval < 200000)
		maxval = 200000;
	else if (maxval < 500000)
		maxval = 500000;
	else if (maxval < 1000000)
		maxval = 1000000;
	else if (maxval < 2000000)
		maxval = 2000000;

	paint->setPen(Qt::red);
	for (i = 0, j = cycle + 1; i < maxcycle - 1; i++) {
		float y1 = dbpid->data_[j].get(id);
		y1 = (y1 / maxval) * 100;
		y1 = 100 - y1 + TOPMARGIN;
		j++;
		if (j >= maxcycle)
			j = 0;
		float y2 = dbpid->data_[j].get(id);
		y2 = (y2 / maxval) * 100;
		y2 = 100 - y2 + TOPMARGIN;
		paint->drawLine(i * 4, (int) y1, (i + 1) * 4, (int) y2);
	}

	// axis
	paint->setPen(Qt::black);
	QString ymax = QString::number((int) maxval);
	paint->drawText((maxcycle - 1) * 4 + 3, TOPMARGIN + 3, QString::number((int) maxval));
	if (qCeil(maxval / 2) == maxval / 2)
		paint->drawText((maxcycle - 1) * 4 + 3, TOPMARGIN + 50 + 3, QString::number((int) maxval / 2));
	else
		paint->drawText((maxcycle - 1) * 4 + 3, TOPMARGIN + 50 + 3, QString::number(maxval / 2, 'f', 1));
	paint->drawText((maxcycle - 1) * 4 + 3, TOPMARGIN + 100 + 3, QString("0"));
	paint->drawText(0 + 2, TOPMARGIN + 100 + 15, QString("(minutes)"));
	paint->drawText((maxcycle - 1) * 2, TOPMARGIN + 100 + 15, QString("-2"));
	paint->drawText((maxcycle - 1) * 3, TOPMARGIN + 100 + 15, QString("-1"));
	
	// title
	paint->setPen(Qt::black);
	paint->drawText(0 + 2, TOPMARGIN - 2, QString(id_label[id]));
	
	// generate image
	QBuffer buffer(&byteArray[id]);
	pixmap->save(&buffer, "PNG");
//	QString url = QString("<img src=\":resources/fjail.png\"  />");
	QString url = QString("<img src=\"data:image/png;base64,") + byteArray[id].toBase64() + "\"  />";
	delete paint;
	delete pixmap;
	return url;
}
// qt bug reported at https://bugreports.qt.io/browse/QTBUG-43270:
// A html source containing an embedded image read into QTextBrowser creates an error message 
// "QFSFileEngine::open: No file name specified", even though the image is parsed and rendered fine.

//http://stackoverflow.com/questions/6598554/is-there-any-way-to-insert-qpixmap-object-in-html

#if 0
    QString html;
     
    QImage img("c:\\temp\\sample.png"); // todo: generate image in memory
    myTextEdit->document()->addResource(QTextDocument::ImageResource, QUrl("sample.png" ), img);
     
    html.append("<p><img src=\":sample.png\"></p>");
     
    myTextEdit->setHtml(html);
#endif
