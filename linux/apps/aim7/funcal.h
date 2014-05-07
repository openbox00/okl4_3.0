
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
#ifndef	funcal_h
#	define funcal_h " @(#) funcal.h:1.2 1/22/96 00:00:00"
#endif

/*
 ** funcal.h
 **
 **      Common include file for Suite 3 benchmark.
 **
 */

/*
 ** HISTORY
 **
 ** 4/25/90 CTN Created
 **
 */

#if defined(FUNCAL)
#define EXTERN
#else
#define EXTERN          extern
#endif

EXTERN int fcal0(),
  fcal1(),
  fcal2(),
  fcal15(),
  fcalfake();

#undef EXTERN
