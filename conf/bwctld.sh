#!/bin/bash
#
# Init file for  Bandwidth Control daemon
#
# chkconfig: 2345 60 20
# description: Bandwidth Control daemon
#
# processname: bwctld 
#


#PREFIX=/prefixdir
BWCTLDBINDIR=${PREFIX}/bin
CONFDIR=${PREFIX}/etc
BWCTLDVARDIR=/var/run
PIDFILE=${BWCTLDVARDIR}/bwctl-server.pid

BWCTLD="${BWCTLDBINDIR}/bwctld -c ${CONFDIR} "

ERROR=0
ARGV="$@"
if [ "x$ARGV" = "x" ] ; then 
    ARGS="help"
fi

for ARG in $@ $ARGS
do
    # check for pidfile
    if [ -f $PIDFILE ] ; then
	PID=`cat $PIDFILE`
	if [ "x$PID" != "x" ] && kill -0 $PID 2>/dev/null ; then
	    STATUS="bwctld (pid $PID) running"
	    RUNNING=1
	else
	    STATUS="bwctld (pid $PID?) not running"
	    RUNNING=0
	fi
    else
	STATUS="bwctld (no pid file) not running"
	RUNNING=0
    fi

    case $ARG in
    start)
	if [ $RUNNING -eq 1 ]; then
	    echo "$0 $ARG: bwctld (pid $PID) already running"
	    continue
	fi

	echo $BWCTLD

	if $BWCTLD ; then
	    echo "$0 $ARG: bwctld started"
	else
	    echo "$0 $ARG: bwctld could not be started"
	    ERROR=3
	fi
	;;
    stop)
	if [ $RUNNING -eq 0 ]; then
	    echo "$0 $ARG: $STATUS"
	    continue
	fi
	if kill $PID ; then
	    echo "$0 $ARG: bwctld stopped"
	else
	    echo "$0 $ARG: bwctld could not be stopped"
	    ERROR=4
	fi
	;;
    restart)
    	$0 stop; echo "waiting..."; sleep 10; $0 start;
	;;
#	if [ $RUNNING -eq 0 ]; then
#	    echo "$0 $ARG: bwctld not running, trying to start"
#	    if $BWCTLD ; then
#		echo "$0 $ARG: bwctld started"
#	    else
#		echo "$0 $ARG: bwctld could not be started"
#		ERROR=5
#	    fi
#	else
#	    if kill -HUP $PID ; then
#	       echo "$0 $ARG: bwctld restarted"
#	    else
#	       echo "$0 $ARG: bwctld could not be restarted"
#	       ERROR=6
#	    fi
#	fi
#	;;
    *)
	echo "usage: $0 (start|stop|restart|help)"
	cat <<EOF

start      - start bwctld
stop       - stop bwctld
restart    - restart bwctld if running by sending a SIGHUP or start if 
             not running
help       - this screen

EOF
	ERROR=2
    ;;

    esac

done

exit $ERROR
