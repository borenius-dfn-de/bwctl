/*
 *      $Id$
 */
/*
 *	File:		bwctldP.h
 *
 *	Author:		Jeff W. Boote
 *			Internet2
 *
 *	Date:		Tue Sep  9 16:07:42 MDT 2003
 *
 *	Description:	
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
#ifndef	_BWCTLDP_H_
#define	_BWCTLDP_H_

#ifndef	BWCTLD_CONF_FILE
#define	BWCTLD_CONF_FILE	"bwctld.conf"
#endif

#define	BWCTLD_IPERF_DEF_TESTPORT	(5001)
#define	BWCTLD_THRULAYD_DEF_TESTPORT	(5003)

/*
 * Types
 */
typedef struct {
    I2Boolean	    help;

    char	    cwd[MAXPATHLEN];
    char	    *confdir;
    char	    *vardir;

    char	    *authmode;
    uint32_t	    auth_mode;	/* cooked version of authmode */
    char	    *srcnode;

    char	    *user;
    char	    *group;
    I2Boolean       allowRoot;

    uint32_t	    dieby;
    uint32_t	    controltimeout;
    I2Boolean	    daemon;

    char            **posthook;
    int             posthook_count;
} bwctld_opts;

#endif	/*	_BWCTLDP_H_	*/
