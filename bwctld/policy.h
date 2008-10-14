/*
 *      $Id$
 */
/*
 *	File:		policy.h
 *
 *	Author:		Jeff W. Boote
 *			Internet2
 *
 *	Date:		Tue Sep 16 14:29:07 MDT 2003
 *
 *	Description:	
 *			This file declares the types needed by applications
 *			to use the "default" 
 *
 *
 *    License:
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the following copyright notice,
 *       this list of conditions and the disclaimer below.
 * 
 *        Copyright (c) 2003-2008, Internet2
 * 
 *                              All rights reserved.
 * 
 *     * Redistribution in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 * 
 *    *  Neither the name of Internet2 nor the names of its contributors may be
 *       used to endorse or promote products derived from this software without
 *       explicit prior written permission.
 * 
 * You are under no obligation whatsoever to provide any enhancements to Internet2,
 * or its contributors.  If you choose to provide your enhancements, or if you
 * choose to otherwise publish or distribute your enhancement, in source code form
 * without contemporaneously requiring end users to enter into a separate written
 * license agreement for such enhancements, then you thereby grant Internet2, its
 * contributors, and its members a non-exclusive, royalty-free, perpetual license
 * to copy, display, install, use, modify, prepare derivative works, incorporate
 * into the software or other computer software, distribute, and sublicense your
 * enhancements or derivative works thereof, in binary and source code form.
 * 
 * DISCLAIMER - THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * “AS IS” AND WITH ALL FAULTS.  THE UNIVERSITY OF DELAWARE, INTERNET2, ITS CONTRI-
 * BUTORS, AND ITS MEMBERS DO NOT IN ANY WAY WARRANT, GUARANTEE, OR ASSUME ANY RES-
 * PONSIBILITY, LIABILITY OR OTHER UNDERTAKING WITH RESPECT TO THE SOFTWARE. ANY E-
 * XPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRAN-
 * TIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT
 * ARE HEREBY DISCLAIMED AND THE ENTIRE RISK OF SATISFACTORY QUALITY, PERFORMANCE,
 * ACCURACY, AND EFFORT IS WITH THE USER THEREOF.  IN NO EVENT SHALL THE COPYRIGHT
 * OWNER, CONTRIBUTORS, OR THE UNIVERSITY CORPORATION FOR ADVANCED INTERNET DEVELO-
 * PMENT, INC. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTIT-
 * UTE GOODS OR SERVICES; REMOVAL OR REINSTALLATION LOSS OF USE, DATA, SAVINGS OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILIT-
 * Y, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHE-
 * RWISE) ARISING IN ANY WAY OUT OF THE USE OR DISTRUBUTION OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#ifndef	_BWL_DEFAULTS_H
#define	_BWL_DEFAULTS_H

#include <I2util/util.h>
#include <bwlib/bwlib.h>

#ifndef	BWL_KEY_FILE
#define	BWL_KEY_FILE		"bwctld.keys"
#endif

#ifndef	BWL_LIMITS_FILE
#define	BWL_LIMITS_FILE		"bwctld.limits"
#endif

/*
 * Holds the policy record that was parsed and contains all the "limits"
 * and identity information.
 *
 * type: (owp_policy_data*) - defined in access.h
 * location: Context Config
 */
#define BWLDPOLICY	"V.BWLDPOLICY"

/*
 * Holds the identifying "node" from the policy tree that contains the
 * class and limits information for the given control connection.
 *
 * type: (owp_tree_node_ptr) - defined in access.h
 * location: Control Config
 */
#define BWLDPOLICY_NODE	"V.BWLDPOLICY_NODE"

/*
 * Types used by policy functions
 */
#define BWLDMAXCLASSLEN	(80)

typedef struct BWLDPolicyRec BWLDPolicyRec, *BWLDPolicy;
typedef struct BWLDPolicyNodeRec BWLDPolicyNodeRec, *BWLDPolicyNode;
typedef struct BWLDPolicyKeyRec BWLDPolicyKeyRec, *BWLDPolicyKey;

struct BWLDPolicyRec{
    BWLContext	    ctx;

    int		    fd;	/* socket to parent. */

    int		    *retn_on_intr;	/* If one, exit I/O on sigs */

    BWLDPolicyNode  root;

    /* limits:
     * 	key = char* (classname from "limit" lines)
     * 	val = BWLDPolicyNode
     */
    I2Table	    limits;

    /* idents:
     * 	key = BWLDPid
     * 	val = BWLDPolicyNode
     */
    I2Table	    idents;

    /* keys:
     * 	key = uint8_t[16]	(username from bwlib protocol)
     * 	val = BWLKey
     */
    I2Table	    keys;

};

typedef I2numT      BWLDLimitT;		/* values */
typedef uint32_t    BWLDMesgT;

typedef struct BWLDLimRec{
    BWLDMesgT	limit;
    BWLDLimitT	value;
} BWLDLimRec;

/* parent		cname		*/
/* bandwidth		uint (bits/sec)*/
/* allow_open_mode	(on)/off	*/

