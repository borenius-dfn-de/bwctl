/*
 *      $Id$
 */
/************************************************************************
*									*
*			     Copyright (C)  2003			*
*				Internet2				*
*			     All Rights Reserved			*
*									*
************************************************************************/
/*
 *	File:		iperfc.c
 *
 *	Authors:	Jeff Boote
 *			Internet2
 *
 *	Date:		Mon Sep 15 10:54:30 MDT 2003
 *
 *	Description:	
 *
 *	Initial implementation of iperfc commandline application. This
 *	application will measure active one-way udp latencies. And it will
 *	set up perpetual tests and keep them going until this application
 *	is killed.
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <signal.h>
#include <assert.h>
#include <syslog.h>
#include <math.h>

#include <ipcntrl/ipcntrl.h>


#if defined HAVE_DECL_OPTRESET && !HAVE_DECL_OPTRESET
int optreset;
#endif

#include "./iperfcP.h"

/*
 * The iperfc context
 */
static	ipapp_trec		app;
static	I2ErrHandle		eh;
static	u_int32_t		file_offset,ext_offset;
static	int			ip_reset = 0;
static	int			ip_exit = 0;
static	int			ip_error = SIGCONT;
static	IPFContext		ctx;
static	ipsess_trec		local;
static	ipsess_trec		remote;
static	IPFNum64		zero64;
static	IPFSID			sid;
static	u_int16_t		recv_port;
static	ipsess_t		s[2];	/* receiver == 0, sender == 1 */

static void
print_conn_args()
{
	fprintf(stderr,"              [Connection Args]\n\n"
"   -A authmode    requested modes: [A]uthenticated, [E]ncrypted, [O]pen\n"
"   -k keyfile     AES keyfile to use with Authenticated/Encrypted modes\n"
"   -U username    username to use with Authenticated/Encrypted modes\n"
"   -B srcaddr     use this as a local address for control connection and tests\n"
	);
}

static void
print_test_args()
{
	fprintf(stderr,
"              [Test Args]\n\n"
"   -i interval    report interval (seconds)\n"
"   -l len         length of read/write buffers (bytes)\n"
"   -u             UDP test\n"
"   -w window      TCP window size (bytes)\n"
"   -P nThreads    number of concurrent connections (ENOTSUPPORTED)\n"
"   -S TOS         type-of-service for outgoing packets (ENOTSUPPORTED)\n"
"   -b bandwidth   bandwidth to use for UDP test (bits/sec KM) (Default: 1Mb)\n"
"   -t time        duration of test (seconds) (Default: 10)\n"
"   -c             local sender \"client in iperf speak\" (TAKES NO ARG)\n"
"   -s             local receiver \"server in iperf speak\" (TAKES NO ARG)\n"
"              [MUST SPECIFY EXACTLY ONE OF -c/-s]"
	);
}

static void
print_output_args()
{
	fprintf(stderr,
"              [Output Args]\n\n"
"   -d dir         directory to save session file in (only if -p)\n"
"   -I Interval    time between IPF test sessions(seconds)\n"
"   -L LatestDelay latest time into an interval to run test(seconds)\n"
"   -p             print completed filenames to stdout - not session data\n"
"   -h             print this message and exit\n"
"   -e             syslog facility to log to\n"
"   -r             send syslog to stderr\n"
		);
}

static void
usage(const char *progname, const char *msg)
{
	if(msg) fprintf(stderr, "%s: %s\n", progname, msg);
	fprintf(stderr,"usage: %s %s\n", 
			progname,
			 "[arguments] remotehost"
			);
	fprintf(stderr, "\n");
	print_conn_args();
		
	fprintf(stderr, "\n");
	print_test_args();
		
	fprintf(stderr, "\n");
	print_output_args();

	return;
}

