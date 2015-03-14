#include <stdio.h>
#include "applications.h"
#include "../common/utils.h"
QList<Application> applist;

void applications_init() {
	applist.append(Application("Firejail Tools and Statistics", "firestats", ":resources/firestats.png"));

	if (which("iceweasel"))
		applist.append(Application("Debian Iceweasel", "firejail iceweasel", ":resources/iceweasel.png"));
	else if (which("firefox"))
		applist.append(Application("Mozilla Firefox", "firejail firefox", ":resources/firefox.png"));
		
	if (which("chromium"))
		applist.append(Application("Chromium Web Browser", "firejail chromium", ":resources/chromium.png"));
	else if (which("chromium-browser"))
		applist.append(Application("Chromium Web Browser", "firejail chromium-browser", ":resources/chromium.png"));

	if (which("midori"))
		applist.append(Application("Midori Web Browser", "firejail midori", ":resources/midori.png"));

	if (which("opera"))
		applist.append(Application("Opera Web Browser", "firejail opera", ":resources/opera.png"));

	if (which("icedove"))
		applist.append(Application("Debian Icedove", "firejail icedove", ":resources/icedove.png"));
	else if (which("thunderbird"))
		applist.append(Application("Thunderbird", "firejail thunderbird", ":resources/thunderbird.png"));

	if (which("evince"))
		applist.append(Application("Evince PDF viewer", "firejail evince", ":resources/evince.png"));
	
	if (which("transmission-gtk"))
		applist.append(Application("Transmission BitTorrent", "firejail transmission-gtk", ":resources/transmission.png"));
	else if (which("transmission-qt"))
		applist.append(Application("Transmission BitTorrent", "firejail transmission-qt", ":resources/transmission.png"));
	
	if (which("vlc"))
		applist.append(Application("VideoLAN Client", "firejail vlc", ":resources/vlc.png"));
}

int applications_get_index(QPoint pos) {
	if (pos.y() < (MARGIN * 2 + TOP))
		return -1;
		
	if (pos.x() > (MARGIN * 2) && pos.x() < (MARGIN * 2 + 64)) {
		int nelem = applist.count();
		int index = (pos.y() - 2 * MARGIN - TOP) / 64;
		if (index < nelem) {
			return index;
		}
	}
	return -1;
}
