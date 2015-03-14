#include "hyperlink.h"

Hyperlink::Hyperlink( const QString & text, QWidget * parent )
:QLabel(parent) {
	(void) text;
}

void Hyperlink::mousePressEvent ( QMouseEvent * event ) {
	(void) event;
	emit clicked();
}
