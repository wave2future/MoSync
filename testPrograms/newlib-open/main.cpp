/* Copyright (C) 2010 MoSync AB

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.
*/

#include <ma.h>
#include <maassert.h>
#include <conprint.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <errno.h>

int rc;
const char *dbname = "/my_db.sql3";

int MAMain() {
	rc = open( dbname, O_WRONLY | O_CREAT);
	fprintf(stderr, "rc= %-3d %i %s \n", rc, errno, dbname);
	close(rc);
	FREEZE;
	return 0;
}
