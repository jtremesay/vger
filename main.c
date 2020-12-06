#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "mimes.h"

#define BUFF_LEN_2	 1025
#define BUFF_LEN_3	 1024
#define GEMINI_PART	 9
#define DEFAULT_LANG	 "en"
#define DEFAULT_CHROOT	 "/var/gemini/"


void 		display_file(const char *, const char *);
void 		status   (const int, const char *, const char *);
void 		drop_privileges(const char *, const char *);

void
drop_privileges(const char *user, const char *path)
{
	struct passwd  *pw;

	/*
	 * use chroot() if an user is specified requires root user to be
	 * running the program to run chroot() and then drop privileges
	 */
	if (strlen(user) > 0) {

		/* is root? */
		if (getuid() != 0) {
			syslog(LOG_DAEMON, "chroot requires program to be run as root");
			errx(1, "chroot requires root user");
		}
		/* search user uid from name */
		if ((pw = getpwnam(user)) == NULL) {
			syslog(LOG_DAEMON, "the user %s can't be found on the system", user);
			err(1, "finding user");
		}
		/* chroot worked? */
		if (chroot(path) != 0) {
			syslog(LOG_DAEMON, "the chroot_dir %s can't be used for chroot", path);
			err(1, "chroot");
		}
		if (chdir("/") == -1) {
			syslog(LOG_DAEMON, "failed to chdir(\"/\")");
			err(1, "chdir");
		}
		/* drop privileges */
		if (setgroups(1, &pw->pw_gid) ||
		    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
		    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid)) {
			syslog(LOG_DAEMON, "dropping privileges to user %s (uid=%i) failed",
			       user, pw->pw_uid);
			err(1, "Can't drop privileges");
		}
		path = "/";
	}
#ifdef __OpenBSD__
	/*
    	 * prevent access to files other than the one in path
    	 */
	if (unveil(path, "r") == -1) {
		syslog(LOG_DAEMON, "unveil on %s failed", path);
		err(1, "unveil");
	}
	/*
	 * prevent system calls other parsing queryfor fread file and
	 * write to stdio
	 */
	if (pledge("stdio rpath", NULL) == -1) {
		syslog(LOG_DAEMON, "pledge call failed");
		err(1, "pledge");
	}
#endif

}

void
status(const int code, const char *file_mime, const char *lang)
{
	printf("%i %s; lang=%s\r\n",
	       code, file_mime, lang);
}

void
display_file(const char *path, const char *lang)
{
	FILE		*fd;
	struct stat	 sb;
	ssize_t		 nread;
	char		*buffer[BUFSIZ];
	char		 extension[10];
	const char	*file_mime;

	/* this is to check if path is a directory */
	if (stat(path, &sb) == -1)
		goto err;

	/* open the file requested */
	if ((fd = fopen(path, "r")) == NULL)
		goto err;

	/* check if directory */
	if (S_ISDIR(sb.st_mode) == 1)
		goto err;

	file_mime = get_file_mime(path);

	status(20, file_mime, lang);

	/* read the file and write it to stdout */
	while ((nread = fread(buffer, sizeof(char), sizeof(buffer), fd)) != 0)
		fwrite(buffer, sizeof(char), nread, stdout);
	fclose(fd);
	syslog(LOG_DAEMON, "path served %s", path);

	return;
err:
	/* return an error code and no content */
	status(40, "text/gemini", lang);
	syslog(LOG_DAEMON, "path invalid %s", path);
}

int
main(int argc, char **argv)
{
	char 		buffer   [BUFF_LEN_2];
	char 		request  [BUFF_LEN_2];
	char 		hostname [BUFF_LEN_2];
	char 		file     [BUFF_LEN_2];
	char 		path     [BUFF_LEN_2] = DEFAULT_CHROOT;
	char 		lang     [3] = DEFAULT_LANG;
	char 		user     [_SC_LOGIN_NAME_MAX] = "";
	int 		virtualhost = 0;
	int 		option;
	int 		chroot = 0;
	int 		start_with_gemini;
	char           *pos;

	while ((option = getopt(argc, argv, ":d:l:u:v")) != -1) {
		switch (option) {
		case 'd':
			strlcpy(path, optarg, sizeof(path));
			break;
		case 'v':
			virtualhost = 1;
			break;
		case 'l':
			strlcpy(lang, optarg, sizeof(lang));
			break;
		case 'u':
			strlcpy(user, optarg, sizeof(user));
			chroot = 1;
			break;
		}
	}

	/*
	 * do chroot if an user is supplied run pledge/unveil if OpenBSD
	 */
	drop_privileges(user, path);

	/* change basedir to / to build the filepath if we use chroot */
	if (chroot == 1)
		strlcpy(path, "/", sizeof(path));

	/*
	 * read 1024 chars from stdin
	 * to get the request
	 */
	fgets(request, BUFF_LEN_3, stdin);

	/* remove \r\n at the end of string */
	pos = strchr(request, '\r');
	if (pos != NULL)
		*pos = '\0';

	/*
	 * check if the beginning of the request starts with
	 * gemini://
	 */
	start_with_gemini = strncmp(request, "gemini://", 9);

	/* the request must start with gemini:// */
	if (start_with_gemini != 0) {
		/* error code url malformed */
		syslog(LOG_DAEMON, "request «%s» doesn't match gemini:// at index %i",
		       request, start_with_gemini);
		exit(1);
	}
	syslog(LOG_DAEMON, "request %s", request);

	/* remove the gemini:// part */
	strlcpy(buffer, request + GEMINI_PART, sizeof(buffer) - GEMINI_PART);
	strlcpy(request, buffer, sizeof(request));

	/*
	 * look for the first / after the hostname
	 * in order to split hostname and uri
	 */
	pos = strchr(request, '/');

	if (pos != NULL) {
		/* if there is a / found */
		int 		position = -1;
		for (int i = 0; i < sizeof(request); i++) {
			if (*pos == request[i]) {
				position = i;
				break;
			}
		}

		/* separate hostname and uri */
		if (position != -1) {
			strlcpy(hostname, request, position + 1);
			strlcpy(file, request + position + 1, sizeof(request));

			/*
			 * use a default file if no file are requested this
			 * can happen in two cases gemini://hostname/
			 * gemini://hostname/directory/
			 */
			if (strlen(file) == 0)
				strlcpy(file, "/index.gmi", 11);
			if (file[strlen(file) - 1] == '/')
				strlcat(file, "index.gmi", sizeof(file));

		} else {
			syslog(LOG_DAEMON, "unknown situation after parsing query");
			exit(2);
		}
	} else {
		/*
		 * there are no slash / in the request
		 * -2 to remove \r\n
		*/
		strlcpy(hostname, request, sizeof(hostname));
		strlcpy(file, "/index.gmi", 11);
	}

	/*
	 * if virtualhost feature is actived looking under the default path +
	 * hostname directory gemini://foobar/hello will look for
	 * path/foobar/hello
	 */
	if (virtualhost) {
		strlcat(path, hostname, sizeof(path));
		strlcat(path, "/", sizeof(path));
	}
	/* add the base dir to the file requested */
	strlcat(path, file, sizeof(path));

	/* open file and send it to stdout */
	display_file(path, lang);

	return (0);
}
