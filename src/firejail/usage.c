/*
 * Copyright (C) 2014, 2015 netblue30 (netblue30@yahoo.com)
 *
 * This file is part of firejail project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "firejail.h"

void usage(void) {
	printf("firejail - version %s\n\n", VERSION);
	printf("Firejail is a SUID sandbox program that reduces the risk of security breaches by\n");
	printf("restricting the running environment of untrusted applications using Linux\n");
	printf("namespaces. It includes a sandbox profile for Mozilla Firefox.\n\n");
	printf("Usage: firejail [options] [program and arguments]\n\n");
	printf("Without any options, the sandbox consists of a filesystem chroot build from the\n");
	printf("current system directories  mounted  read-only,  and  new PID and IPC\n");
	printf("namespaces. If no program is specified as an argument, /bin/bash is started by\n");
	printf("default in the sandbox.\n\n");
	printf("Options:\n");
	printf("\t-- - signal the end of options and disables further option processing.\n\n");
#ifdef HAVE_BIND		
	printf("\t--bind=dirname1,dirname2 - mount-bind dirname1 on top of dirname2.\n\n");
	printf("\t--bind=filename1,dirname2 - mount-bind filename1 on top of filename2.\n\n");
#endif
	printf("\t--blacklist=dirname_or_filename - blacklist directory or file.\n\n");
	printf("\t-c - execute command and exit.\n\n");
	printf("\t--caps - enable default Linux capabilities filter. The filter disables\n");
	printf("\t\tCAP_SYS_MODULE, CAP_SYS_RAWIO, CAP_SYS_BOOT, CAP_SYS_NICE,\n");
	printf("\t\tCAP_SYS_TTY_CONFIG, CAP_SYSLOG, CAP_MKNOD, CAP_SYS_ADMIN.\n\n");
	printf("\t--caps.drop=all - drop all capabilities.\n\n");
	printf("\t--caps.drop=capability,capability,capability - blacklist Linux\n");
	printf("\t\tcapabilities filter.\n\n");
	printf("\t--caps.keep=capability,capability,capability - whitelist Linux\n");
	printf("\t\tcapabilities filter.\n\n");
	printf("\t--caps.print=name - print the caps filter for the sandbox started\n");
	printf("\t\tusing --name option.\n\n");
	printf("\t--caps.print=pid - print the caps filter for the specified sandbox.\n\n");
	printf("\t--cgroup=tasks-file - place the sandbox in the specified control group.\n");
	printf("\t\ttasks-file is the full path of cgroup tasks file.\n");
	printf("\t\tExample: --cgroup=/sys/fs/cgroup/g1/tasks\n\n");
#ifdef HAVE_CHROOT		
	printf("\t--chroot=dirname - chroot into dirname directory.\n\n");
#endif
	printf("\t--cpu=cpu-number,cpu-number - set cpu affinity.\n");
	printf("\t\tExample: cpu=0,1,2\n\n");
	printf("\t--csh - use /bin/csh as default shell.\n\n");
	printf("\t--debug - print sandbox debug messages.\n\n");
	printf("\t--debug-syscalls - print all recognized system calls in the current\n");
	printf("\t\tFirejail software build and exit.\n\n");
	printf("\t--debug-caps - print all recognized capabilities in the current\n");
	printf("\t\tFirejail software build and exit.\n\n");
	printf("\t--defaultgw=address - use this address as default gateway in the new\n");
	printf("\t\tnetwork namespace.\n\n");
	printf("\t--dns=address - set a DNS server for the sandbox. Up to three DNS\n");
	printf("\t\tservers can be defined.\n\n");
	printf("\t--dns.print=name - print DNS configuration for the sandbox started\n");
	printf("\t\tusing --name option.\n\n");
	printf("\t--dns.print=pid - print DNS configuration of the specified process.\n\n");
	printf("\t--help, -? - this help screen.\n\n");
	printf("\t--ip=address - use this IP address in the new network namespace.\n\n");
	printf("\t--ip=none - no IP address and no default gateway address are configured\n");
	printf("\t\tin the new network namespace. Use this option in case you intend\n");
	printf("\t\tto start an external DHCP client in the sandbox.\n\n");
	printf("\t--ipc-namespace - enable a new IPC namespace if the sandbox was started\n");
	printf("\t\tas a regular user. IPC namespace is enabled by default only if\n");
	printf("\t\tthe sandbox is started as root.\n\n");
	printf("\t--join=name - join the sandbox started using --name option.\n\n");
	printf("\t--join=pid - join the sandbox of the specified process.\n\n");
	printf("\t--list - list all sandboxes.\n\n");
	printf("\t--name=name - set sandbox hostname.\n\n");
	printf("\t--net=bridgename - enable network namespaces and connect to this bridge\n");
	printf("\t\tdevice. Unless specified with option --ip and --defaultgw, an\n");
	printf("\t\tIP address and a default gateway will be assigned automatically\n");
	printf("\t\tto the sandbox. The IP address is checked using ARP before\n");
	printf("\t\tassignment. The IP address assigned as default gateway is the\n");
	printf("\t\tbridge device IP address. Up to four --net devices can\n");
	printf("\t\tbe defined. Mixing bridge and macvlan devices is allowed.\n\n");
	printf("\t--net=ethernet_interface - enable network namespaces and connect\n");
	printf("\t\tto this ethernet_interface using the standard Linux macvlan\n");
	printf("\t\tdriver. Unless specified with option --ip and --defaultgw, an\n");
	printf("\t\tIP address and a default gateway will be assigned automatically\n");
	printf("\t\tto the sandbox. The IP address is checked using ARP before\n");
	printf("\t\tassignment. The IP address assigned as default gateway is the\n");
	printf("\t\tdefault gateway of the host. Up to four --net devices can\n");
	printf("\t\tbe defined. Mixing bridge and macvlan devices is allowed.\n\n");
	printf("\t--net=none - enable a new, unconnected network namespace.\n\n");

	printf("\t--netfilter - enable the default client network filter in the new\n");
	printf("\t\tnetwork namespace:\n\n");
	printf("\t\t*filter\n");
	printf("\t\t:INPUT DROP [0:0]\n");
	printf("\t\t:FORWARD DROP [0:0]\n");
	printf("\t\t:OUTPUT ACCEPT [0:0]\n");
	printf("\t\t-A INPUT -i lo -j ACCEPT\n");
	printf("\t\t-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT\n");
	printf("\t\t-A INPUT -p icmp --icmp-type destination-unreachable -j ACCEPT\n");
	printf("\t\t-A INPUT -p icmp --icmp-type time-exceeded -j ACCEPT\n");
	printf("\t\t-A INPUT -p icmp --icmp-type echo-request -j ACCEPT \n");
	printf("\t\tCOMMIT\n\n");
	printf("\t--netfilter=filename - enable the network filter specified by\n");
	printf("\t\tfilename in the new network namespace. The filter file format\n");
	printf("\t\tis the format of iptables-save and iptable-restore commands.\n\n");

	printf("\t--netstats - monitor network statistics for sandboxes creating a new\n");
	printf("\t\tnetwork namespace.\n\n");
	printf("\t--nogroups - disable supplementary groups. Without this option,\n");
	printf("\t\tsupplementary groups are enabled for the user starting the\n");
	printf("\t\tsandbox. For root user supplementary groups are always\n");
	printf("\t\tdisabled.\n\n");
	printf("\t--output=logfile - stdout logging and log rotation. Copy stdout to\n");
	printf("\t\tlogfile, and keep the size of the file under 500KB using log\n");
	printf("\t\trotation. Five files with prefixes .1 to .5 are used in\n");
	printf("\t\trotation.\n\n");
	printf("\t--overlay - mount a filesystem overlay on top of the current filesystem.\n");
	printf("\t\t(OverlayFS support is required in Linux kernel for this option\n");
	printf("\t\tto work)\n\n");    
	printf("\t--private - mount new /root and /home/user directories.\n\n");
	printf("\t--private=directory - use directory as user home.\n\n");
	printf("\t--profile=filename - use a custom profile.\n\n");
	printf("\t--read-only=dirname_or_filename - set directory or file read-only.\n\n");
	printf("\t--rlimit-fsize=number - set the maximum file size that can be created\n");
	printf("\t\tby a process.\n\n");
	printf("\t--rlimit-nofile=number - set the maximum number of files that can be\n");
	printf("\t\topened by a process.\n\n");
	printf("\t--rlimit-nproc=number - set the maximum number of processes that can be\n");
	printf("\t\tcreated for the real user ID of the calling process.\n\n");
	printf("\t--rlimit-sigpending=number - set the maximum number of pending signals\n");
	printf("\t\tfor a process.\n\n");

#ifdef HAVE_SECCOMP
	printf("\t--seccomp - enable seccomp filter and blacklist the syscalls in the\n");
	printf("\t\tlist. The default list is as follows: mount, umount2,\n");
	printf("\t\tptrace, kexec_load, open_by_handle_at, init_module,\n");
	printf("\t\tfinit_module, delete_module, iopl, ioperm, swapon, swapoff,\n");
	printf("\t\tmknode, syslog, process_vm_readv and process_vm_writev\n\n");
	
	printf("\t--seccomp=syscall,syscall,syscall - enable seccomp filter, blacklist the\n");
	printf("\t\tdefault syscall list and the syscalls specified by the command.\n\n");
	
	printf("\t--seccomp.drop=syscall,syscall,syscall - enable seccomp filter, and\n");
	printf("\t\tblacklist the syscalls specified by the command.\n\n");
	
	printf("\t--seccomp.keep=syscall,syscall,syscall - enable seccomp filter, and\n");
	printf("\t\twhitelist the syscalls specified by the command.\n\n");
	
	printf("\t--seccomp.print=name - print the seccomp filter for the sandbox started\n");
	printf("\t\tusing --name option.\n\n");
	printf("\t--seccomp.print=pid - print the seccomp filter for the sandbox specified\n");
	printf("\t\tby process ID.\n\n");
#endif

	printf("\t--shell=none - run the program directly without a user shell.\n\n");
	printf("\t--shell=program - set default user shell.\n\n");
	printf("\t--shutdown=name - shutdown the sandbox started using --name option.\n\n");
	printf("\t--shutdown=pid - shutdown the sandbox specified by process ID.\n\n");
	printf("\t--tmpfs=dirname - mount a tmpfs filesystem on directory dirname.\n\n");
	printf("\t--top - monitor the most CPU-intensive sandboxes.\n\n");
	printf("\t--trace - trace open, access and connect system calls.\n\n");
	printf("\t--tree - print a tree of all sandboxed processes.\n\n");
	printf("\t--version - print program version and exit.\n\n");
	printf("\t--zsh - use /usr/bin/zsh as default shell.\n\n");
	printf("\n");

	printf("Monitoring\n\n");

	printf("Option --list prints a list of all sandboxes. The format for each entry is as\n");
	printf("follows:\n\n");
	printf("\tPID:USER:Command\n\n");

	printf("Option --tree prints the tree of processes running in the sandbox. The format\n");
	printf("for each process entry is as follows:\n\n");
	printf("\tPID:USER:Command\n\n");

	printf("Option --top is similar to the UNIX top command, however it applies only to\n");
	printf("sandboxes. Listed below are the available fields (columns) in alphabetical\n");
	printf("order:\n\n");
	printf("\tCommand - command used to start the sandbox.\n");
	printf("\tCPU%% - CPU usage, the sandbox share of the elapsed CPU time since the\n");
	printf("\t       last screen update\n");
	printf("\tPID - Unique process ID for the task controlling the sandbox.\n");
	printf("\tPrcs - number of processes running in sandbox, including the controlling\n");
	printf("\t       process.\n");
	printf("\tRES - Resident Memory Size (KiB), sandbox non-swapped physical memory.\n");
	printf("\t      It is a sum of the RES values for all processes running in the\n");
	printf("\t      sandbox.\n");
	printf("\tSHR - Shared Memory Size (KiB), it reflects memory shared with other\n");
	printf("\t      processes. It is a sum of the SHR values for all processes running\n");
	printf("\t      in the sandbox, including the controlling process.\n");
	printf("\tUptime - sandbox running time in hours:minutes:seconds format.\n");
	printf("\tUser - The owner of the sandbox.\n");
	printf("\n");
	printf("Profile files\n\n");
	printf("Several command line configuration options can be passed to the program using\n");
	printf("profile files. Default Firejail profile files are stored in /etc/firejail\n");
	printf("directory, user profile files are stored in ~/.config/firejail directory. See\n");
	printf("man 5 firejail-profile for more information.\n\n");
	printf("Restricted shell\n\n");
	printf("To  configure a restricted shell, replace /bin/bash with /usr/bin/firejail i\n");
	printf("/etc/password file for each user that needs to  be  restricted.\n");
	printf("Alternatively, you can specify /usr/bin/firejail  in adduser command:\n\n");
	printf("   adduser --shell /usr/bin/firejail username\n\n");
	printf("Arguments to be passed to firejail executable upon login are  declared  in\n");
	printf("/etc/firejail/login.users file.\n\n");
	printf("Examples:\n\n");
	printf("   $ firejail\n");
	printf("          start a regular /bin/bash session in sandbox\n");
	printf("   $ firejail firefox\n");
	printf("          start Mozilla Firefox\n");
	printf("   $ firejail --seccomp firefox\n");
	printf("          start Mozilla Firefox in a seccomp sandbox\n");
	printf("   $ firejail --caps firefox\n");
	printf("          start Mozilla Firefox in a Linux capabilities sandbox\n");
	printf("   $ firejail --debug firefox\n");
	printf("          debug Firefox sandbox\n");
	printf("   $ firejail --private\n");
	printf("          start a /bin/bash session with a new tmpfs home directory\n");
	printf("   $ firejail --net=br0 ip=10.10.20.10\n");
	printf("          start a /bin/bash session in a new network namespace; the session is\n");
	printf("          connected to the main network using br0 bridge device, an IP address\n");
	printf("          of 10.10.20.10 is assigned to the sandbox\n");
	printf("   $ firejail --net=br0 --net=br1 --net=br2\n");
	printf("          start a /bin/bash session in a new network namespace and connect it\n");
	printf("          to br0, br1, and br2 host bridge devices\n");
	printf("   $ firejail --list\n");
	printf("          list all running sandboxes\n");
	printf("\n");
	printf("License GPL version 2 or later\n");
	printf("Homepage: http://firejail.sourceforge.net\n");
	printf("\n");
}
