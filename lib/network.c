/*
  LGPL
  (c) 2016, thorsten.johannvorderbrueggen@t-online.de

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "libservice.h"

#define USE_PID 0x01
#define USE_BIND 0x02
#define USE_CONNECT 0x04

// buffer 10 connection
#define BACKLOG 10

static int
uds_socket(const char *name, const char *dir, char **socket_f, int type,
	   unsigned char flags)
{
	if (name == NULL || dir == NULL)
		return -1;

	char *sfd_f = NULL;
	int sfd = -1;

	int n = -1;
	char str[MAXLINE];
	memset(str, 0, MAXLINE);
	if (flags & USE_PID)
		n = snprintf(str, MAXLINE,"%s/%s.%ld", dir, name,
			     (long) getpid());
	else
		n = snprintf(str, MAXLINE,"%s/%s.%s", dir, name,
			     UDS_NAME_ADD);

	if ((unlink(str) == -1) && (errno != ENOENT))
		error_exit(_("couln't unlink %s"), str);

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(struct sockaddr_un));
	if (strlen(str) > sizeof(addr.sun_path)) {
		info_msg(_("strlen(str) > sizeof(add.sun_path) in %s"),
			 __FUNCTION__);
		return -1;
	}

#ifdef __DEBUG__
	info_msg(_("use %s as socket"), str);
#endif
	sfd = socket(AF_UNIX, type, 0);
	if (sfd == -1)
		error_exit(_("could not open socket in %s"), __FUNCTION__);

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, str, sizeof(addr.sun_path) - 1);

	if (flags & USE_BIND)
		if (bind(sfd, (struct sockaddr *) &addr, SUN_LEN(&addr)) < 0)
			error_exit(_("could not bind socket"));

	if (flags & USE_CONNECT)
		if (connect(sfd, (struct sockaddr *) &addr,
			    sizeof(struct sockaddr_un)) < 0)
			error_exit(_("could not connect socket"));

	sfd_f = malloc(n + 1);
	if (sfd_f == NULL)
		error_exit(_("sfd_f == NULL %s"), str);

	memset(sfd_f, 0, n + 1);
	strncpy(sfd_f, str, n);
	*socket_f = sfd_f;

	return sfd;
}

int
uds_dgram_server(const char *name, const char *dir, char **socket_f)
{
	return uds_socket(name, dir, socket_f, SOCK_DGRAM, USE_BIND);
}

int
uds_stream_server(const char *name, const char *dir, char **socket_f)
{
	int sfd = uds_socket(name, dir, socket_f, SOCK_STREAM, USE_BIND);

	if (listen(sfd, BACKLOG) == -1)
		error_exit(_("listen in %s"), __FUNCTION__);

	return sfd;
}

int
uds_dgram_client(const char *name, const char *dir, char **socket_f)
{
	return uds_socket(name, dir, socket_f, SOCK_DGRAM, USE_PID | USE_BIND);
}

int
uds_stream_client(const char *name, const char *dir, char **socket_f)
{
	return uds_socket(name, dir, socket_f, SOCK_DGRAM, USE_PID | USE_CONNECT);
}

int
unlink_uds(int sfd)
{
	struct sockaddr_un addr;
	socklen_t len = sizeof(struct sockaddr_un);

	if (getsockname(sfd, (struct sockaddr *) &addr, &len ) == -1)
		error_exit(_("could not get sun_path in %s"), __FUNCTION__);

	if (unlink(addr.sun_path) == -1) {
		if (errno == ENOENT)
			info_msg(_("no such file %s"), addr.sun_path);
		else
			error_exit(_("could not unlink %s in %s"),
				   addr.sun_path, __FUNCTION__);
	}

	return 0;
}

char *
get_uds_name_s(const char *file, const char *dir)
{
	if ((file == NULL) || (dir == NULL ))
		return NULL;

	char tmp_s[MAXLINE];
	memset(tmp_s, 0, MAXLINE);

	int n = snprintf(tmp_s, MAXLINE,"%s/%s", dir, file);

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(struct sockaddr_un));
	if (strlen(tmp_s) > sizeof(addr.sun_path)) {
		info_msg(_("strlen(tmp_s) > sizeof(add.sun_path) in %s"),
			 __FUNCTION__);
		return NULL;
	}

	char *str = malloc(n + 1);
	if (str == NULL)
		error_exit(_("str == NULL %s"), tmp_s);

	memset(str, 0, n + 1);
	strncpy(str, tmp_s, n);

#ifdef __DEBUG__
	info_msg(_("assembled name %s"), str);
#endif
	return str;
}
