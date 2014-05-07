
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
#define files_h " @(#) files.h:1.8 1/22/96 00:00:00"

source_file
	* add_c(),
	*benchmark_c(),
	*creat_clo_c(),
	*disk1_c(),
	*disk_src_c(),
	*div_c(),
	*fillin_c(),
	*funcal_c(),
	*int_fcns_c(),
	*mul_c(),
	*num_fcns_c(), *pipe_test_c(), *ram_c(), *rand_c(), *rtmsec_c();

static source_file *(*source_files[]) () = {
add_c, benchmark_c,
		creat_clo_c,
		disk1_c,
		disk_src_c,
		div_c,
		fillin_c,
		funcal_c,
		int_fcns_c,
		mul_c, num_fcns_c, pipe_test_c, ram_c, rand_c, rtmsec_c,};