#define	BWLDLimParent		0
#define	BWLDLimBandwidth	1
#define	BWLDLimPending		2
#define	BWLDLimEventHorizon	3
#define	BWLDLimDuration		4
#define	BWLDLimAllowOpenMode	5
#define	BWLDLimAllowTCP		6
#define	BWLDLimAllowUDP		7
#define	BWLDLimMaxTimeError	8

struct BWLDPolicyNodeRec{
    BWLDPolicy	    policy;
    char	    *nodename;
    BWLDPolicyNode  parent;
    size_t	    ilim;
    BWLDLimRec	    *limits;
    BWLDLimRec	    *used;
};

typedef enum{
    BWLDPidInvalid=0,
    BWLDPidDefaultType,
    BWLDPidNetmaskType,
    BWLDPidUserType
} BWLDPidType;

typedef struct{
    BWLDPidType	id_type;
    uint8_t	mask_len;
    size_t	addrsize;
    uint8_t	addrval[16];
} BWLDPidNetmask;

typedef struct{
    BWLDPidType	id_type;
    BWLUserID	userid;
} BWLDPidUser;

typedef union BWLDPidUnion{
    BWLDPidType	    id_type;
    BWLDPidNetmask  net;
    BWLDPidUser	    user;
} BWLDPidRec, *BWLDPid;

/*
 * The following section defines the message tags used to communicate
 * from the children processes to the parent to request/release
 * resources on a global basis.
 *
 * All message "type" defines will be of type BWLDMesgT.
 */
#define	BWLDMESGMARK		0xfefefefe
#define	BWLDMESGCLASS		0xcdef
#define	BWLDMESGRESOURCE	0xbeef
#define	BWLDMESGRESERVATION	0xdeadbeef
#define	BWLDMESGCOMPLETE	0xabcdefab
#define	BWLDMESGREQUEST		0xfeed
#define	BWLDMESGRELEASE		0xdead
#define	BWLDMESGCLAIM		0x1feed1

/*
 * "parent" response messages will be one of:
 */
#define BWLDMESGINVALID	0x0
#define BWLDMESGOK	0x1
#define BWLDMESGDENIED	0x2

/*
 * After forking, the new "server" process (called "child" in the following)
 * should determine the "usage class" the given connection should belong to.
 * The first message to the "parent" master process should communicate this
 * information so that all further resource requests/releases are relative
 * to that "usage class". The format of this message should be as follows:
 *
 * (All integers are in host order since this is expected to be ipc
 * communication on a single host. It could be a future enhancement to
 * allow a "single" distributed bwctld BWLIB-Control server to manage
 * multiple test  endpoints at which time it might be worth the overhead
 * to deal with byte ordering issues.)
 *
 * Initial child->parent message:
 *
 * 	   0                   1                   2                   3
 * 	   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	00|                      BWLDMESGMARK                             |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	04|                      BWLDMESGCLASS                            |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	            [nul terminated ascii string of classname]
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	00|                      BWLDMESGMARK                             |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * This is a child message format that is used to request or release resources.
 * (The parent should release all "temporary" resources (i.e. bandwidth)
 * on exit of the child if the child does not explicitly release the resource.
 * More "permenent" resources should only be released explicitly
 * (i.e. disk-space).
 *
 * 	   0                   1                   2                   3
 * 	   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	00|                      BWLDMESGMARK                             |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	04|                     BWLDMESGRESOURCE                          |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	08|          BWLDMESGWANT|BWLDMESGRELEASE|BWLDMESGCLAIM           |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	12|                      BWLDMesgT(limit name)                    |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	16|                        BWLDLimitT                             |
 *	20|                                                               |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	24|                      BWLDMESGMARK                             |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * This is a child message format that is used to declare a test complete.
 * (The Accept Value will be 0 if the test was successful.)
 *
 * 	   0                   1                   2                   3
 * 	   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	00|                         BWLDMESGMARK                          |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	04|                       BWLDMESGCOMPLETE                        |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	08|                                                               |
 *	12|                              SID                              |
 *	16|                                                               |
 *	20|                                                               |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	24|                          Accept Value                         |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *	28|                          BWLDMESGMARK                         |
 *	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
* Parent responses to the previous two messages are of the format:
*
* 	   0                   1                   2                   3
* 	   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	00|                      BWLDMESGMARK                             |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	04|                BWLDMESGOK|BWLDMESGDENIED                      |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	08|                      BWLDMESGMARK                             |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*
* This is a child message format that is used to request reservations.
*
* 	   0                   1                   2                   3
* 	   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	00|                         BWLDMESGMARK                          |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	04|                      BWLDMESGRESERVATION                      |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	08|                                                               |
*	12|                              SID                              |
*	16|                                                               |
*	20|                                                               |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	24|                        Request Time                           |
*	28|                                                               |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	32|                           Fuzz TIME                           |
*	36|                                                               |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	40|                          Latest Time                          |
*	44|                                                               |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	48|                            Duration                           |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	52|                           RTT TIME                            |
*	56|                                                               |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	60|           Recv Port           |             Unused            |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	64|                            Tool_id                            |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	68|                          BWLDMESGMARK                         |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*
* Parent responses to the reservation request are of the format:
*
* 	   0                   1                   2                   3
* 	   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	00|                          BWLDMESGMARK                         |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	04|                   BWLDMESGOK|BWLDMESGDENIED                   |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	08|                        Request Time                           |
*	12|                                                               |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	16|           Recv Port             |            Unused           |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*	20|                          BWLDMESGMARK                         |
*	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*
*
*/

