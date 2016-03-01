/*
  GPL
  (c) 2016, thorsten.johannvorderbrueggen@t-online.de

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "common.h"


void *
clone_sdk_repo(void *args)
{
	char *url = get_sdk_repo_url();
	char *path = get_sdk_repo_path();

	(void) args; // not used

	if (url == NULL || path == NULL)
		return NULL;

	enter_sdk_thread();

	if (create_progressbar_window(path) != 0)
		write_error_msg(_("ERROR: create_progress_bar_window != 0"));

	do_clone_repo(url, path);

	set_progressbar_value(100, "100%");
	unlock_button(PROGRESSBAR_B);

	if (is_checksum_array_valid())
		write_info_msg(_("checksum_array is valid"));
	else
		if (read_checksum_file() != 0)
			write_error_msg(_("Can't init checksum structure"));

	leave_sdk_thread();

	return NULL;
}


void *
update_sdk_repo(void *args)
{

	char *url = get_sdk_repo_url();
	char *path = get_sdk_repo_path();

	(void) args; // not used

	if (url == NULL || path == NULL)
		return NULL;

	enter_sdk_thread();

	if (create_progressbar_window(path) != 0)
		write_error_msg(_("ERROR: create_progress_bar_window != 0"));

	do_update_repo(url, path);

	set_progressbar_value(100, "100%");
	unlock_button(PROGRESSBAR_B);

	leave_sdk_thread();

	return NULL;
}


void *
download_toolchain(void *args)
{
	checksum_tupel_t *c = NULL;

	download_tupel_t *download_array[MAX_COUNT_TOOLCHAIN + 1];
	memset(download_array, 0, sizeof(download_array));

	(void) args;

	enter_sdk_thread();

	/*
	 * toolchain -> cross-compiler + sysroot
	 * host -> host related tools like mkimage (source of sysroot)
	 */
	download_array[0] = get_toolchain_tupel();
	download_array[1] = get_host_tupel();

	if (create_progressbar_window(_("Download toolchain/host")) != 0)
		write_error_msg(_("ERROR: create_progress_bar_window != 0"));

	int i = 0;
	char *name = NULL;
	for (;;) {
		if (download_array[i] == NULL)
			break;

		set_progressbar_window_title(get_download_tupel_path(download_array[i]));
		set_progressbar_value(0, "0%");

		if (do_download_tupel(download_array[i]) == -1)
			break;

		set_progressbar_value(100, _("!! calc checksum !!"));

		if (calc_checksum(download_array[i]) != 0)
			write_error_msg("Possible ERROR -> calc_checksum != 0");

		name = strrchr(download_array[i]->path, '/');
		if (name == NULL)
			name = download_array[i]->path;
		else
			name++;

#ifdef DEBUG
		PRINT_LOCATION();
		info_msg(_("download_tupel->path extracted filename %s"), name);
#endif

		c = get_checksum_tupel(name);
		if (c == NULL) {
			write_error_msg("Possible ERROR -> c == NULL");
			break;
		}

#ifdef DEBUG
		PRINT_LOCATION();
		info_msg(_("c->name: %s with c->checksum_s: %s"),
			 c->name, c->checksum_s);
#endif

		if (strncmp(download_array[i]->checksum_s, c->checksum_s,
			    strlen(download_array[i]->checksum_s)) == 0) {
			info_msg(_("checksum is okay"));
		} else {
			write_error_msg(_("checksum is NOT okay"));
			break;
		}

		set_progressbar_value(100, _("!! extracting !!"));

		write_info_msg(_("Extrating file: %s"), download_array[i]->path);
		if (extract_toolchain(download_array[i]->path) == -1)
			break;
		i++;
	}

	leave_sdk_thread();

	set_progressbar_value(100, "100%");
	set_progressbar_window_title(_("Download toolchain/host"));
	unlock_button(PROGRESSBAR_B);

	return NULL;
}


void *
init_sdk_workdir(void *args)
{
	PRINT_LOCATION();

	(void) args;

	write_info_msg(_("--INFO_MSG--: in init_workdir"));

	if (check_sdk_workdir() == 0)
		write_error_msg(_("%s is available"), get_common_workdir());
	else {
		write_error_msg(_("%s isn't available, create it"),
				get_common_workdir());
		// TODO: create workdir
	}

	if (check_sdk_runtimedir() == 0)
		write_error_msg(_("%s is available"), get_common_runtimedir());
	else {
		write_error_msg(_("%s isn't available, create it"),
				get_common_runtimedir());
		// TODO: create runtimedir
	}

	return NULL;
}


void *
config_sdk(void *args)
{
	PRINT_LOCATION();

	(void) args;

	write_info_msg(_("--INFO_MSG--: in config_sdk"));

	return NULL;
}


void *
test_sdk(void *args)
{

	(void) args;

	/*
	 * only one thread could be active
	 */
	enter_sdk_thread();

	write_info_msg(_("--INFO_MSG--: in test_sdk"));

	sleep(5);

        /*
	 * check for correct state
	 */
	leave_sdk_thread();

	return NULL;
}


void *
help(void *args)
{
	PRINT_LOCATION();

	(void) args;

	write_info_msg(_("--INFO_MSG--: in help"));

	return NULL;
}
