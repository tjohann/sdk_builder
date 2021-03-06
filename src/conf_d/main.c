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

static char *program_name;
static int running_mode = RUN_AS_APPLICATION;

static void
__attribute__((noreturn)) usage(int status)
{
	putchar('\n');
	fprintf(stdout, _("Usage: %s [options]              \n"), program_name);
	fprintf(stdout, _("Options:                                       \n"));

	fprintf(stdout, _("        -f filename.conf   \
give a name for a configuration file [mandatory]      \n"));
	fprintf(stdout, _("        -d directory       \
give a directory name to search for configuration file\n"));
	fprintf(stdout, _("        -h                 \
show help                                             \n"));
	fprintf(stdout, _("        -a                 \
run as application in foreground                      \n"));
	fprintf(stdout, _("        -s                 \
run as service/daemon in background (syslog enabled)  \n"));
	putchar('\n');
	fprintf(stdout, _("Examples:                                      \n"));
	fprintf(stdout, _("Give configuration as argument:                     \
%s -d -f a20_sdk_builder.conf                         \n"), program_name);
	fprintf(stdout, _("-> check path /etc/sdk_builder                 \n"));
	fprintf(stdout, _("-> check path /usr/local/etc/sdk_builder       \n"));
	fprintf(stdout, _("Give both as argument:                              \
%s -f a20_sdk_builder.conf -d $HOME/etc/sdk-builder/  \n"), program_name);
	putchar('\n');

	exit(status);
}


static void
cleanup(void)
{
	fprintf(stdout, _("Finalize cleanup -> cheers %s\n"), getenv("USER"));
}


static void
show_all_infos()
{
	/*
	 * show some useful info
	 */
	show_program_name(program_name);
	show_package_name();
	show_version_info();
	show_config();
}


static int
read_complete_config(char *conf_file, char *conf_dir)
{
	if (init_main_config(conf_file, conf_dir) != 0) {
		error_msg(_("ERROR main sdk_config"));
		return -1;
	}

	if (init_common_config(conf_file, conf_dir) != 0)
		error_msg(_("ERROR common sdk_config done"));

	if (init_repo_config(conf_file, conf_dir) != 0)
		error_msg(_("ERROR repo sdk_config"));

	if (init_toolchain_config(conf_file, conf_dir) != 0)
		error_msg(_("ERROR toolchain sdk_config"));

	if (init_device_config(conf_file, conf_dir) != 0)
		error_msg(_("ERROR device sdk_config"));


	if (init_external_config(conf_file, conf_dir) != 0)
		error_msg(_("ERROR external sdk_config"));

	if (init_kernel_config(conf_file, conf_dir) != 0)
		error_msg(_("ERROR kernel sdk_config"));

	return 0;
}


int
main(int argc, char *argv[])
{
	char *conf_file = NULL;
	char *conf_dir = NULL;
	int c;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	set_program_name(&program_name, argv[0]);

	while ((c = getopt(argc, argv, "hasf:d:")) != -1) {
		switch (c) {
		case 'a':
			running_mode = RUN_AS_APPLICATION;
			break;
		case 's':
                        running_mode = RUN_AS_DAEMON;
			break;
		case 'f':
			conf_file = optarg;
			break;
		case 'd':
			conf_dir = optarg;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			fprintf(stderr, _("ERROR: no valid argument\n"));
			usage(EXIT_FAILURE);
		}
	}

	if (atexit(cleanup) != 0)
		exit(EXIT_FAILURE);

	if (conf_file == NULL)
		usage(EXIT_FAILURE);

	if (read_complete_config(conf_file, conf_dir) == 0) {
		info_msg(_("Init main sdk_config done"));
	} else {
		error_msg(_("Possible init problem: read_complete_config != 0"));
		usage(EXIT_FAILURE);
	}

	if ((is_this_a_dir(get_common_workdir()) == false) ||
	    (is_this_a_dir(get_common_workdir()) == false))
		usage(EXIT_FAILURE);

	if ((read_checksum_file()) == 0)
		info_msg(_("Read checksum file done"));
	else
		info_msg(_("%s not available or not valid"),
			 NAME_CHECKSUM_FILE);

	/*
	 * show some useful info
	 */
	show_all_infos();

	/*
	 * daemon stuff
	 */

	if (running_mode == RUN_AS_DAEMON) {
		if (become_daemon() != 0) {
			error_exit(_("become_daemon() != 0"));
		} else {
			info_msg(_("now i'm a daemon"));
		}
	} else {
		info_msg(_("run in foreground as application"));
	}




	// only for testing
	sleep(20);

	return EXIT_SUCCESS;
}
