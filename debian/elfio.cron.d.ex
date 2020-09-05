#
# Regular cron jobs for the elfio package
#
0 4	* * *	root	[ -x /usr/bin/elfio_maintenance ] && /usr/bin/elfio_maintenance
