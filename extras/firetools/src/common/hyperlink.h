#ifndef HYPERLINK_H
#define HYPERLINK_H
#include <QLabel>

class Hyperlink : public QLabel
{
Q_OBJECT
	public:
	Hyperlink( const QString & text, QWidget * parent = 0 );
	~Hyperlink(){}

signals:
	void clicked();

protected:
	void mousePressEvent ( QMouseEvent * event ) ;

};
#endif