/*
 * The following api convienence functions are defined to make the child/parent
 * communication easier. (These are the functions needed by the parent in
 * the master bwctld "resource broker" process.)
 */

extern BWLDPolicyNode
BWLDReadClass(
        BWLDPolicy	policy,
        int		fd,
        int		*retn_on_intr,
        int		*err
        );

/*
 * Returns the request type or 0.
 */
extern int
BWLDReadReqType(
        int	fd,
        int	*retn_on_intr,
        int	*err);
/*
 * returns True on success - query/lim_ret will contain request
 * err will be non-zero on error. 0 on empty read.
 */
extern BWLBoolean
BWLDReadQuery(
        int		fd,
        int		*retn_on_intr,
        BWLDMesgT	*query,
        BWLDLimRec	*lim_ret,
        int		*err
        );

extern BWLBoolean
BWLDReadReservationQuery(
        int		fd,
        int		*retn_on_intr,
        BWLSID		sid,
        BWLNum64	*req_time,
        BWLNum64	*fuzz_time,
        BWLNum64	*last_time,
        uint32_t	*duration,
        BWLNum64	*rtt_time,
        uint16_t	*toolport,
        BWLToolType     *tool_id,
        int		*err
        );

extern BWLBoolean
BWLDReadTestComplete(
        int		fd,
        int		*retn_on_intr,
        BWLSID		sid,
        BWLAcceptType	*aval,
        int		*err
        );

extern int
BWLDSendResponse(
        int		fd,
        int		*retn_on_intr,
        BWLDMesgT	mesg
        );

extern int
BWLDSendReservationResponse(
        int		fd,
        int		*retn_on_intr,
        BWLDMesgT	mesg,
        BWLNum64	reservation,
        uint16_t	toolport
        );

/*
 * This function is used to add/subtract resource allocations from the
 * current tree of resource usage. (It is also used for "fixed" value
 * resouces to determine if the request is valid or not. For "fixed"
 * value resources, the current "usage" is not tracked.)
 */
extern BWLBoolean
BWLDResourceDemand(
        BWLDPolicyNode	node,
        BWLDMesgT	query,
        BWLDLimRec	lim
        );

/*
 * This function is used to return the "fixed" limit defined for a
 * given node for a particular resource. It returns True if it was
 * able to fetch the value. (It should only return False if called
 * for a non-fixed value resource.)
 */
extern BWLBoolean
BWLDGetFixedLimit(
        BWLDPolicyNode	node,
        BWLDMesgT	limname,
        BWLDLimitT	*ret_val
        );
/*
 * Functions called directly from bwctld regarding "policy" decisions
 * (If false, check err_ret to determine if it is an "error" condition,
 * or if open_mode is simply denied.)
 */
extern BWLBoolean
BWLDAllowOpenMode(
        BWLDPolicy	policy,
        struct sockaddr	*peer_addr,
        BWLErrSeverity	*err_ret
        );

/*
 * Functions actually used to install policy hooks into libbwlib.
 */
extern BWLBoolean
BWLDGetAESKey(
        BWLContext	ctx,
        const BWLUserID	userid,
        uint8_t	*key_ret,
        BWLErrSeverity	*err_ret
        );

extern BWLBoolean
BWLDCheckControlPolicy(
        BWLControl	cntrl,
        BWLSessionMode	mode,
        const BWLUserID	userid,
        struct sockaddr	*local_saddr,
        struct sockaddr	*remote_saddr,
        BWLErrSeverity	*err_ret
        );

extern BWLBoolean
BWLDCheckTestPolicy(
        BWLControl	cntrl,
        BWLSID		sid,
        BWLBoolean	local_sender,
        struct sockaddr	*local_saddr,
        struct sockaddr	*remote_saddr,
        socklen_t	sa_len,
        BWLTestSpec	*tspec,
        BWLNum64	fuzz_time,
        BWLNum64	*reservation_ret,
        uint16_t	*tool_port_ret,
        void		**closure,
        BWLErrSeverity	*err_ret
        );

extern void
BWLDTestComplete(
        BWLControl	cntrl,
        void		*closure,
        BWLAcceptType	aval
        );

extern BWLDPolicy
BWLDPolicyInstall(
        BWLContext	ctx,
        char		*confdir,	/* conf dir for policy		*/
        int		*retn_on_intr,
        char		**lbuf,
        size_t		*lbuf_max
        );

#endif	/*	_BWL_DEFAULTS_H	*/
