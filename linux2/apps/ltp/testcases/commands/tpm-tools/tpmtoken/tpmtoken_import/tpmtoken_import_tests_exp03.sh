#! /usr/bin/expect -f
#
# Copyright (c) International Business Machines  Corp., 2005
#
# This program is free software;  you can redistribute it and#or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program;  if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

set P11_SO_PWD $env(P11_SO_PWD)
set P11_USER_PWD $env(P11_USER_PWD)
set TPM_CERTFILE $env(TPM_CERTFILE)
set TPM_KEYFILE $env(TPM_KEYFILE)
set TPM_COMBFILE $env(TPM_COMBFILE)
set SSL_PWD $env(SSL_PWD)
set TCID $env(TCID)
set timeout 30

# Import the combined certificate and key
#   there should already be a cert and key objects,
#   but the -y option takes care of that
spawn tpmtoken_import -y -n "$TCID" $TPM_COMBFILE
expect -re "Enter PEM pass phrase:"
send "$SSL_PWD\n"
expect -re "Enter your TPM user password: "
send "$P11_USER_PWD\n"
expect timeout

set rc_list [wait -i $spawn_id]
set rc [lindex $rc_list {3}]

exit $rc