/*
** Initialize authentication and policy data (used by owping and owfetch)
*/
void
ip_set_auth(
	ipapp_trec	*pctx, 
	char		*progname,
	IPFContext	ctx __attribute__((unused))
	)
{
#if	NOT
#ifndef	NDEBUG
	somestate.childwait = app.opt.childwait;
#endif
#endif
#if	NOT
	IPFErrSeverity err_ret;

	if(pctx->opt.identity){
		/*
		 * Eventually need to modify the policy init for the
		 * client to deal with a pass-phrase instead of/ or in
		 * addition to the keyfile file.
		 */
		*policy = IPFPolicyInit(ctx, NULL, NULL, pctx->opt.keyfile, 
				       &err_ret);
		if (err_ret == IPFErrFATAL){
			I2ErrLog(eh, "PolicyInit failed. Exiting...");
			exit(1);
		};
	}
#endif


	/*
	 * Verify/decode auth options.
	 */
	if(pctx->opt.authmode){
		char	*s = app.opt.authmode;
		pctx->auth_mode = 0;
		while(*s != '\0'){
			switch (toupper(*s)){
				case 'O':
				pctx->auth_mode |= IPF_MODE_OPEN;
				break;
				case 'A':
				pctx->auth_mode |= IPF_MODE_AUTHENTICATED;
				break;
				case 'E':
				pctx->auth_mode |= IPF_MODE_ENCRYPTED;
				break;
				default:
				I2ErrLogP(eh,EINVAL,"Invalid -authmode %c",*s);
				usage(progname, NULL);
				exit(1);
			}
			s++;
		}
	}else{
		/*
		 * Default to all modes.
		 * If identity not set - library will ignore A/E.
		 */
		pctx->auth_mode = IPF_MODE_OPEN|IPF_MODE_AUTHENTICATED|
							IPF_MODE_ENCRYPTED;
	}
}

static void
CloseSessions()
{
	/* TODO: Handle clearing other state. Canceling tests nicely? */

	if(remote.cntrl){
		IPFControlClose(remote.cntrl);
		remote.cntrl = NULL;
		remote.tspec.req_time.ipftime = zero64;
	}
	if(local.cntrl){
		IPFControlClose(local.cntrl);
		local.cntrl = NULL;
		local.tspec.req_time.ipftime = zero64;
	}

	return;
}

static void
sig_catch(
		int	signo
		)
{
	switch(signo){
		case SIGINT:
		case SIGTERM:
			ip_exit++;
			break;
		case SIGHUP:
			ip_reset++;
			break;
		default:
			ip_error = signo;
			break;
	}

	return;
}

static int
sig_check()
{
	if(ip_error != SIGCONT){
		I2ErrLog(eh,"sig_catch(%d):UNEXPECTED SIGNAL NUMBER",ip_error);
		exit(1);
	}

	if(ip_exit || ip_reset){
		CloseSessions();
	}

	if(ip_exit){
		I2ErrLog(eh,"SIGTERM/SIGINT: Exiting.");
		exit(0);
	}

	if(ip_reset){
		ip_reset = 0;
		return 1;
	}

	return 0;
}

