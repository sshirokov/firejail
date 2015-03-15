# Firejail profile for Mozilla Thunderbird (Icedove in Debian)
include /etc/firejail/disable-mgmt.inc
include /etc/firejail/disable-secret.inc
blacklist ${HOME}/.adobe
blacklist ${HOME}/.macromedia
blacklist ${HOME}/.mozilla
caps.drop all
seccomp
netfilter

