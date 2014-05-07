
/****************************************************************
**                                                             **
**    Copyright (c) 1996 - 2001 Caldera International, Inc.    **
**                    All Rights Reserved.                     **
**                                                             **
** This program is free software; you can redistribute it      **
** and/or modify it under the terms of the GNU General Public  **
** License as published by the Free Software Foundation;       **
** either version 2 of the License, or (at your option) any    **
** later version.                                              **
**                                                             **
** This program is distributed in the hope that it will be     **
** useful, but WITHOUT ANY WARRANTY; without even the implied  **
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR     **
** PURPOSE. See the GNU General Public License for more        **
** details.                                                    **
**                                                             **
** You should have received a copy of the GNU General Public   **
** License along with this program; if not, write to the Free  **
** Software Foundation, Inc., 59 Temple Place, Suite 330,      **
** Boston, MA  02111-1307  USA                                 **
**                                                             **
****************************************************************/
#ifndef	testerr_h
#	define testerr_h " @(#) testerr.h:1.2 1/22/96 00:00:00"
#endif

/*
 **
 ** testerr.h
 **
 */

#define BADTTY -1
#define BADMEM -2
#define BADTAPE -3
#define BADIOCTL -4
#define BADPIPE -5
#define BADPROC -6

extern long debug;

#define TTFIELD 	0x2
#define TPFIELD		0x4
#define VMFIELD		0x8
#define DEBUG(x)	if(debug) {{x;}fflush(stdout);fflush(stderr);}
#define TTDEBUG(x)	if(debug&TTFIELD) {{x;}fflush(stdout);fflush(stderr);}
#define TPDEBUG(x)	if(debug&TPFIELD) {{x;}fflush(stdout);fflush(stderr);}
#define VMDEBUG(x)	if(debug&VMFIELD) {{x;}fflush(stdout);fflush(stderr);}
