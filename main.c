#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "mimes.h"

#define GEMINI_PART	 9
#define DEFAULT_LANG	 "en"
#define DEFAULT_CHROOT	 "/var/gemini"
#define GEMINI_REQUEST_MAX 1024 /* see https://gemini.circumlunar.space/docs/specification.html */



void 		display_file(const char *, const char *);
void 		status(const int, const char *, const char *);
void		status_redirect(const int code, const char *url);
void 		drop_privileges(const char *, const char *);
void        eunveil(const char *path, const char *permissions);
size_t      estrlcat(char *dst, const char *src, size_t dstsize);
size_t      estrlcpy(char *dst, const char *src, size_t dstsize);

void
eunveil(const char *path, const char *permissions)
{
	if (unveil(path, permissions) == -1) {
		syslog(LOG_DAEMON, "unveil on %s failed", path);
		err(1, "unveil");
	}
}

size_t
estrlcpy(char *dst, const char *src, size_t dstsize)
{
	size_t n = 0;

	n = strlcpy(dst, src, dstsize);
	if (n >= dstsize) {
		err(1, "estrlcpy failed");	
	}

	return n;
}

size_t
estrlcat(char *dst, const char *src, size_t dstsize)
{
    size_t size;
    if ((size = strlcat(dst, src, dstsize)) >= dstsize)
        err(1, "strlcat");

    return size;
}

void
drop_privileges(const char *user, const char *path)
{
	struct passwd  *pw;
	int    chrooted = 0;

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
		chrooted = 1;
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
	}
#ifdef __OpenBSD__
	/*
	 * prevent access to files other than the one in path
	 */
	if (chrooted) {
		eunveil("/", "r");
	} else {
		eunveil(path, "r");
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
status_redirect(const int code, const char *url)
{
	printf("%i %s\r\n",
	       code, url);
}

void
display_file(const char *path, const char *lang)
{
	FILE		*fd = NULL;
	struct stat	 sb = {0};
	ssize_t		 nread = 0;
	char		*buffer[BUFSIZ];
	const char	*file_mime;
	char		target[FILENAME_MAX] = "";

	/* this is to check if path exists and obtain metadata later */
	if (stat(path, &sb) == -1) {

		/* check if path is a symbolic link
	         * if so, redirect using its target */
                if (lstat(path, &sb) != -1 && S_ISLNK(sb.st_mode) == 1)
		       	goto redirect;
                else
        		goto err;
	}


	/* check if directory */
	if (S_ISDIR(sb.st_mode) == 1)
		goto err;

	/* open the file requested */
	if ((fd = fopen(path, "r")) == NULL)
		goto err;

	file_mime = get_file_mime(path);

	status(20, file_mime, lang);

	/* read the file and write it to stdout */
	while ((nread = fread(buffer, sizeof(char), sizeof(buffer), fd)) != 0)
		fwrite(buffer, sizeof(char), nread, stdout);
	goto closefd;
	syslog(LOG_DAEMON, "path served %s", path);

	return;
err:
	/* return an error code and no content */
	status(51, "text/gemini", lang);
	syslog(LOG_DAEMON, "path invalid %s", path);
	goto closefd;

redirect:
    	/* read symbolic link target to redirect */
	if (readlink(path, target, FILENAME_MAX) == -1) {
    	     int  errnum = errno;
    	            fprintf(stderr, "Value of errno: %d\n", errno);
    	                  perror("Error printed by perror");
    	                        fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
     		goto err;
	}

    	status_redirect(30, target);
    	syslog(LOG_DAEMON, "redirection from %s to %s", path, target);

closefd:
    	if (S_ISREG(sb.st_mode) == 1) {
        	fclose(fd);
    	}
}

int
main(int argc, char **argv)
{
	char 		request  [GEMINI_REQUEST_MAX] = {'\0'};
	char 		hostname [GEMINI_REQUEST_MAX] = {'\0'};
	char 		file     [GEMINI_REQUEST_MAX] = {'\0'};
	char 		path     [GEMINI_REQUEST_MAX] = DEFAULT_CHROOT;
	char 		lang     [3] = DEFAULT_LANG;
	char 		user     [_SC_LOGIN_NAME_MAX] = "";
	int 		virtualhost = 0;
	int 		option = 0;
	int 		chroot = 0;
	char        *pos = NULL;

	while ((option = getopt(argc, argv, ":d:l:u:v")) != -1) {
		switch (option) {
		case 'd':
			estrlcpy(path, optarg, sizeof(path));
			break;
		case 'v':
			virtualhost = 1;
			break;
		case 'l':
			estrlcpy(lang, optarg, sizeof(lang));
			break;
		case 'u':
			estrlcpy(user, optarg, sizeof(user));
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
		estrlcpy(path, "/", sizeof(path));

	/*
	 * read 1024 chars from stdin
	 * to get the request
	 */
	if (fgets(request, GEMINI_REQUEST_MAX, stdin) == NULL){
		/*TODO : add error code 5x */
		syslog(LOG_DAEMON, "request is too long (1024 max): %s", request);
		exit(1);
	}

	/* remove \r\n at the end of string */
	pos = strchr(request, '\r');
	if (pos != NULL)
		*pos = '\0';

	/*
	 * check if the beginning of the request starts with
	 * gemini://
	 */
	if (strncmp(request, "gemini://", GEMINI_PART) != 0) {
		/* error code url malformed */
		syslog(LOG_DAEMON, "request «%s» doesn't match gemini://",
		       request);
		exit(1);
	}
	syslog(LOG_DAEMON, "request %s", request);

	/* remove the gemini:// part */
	memmove(request, request + GEMINI_PART, sizeof(request) - GEMINI_PART);

	/*
	 * look for the first / after the hostname
	 * in order to split hostname and uri
	 */
	pos = strchr(request, '/');

	if (pos != NULL) {
		/* if there is a / found */
		/* separate hostname and uri */
		estrlcpy(file, pos, strlen(pos)+1);
		/* just keep hostname in request */
		pos[0] = '\0';

		/*
		 * use a default file if no file are requested this
		 * can happen in two cases gemini://hostname/
		 * gemini://hostname/directory/
		 */
		if (strlen(file) == 0)
			estrlcpy(file, "/index.gmi", 11);
		if (file[strlen(file) - 1] == '/')
			estrlcat(file, "index.gmi", sizeof(file));
	} else {
		/*
		 * there are no slash / in the request
		 */
		estrlcpy(file, "/index.gmi", 11);
	}
	/* check if client added :port at end of request */
	pos = strchr(request, ':');
	if (pos != NULL) {
	/* end string at :*/
	pos[0] = '\0';
	}
	/* copy hostname from request */
	estrlcpy(hostname, request, sizeof(hostname));

	/*
	 * if virtualhost feature is actived looking under the default path +
	 * hostname directory gemini://foobar/hello will look for
	 * path/foobar/hello
	 */
	if (virtualhost) {
		estrlcat(path, hostname, sizeof(path));
		estrlcat(path, "/", sizeof(path));
	}
	/* add the base dir to the file requested */
	estrlcat(path, file, sizeof(path));

	/* open file and send it to stdout */
	display_file(path, lang);

	return (0);
}