int
main(
	int	argc,
	char	**argv
)
{
	char			*progname;
	int			lockfd;
	char			lockpath[PATH_MAX];
	int			rc;
	IPFErrSeverity		err_ret = IPFErrOK;
	I2ErrLogSyslogAttr	syslogattr;

	int			fname_len;
	int			ch;
	char                    *endptr = NULL;
	char                    optstring[128];
	static char		*conn_opts = "A:B:k:U:";
	static char		*out_opts = "d:I:L:pe:rv";
	static char		*test_opts = "i:l:uw:P:S:b:t:cs";
	static char		*gen_opts = "hW";

	char			dirpath[PATH_MAX];
	struct flock		flk;
	struct sigaction	act;
	IPFNum64		latest64;
	u_int32_t		p,q;

	progname = (progname = strrchr(argv[0], '/')) ? ++progname : *argv;

	/* Create options strings for this program. */
	strcpy(optstring, conn_opts);
	strcat(optstring, test_opts);
	strcat(optstring, out_opts);
	strcat(optstring, gen_opts);
		

	syslogattr.ident = progname;
	syslogattr.logopt = LOG_PID;
	syslogattr.facility = LOG_USER;
	syslogattr.priority = LOG_ERR;
	syslogattr.line_info = I2MSG;
#ifndef	NDEBUG
	syslogattr.line_info |= I2FILE | I2LINE;
#endif

	opterr = 0;
	while((ch = getopt(argc, argv, optstring)) != -1){
		if(ch == 'e'){
			int fac;
			if((fac = I2ErrLogSyslogFacility(optarg)) == -1){
				fprintf(stderr,
				"Invalid -e: Syslog facility \"%s\" unknown\n",
				optarg);
				exit(1);
			}
			syslogattr.facility = fac;
		}
		else if(ch == 'r'){
			syslogattr.logopt |= LOG_PERROR;
		}
	}
	opterr = optreset = optind = 1;

	/*
	* Start an error logging session for reporing errors to the
	* standard error
	*/
	eh = I2ErrOpen(progname, I2ErrLogSyslog, &syslogattr, NULL, NULL);
	if(! eh) {
		fprintf(stderr, "%s : Couldn't init error module\n", progname);
		exit(1);
	}

	/* Set default options. */
	memset(&app,0,sizeof(app));
	app.opt.timeDuration = 10;

	while ((ch = getopt(argc, argv, optstring)) != -1)
		switch (ch) {
		/* Connection options. */
		case 'A':
			if (!(app.opt.authmode = strdup(optarg))) {
				I2ErrLog(eh,"malloc:%M");
				exit(1);
			}
			break;
		case 'B':
			if (!(app.opt.srcaddr = strdup(optarg))) {
				I2ErrLog(eh,"malloc:%M");
				exit(1);
			}
			break;
		case 'U':
			if (!(app.opt.identity = strdup(optarg))) {
				I2ErrLog(eh,"malloc:%M");
				exit(1);
			}
			break;
		case 'k':
			if (!(app.opt.keyfile = strdup(optarg))) {
				I2ErrLog(eh,"malloc:%M");
				exit(1);
			}
			break;

		/* OUTPUT OPTIONS */
		case 'd':
			if (!(app.opt.savedir = strdup(optarg))) {
				I2ErrLog(eh,"malloc:%M");
				exit(1);
			}
			break;
		case 'I':
			app.opt.seriesInterval =strtoul(optarg, &endptr, 10);
			if (*endptr != '\0') {
				usage(progname, 
				"Invalid value. Positive integer expected");
				exit(1);
			}
			break;
		case 'L':
			app.opt.seriesWindow = strtoul(optarg,&endptr,10);
			if(*endptr != '\0'){
				usage(progname, 
				"Invalid value. Positive integer expected");
				exit(1);
			}
			break;
		case 'p':
			app.opt.printfiles = True;
			break;
		case 'e':
		case 'r':
			/* handled in prior getopt call... */
			break;
		case 'v':
			app.opt.version = True;
			I2ErrLog(eh,"Version: $Revision$");
			exit(0);

		/* TEST OPTIONS */
		case 'c':
			app.opt.send = True;
			break;
		case 's':
			app.opt.recv = True;
			break;
		case 'i':
			app.opt.reportInterval =strtoul(optarg, &endptr, 10);
			if (*endptr != '\0') {
				usage(progname, 
				"Invalid value. Positive integer expected");
				exit(1);
			}
			break;
		case 'l':
			app.opt.lenBuffer =strtoul(optarg, &endptr, 10);
			if (*endptr != '\0') {
				usage(progname, 
				"Invalid value. Positive integer expected");
				exit(1);
			}
			break;
		case 'u':
			app.opt.udpTest = True;
			break;
		case 'w':
			app.opt.windowSize =strtoul(optarg, &endptr, 10);
			if (*endptr != '\0') {
				usage(progname, 
				"Invalid value. Positive integer expected");
				exit(1);
			}
			break;
		case 'P':
			app.opt.parallel =strtoul(optarg, &endptr, 10);
			if (*endptr != '\0') {
				usage(progname, 
				"Invalid value. Positive integer expected");
				exit(1);
			}
			I2ErrLog(eh,"-P option not currently supported");
			exit(1);
			break;
		case 'S':
			app.opt.tos =strtoul(optarg, &endptr, 10);
			if (*endptr != '\0') {
				usage(progname, 
				"Invalid value. Positive integer expected");
				exit(1);
			}
			I2ErrLog(eh,"-S option not currently supported");
			exit(1);
			break;
		case 'b':
			app.opt.bandWidth =strtoul(optarg, &endptr, 10);
			if (*endptr != '\0') {
				usage(progname, 
				"Invalid value. Positive integer expected");
				exit(1);
			}
			break;
		case 't':
			app.opt.timeDuration = strtoul(optarg, &endptr, 10);
			if((*endptr != '\0') || (app.opt.timeDuration == 0)){
				usage(progname, 
				"Invalid value. Positive integer expected");
				exit(1);
			}
			break;
#ifndef	NDEBUG
		case 'W':
			app.opt.childwait = True;
			break;
#endif
		/* Generic options.*/
		case 'h':
		case '?':
		default:
			usage(progname, "");
			exit(0);
		/* UNREACHED */
		}
	argc -= optind;
	argv += optind;

	if(argc != 1){
		usage(progname, NULL);
		exit(1);
	}
	app.remote_test = argv[0];

	if(app.opt.recv == app.opt.send){
		usage(progname,"Exactly one of -s or -c must be specified.");
		exit(1);
	}

	/*
	 * Useful constant
	 */
	zero64 = IPFULongToNum64(0);

	/*
	 * Check savedir option. Make sure it will not make fnames
	 * exceed PATH_MAX even with the nul byte.
	 * Also set file_offset and ext_offset to the lengths needed.
	 */
	fname_len = TSTAMPCHARS + strlen(IPF_FILE_EXT) + strlen(SUMMARY_EXT);
	assert((fname_len+1)<PATH_MAX);
	if(app.opt.savedir){
		if((strlen(app.opt.savedir) + strlen(IPF_PATH_SEPARATOR)+
						fname_len + 1) > PATH_MAX){
			usage(progname,"-d: pathname too long.");
			exit(1);
		}
		strcpy(dirpath,app.opt.savedir);
		strcat(dirpath,IPF_PATH_SEPARATOR);
	}else
		dirpath[0] = '\0';

	if(!app.opt.timeDuration){
		app.opt.timeDuration = 10; /* 10 second default */
	}

	/*
	 * If seriesInterval is in use, verify the args and pick a
	 * resonable default for seriesWindow if needed.
	 */
	if(app.opt.seriesInterval){
		if(app.opt.seriesInterval <
				(app.opt.timeDuration + SETUP_ESTIMATE)){
			usage(progname,"-I: interval too small relative to -t");
			exit(1);
		}
		/*
		 * Make sure tests start before 50% of the 'interval' is
		 * gone.
		 */
		if(!app.opt.seriesWindow){
			app.opt.seriesWindow = MIN(
			app.opt.seriesInterval-app.opt.timeDuration,
			app.opt.seriesInterval * 0.5);
		}
	}
	else{
		/*
		 * Make sure tests start within 2 test durations.
		 */
		if(!app.opt.seriesWindow){
			app.opt.seriesWindow = app.opt.timeDuration * 2;
		}
	}
	latest64 = IPFULongToNum64(app.opt.seriesWindow);

	if(app.opt.udpTest && !app.opt.bandWidth){
		app.opt.bandWidth = DEF_UDP_RATE;
	}

	if(app.opt.bandWidth && !app.opt.udpTest){
		usage(progname,"-b: only valid with -u");
		exit(1);
	}

	/*
	 * Lock the directory for iperfc if it is in printfiles mode.
	 */
	if(app.opt.printfiles){
		strcpy(lockpath,dirpath);
		strcat(lockpath,IPLOCK);
		lockfd = open(lockpath,O_RDWR|O_CREAT,
					S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if(lockfd < 0){
		     I2ErrLog(eh,"open(%s):%M",lockpath);
		     exit(1);
		}

		flk.l_start = 0;
		flk.l_len = 0;
		flk.l_type = F_WRLCK;
		flk.l_whence = SEEK_SET;
		while((rc = fcntl(lockfd,F_SETLK,&flk)) < 0 && errno == EINTR);
		if(rc < 0){
			I2ErrLog(eh,"Unable to lock file %s:%M",lockpath);
			if(I2Readn(lockfd,&ch,sizeof(ch)) == sizeof(ch)){
				I2ErrLog(eh,"Possibly locked by pid(%d)",ch);
			}
			exit(1);
		}

		ch = getpid();
		if(I2Writen(lockfd,&ch,sizeof(ch)) != sizeof(ch)){
			I2ErrLog(eh,"Unable to write to lockfile:%M");
			exit(1);
		}
	}
	file_offset = strlen(dirpath);
	ext_offset = file_offset + TSTAMPCHARS;

	/*
	 * Initialize library with configuration functions.
	 */
	if( !(ctx = IPFContextCreate(eh))){
		I2ErrLog(eh, "Unable to initialize IPF library.");
		exit(1);
	}

	/*
	 * Initialize session records
	 */
	memset(&local,0,sizeof(local));
	/* skip req_time/latest_time - set per/test */
	local.tspec.duration = app.opt.timeDuration;
	local.tspec.udp = app.opt.udpTest;
	if(local.tspec.udp){
		local.tspec.bandwidth = app.opt.bandWidth;
	}
	local.tspec.window_size = app.opt.windowSize;
	local.tspec.len_buffer = app.opt.lenBuffer;
	local.tspec.report_interval = app.opt.reportInterval;

	/*
	 * Setup addresses of test endpoints.
	 */
	local.tspec.sender = (app.opt.send)?
				IPFAddrByNode(ctx,app.opt.srcaddr):
					IPFAddrByNode(ctx,app.remote_test);
	local.tspec.receiver = (!app.opt.send)?
				IPFAddrByNode(ctx,app.opt.srcaddr):
					IPFAddrByNode(ctx,app.remote_test);

	/*
	 * copy local tspec to remote record.
	 */
	memcpy(&remote,&local,sizeof(local));


	/* s[0] == reciever, s[1] == sender */
	s[0] = (app.opt.send)? &remote: &local;
	s[1] = (!app.opt.send)? &remote: &local;
	s[1]->send = True;

	/*
	 * TODO: Fix this.
	 * Setup policy stuff - this currently sucks.
	 */
	ip_set_auth(&app,progname,ctx); 

	/*
	 * setup sighandlers
	 */
	ip_reset = ip_exit = 0;
	act.sa_handler = sig_catch;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if(		(sigaction(SIGTERM,&act,NULL) != 0) ||
			(sigaction(SIGINT,&act,NULL) != 0) ||
			(sigaction(SIGHUP,&act,NULL) != 0)){
		I2ErrLog(eh,"sigaction(): %M");
		exit(1);
	}


	/*
	 * TODO
	 * If doing sessionInterval:
	 * 	create a pseudo-poisson context
	 * 	sleep for rand([0,1])*sessionInterval (spread out start time)
	 * 	create loop for the remainder of main() that sets up a test
	 * 	every sessionInterval*pseudo-poisson
	 */
	do{
		IPFTimeStamp	req_time;
		IPFTimeStamp	currtime;

		if(sig_check()) exit(1);

		/* Open remote connection */
		if(!remote.cntrl){
			remote.cntrl = IPFControlOpen(ctx,
				IPFAddrByNode(ctx,app.opt.srcaddr),
				IPFAddrByNode(ctx,app.remote_test),
				app.auth_mode,app.opt.identity,
				NULL,&err_ret);
			/* TODO: deal with temporary failures */
			if(sig_check()) exit(1);
			if(!remote.cntrl) exit(1);
		}
		/* Open local connection */
		if(!local.cntrl){
			local.cntrl = IPFControlOpen(ctx,
				NULL,
				IPFAddrByLocalControl(remote.cntrl),
				app.auth_mode,app.opt.identity,
				NULL,&err_ret);
			/* TODO: deal with temporary failures */
			if(sig_check()) exit(1);
			if(!local.cntrl) exit(1);
		}

		/*
		 * Now caluculate how far into the future the test
		 * request should be made for.
		 */
		/* initialize */
		req_time.ipftime = zero64;

		/*
		 * Query remote time error and update round-trip bound.
		 * (The time will be over-written later, we really only
		 * care about the errest portion of the timestamp.)
		 */
		if(IPFControlTimeCheck(remote.cntrl,&local.tspec.req_time) !=
								IPFErrOK){
			I2ErrLogP(eh,errno,"IPFControlTimeCheck: %M");
			exit(1);
		}
		/* req_time.ipftime += (3*round-trip-bound) */
		req_time.ipftime = IPFNum64Add(req_time.ipftime,
				IPFNum64Mult(IPFGetRTTBound(remote.cntrl),
							IPFULongToNum64(3)));

		/*
		 * Query local time error and update round-trip bound.
		 * (The time will be over-written later, we really only
		 * care about the errest portion of the timestamp.)
		 */
		if(IPFControlTimeCheck(local.cntrl,&remote.tspec.req_time) !=
								IPFErrOK){
			I2ErrLogP(eh,errno,"IPFControlTimeCheck: %M");
			exit(1);
		}
		/* req_time.ipftime += (3*round-trip-bound) */
		req_time.ipftime = IPFNum64Add(req_time.ipftime,
				IPFNum64Mult(IPFGetRTTBound(local.cntrl),
							IPFULongToNum64(3)));

		/*
		 * Add a small constant value to this... Will need to experiment
		 * to find the right number.
		 */
		req_time.ipftime = IPFNum64Add(req_time.ipftime,
						IPFULongToNum64(5));

		/*
		 * req_time currently holds a reasonable relative amount of
		 * time from 'now' that a test could be held. Get the current
		 * time and add to make that an 'absolute' value.
		 */
		if(!IPFGetTimeOfDay(&currtime)){
			I2ErrLogP(eh,errno,"IPFGetTimeOfDay: %M");
			exit(1);
		}
		req_time.ipftime = IPFNum64Add(req_time.ipftime,
							currtime.ipftime);
		/*
		 * Get a reservation:
		 * 	s[0] == receiver
		 * 	s[1] == sender
		 * 	initialize req_time/latest_time
		 * 	keep querying each server in turn until satisfied,
		 * 	or denied.
		 */
		s[0]->tspec.latest_time = s[1]->tspec.latest_time =
					IPFNum64Add(req_time.ipftime, latest64);
		s[1]->tspec.req_time.ipftime = zero64;
		memset(sid,0,sizeof(sid));
		recv_port = 0;

		p=0;q=0;
		while(1){

			/*
			 * p is the current connection we are talking to,
			 * q is the "other" one.
			 * (Logic is started so the first time through this loop
			 * we are talking to the "receiver". That is required
			 * to initialize the sid and recv_port.)
			 */
			p = q++;
			q %= 2;

			s[p]->tspec.req_time.ipftime = req_time.ipftime;

			/* TODO: do something with return values.
			 */
			if(!IPFSessionRequest(s[p]->cntrl,s[p]->send,
					&s[p]->tspec,&req_time,&recv_port,
					sid,&err_ret)){
				if((err_ret == IPFErrOK) &&
						(IPFNum64Cmp(req_time.ipftime,
							     zero64) != 0)){
					/*
					 * Request is ok, but server is too
					 * busy. Skip this test and proceed
					 * to next session interval.
					 */
					I2ErrLog(eh,
						"SessionRequest: Server busy.");
					goto next_test;
				}
				CloseSessions();
				I2ErrLog(eh,
					"SessionRequest failure. Skipping.");
				goto next_test;
			}else{
				s[p]->tspec.req_time.ipftime = req_time.ipftime;
			}

			/*
			 * Do we have a meeting?
			 */
			if(IPFNum64Cmp(s[p]->tspec.req_time.ipftime,
					s[q]->tspec.req_time.ipftime) == 0){
				break;
			}
		}

		/* TODO: Add sighandler for SIGINT that sends StopSessions? */

		/*
		 * Now...
		 * 	StartSessions
		 * 	WaitForStopSessions
		 */

		/*
		 * Skip to here on failure for now. Will perhaps add
		 * intermediate retries until some threshold of the
		 * current period.
		 */
next_test:
		;

	}while(0); /* TODO: test for "next" interval time */


	exit(0);
}
