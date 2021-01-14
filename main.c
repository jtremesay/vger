#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "mimes.h"
#include "opts.h"
#include "utils.h"

#define GEMINI_PART	 9
#define GEMINI_REQUEST_MAX 1024 /* see https://gemini.circumlunar.space/docs/specification.html */



void        autoindex(const char *);
void        cgi(const char *cgicmd);
void 		display_file(const char *);
void 		status(const int, const char *);
void		status_redirect(const int, const char *);
void 		drop_privileges(const char *, const char *);

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
			errlog("chroot requires program to be run as root");
		}
		/* search user uid from name */
		if ((pw = getpwnam(user)) == NULL) {
			errlog("the user %s can't be found on the system", user);
		}
		/* chroot worked? */
		if (chroot(path) != 0) {
			errlog("the chroot_dir %s can't be used for chroot", path);
		}
		chrooted = 1;
		if (chdir("/") == -1) {
			errlog("failed to chdir(\"/\")");
		}
		/* drop privileges */
		if (setgroups(1, &pw->pw_gid) ||
		    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
		    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid)) {
			errlog("dropping privileges to user %s (uid=%i) failed",
			       user, pw->pw_uid);
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
	if (strlen(cgibin) > 0) {
		char cgifullpath[PATH_MAX] = {'\0'};
		estrlcpy(cgifullpath, path, sizeof(cgifullpath));
		estrlcat(cgifullpath, cgibin, sizeof(cgifullpath));
		eunveil(cgifullpath, "rx");
	}
	/*
	 * prevent system calls other parsing queryfor fread file and
	 * write to stdio
	 */
	if (strlen(cgibin) > 0) {
		epledge("stdio rpath exec proc", NULL);
	} else {
		epledge("stdio rpath", NULL);
	}
#endif
}

void
status(const int code, const char *file_mime)
{
	printf("%i %s; %s\r\n",
	       code, file_mime, lang);
}

void
status_redirect(const int code, const char *url)
{
	printf("%i %s\r\n",
	       code, url);
}

void
display_file(const char *uri)
{
	FILE		*fd = NULL;
	struct stat	 sb = {0};
	ssize_t		 nread = 0;
	const char	*file_mime;
	char		*buffer[BUFSIZ];
	char		target[FILENAME_MAX] = {'\0'};
	char		fp[PATH_MAX] = {'\0'};

	/* build file path inside chroot */
	estrlcpy(fp, chroot_dir, sizeof(fp));
	estrlcat(fp, uri, sizeof(fp));

	/* this is to check if path exists and obtain metadata later */
	if (stat(fp, &sb) == -1) {

		/* check if fp is a symbolic link
		 * if so, redirect using its target */
		if (lstat(fp, &sb) != -1 && S_ISLNK(sb.st_mode) == 1)
		goto redirect;
		else
		goto err;
	}

	/* check if directory */
	if (S_ISDIR(sb.st_mode) != 0) {
		if (fp[strlen(fp) -1 ] != '/') {
			/* no ending "/", redirect to "path/" */
			char new_uri[PATH_MAX] = {'\0'};
			estrlcpy(new_uri, uri,  sizeof(new_uri));
			estrlcat(new_uri, "/", sizeof(new_uri));
			status_redirect(31, new_uri);
			return;

		} else {
			/* there is a leading "/", display index.gmi */
			char index_path[PATH_MAX] = {'\0'};
			estrlcpy(index_path, fp, sizeof(index_path));
			estrlcat(index_path, "index.gmi", sizeof(index_path));

			/* check if index.gmi exists or show autoindex */
			if (stat(index_path, &sb) == 0) {
				estrlcpy(fp, index_path, sizeof(fp));
			} else if (doautoidx != 0) {
				autoindex(fp);
				return;
			} else {
				goto err;
			}
		}
	}

	/* open the file requested */
	if ((fd = fopen(fp, "r")) == NULL) { goto err; }

	file_mime = get_file_mime(fp, default_mime);

	status(20, file_mime);

	/* read the file and write it to stdout */
	while ((nread = fread(buffer, 1, sizeof(buffer), fd)) != 0)
		fwrite(buffer, 1, nread, stdout);
	goto closefd;
	syslog(LOG_DAEMON, "path served %s", fp);

	return;

err:
	/* return an error code and no content */
	status(51, "text/gemini");
	syslog(LOG_DAEMON, "path invalid %s", fp);
	goto closefd;

redirect:
	/* read symbolic link target to redirect */
	if (readlink(fp, target, FILENAME_MAX) == -1) {
		goto err;
	}

	status_redirect(30, target);
	syslog(LOG_DAEMON, "redirection from %s to %s", fp, target);

closefd:
	if (S_ISREG(sb.st_mode) != 0) {
		fclose(fd);
	}
}

void
autoindex(const char *path)
{
	struct dirent *dp;
	DIR *fd;


	if (!(fd = opendir(path))) {
		err(1,"opendir '%s':", path);
	}

	syslog(LOG_DAEMON, "autoindex: %s", path);

	status(20, "text/gemini");

	/* TODO : add ending / in name if directory */
	while ((dp = readdir(fd))) {
		/* skip self */
		if (!strcmp(dp->d_name, ".")) {
			continue;
		}
		if (dp->d_type == DT_DIR) {
			printf("=> ./%s/ %s/\n", dp->d_name, dp->d_name);
		} else {
			printf("=> ./%s %s\n", dp->d_name, dp->d_name);
		}
	}
	closedir(fd);
}

void
cgi(const char *cgicmd)
{

	int pipedes[2] = {0};
	pid_t pid;

	if (pipe(pipedes) != 0) {
		err(1, "pipe failed");
	}

	pid = fork();

	if (pid < 0) {
		close(pipedes[0]);
		close(pipedes[1]);
		err(1, "fork failed");
	}

	if (pid > 0) { /* parent */
		char buf[3];
		size_t nread = 0;
		FILE *output = NULL;

		close(pipedes[1]); /* make sure entry is closed so fread() gets EOF */

		/* use fread/fwrite because are buffered */
		output = fdopen(pipedes[0], "r");
		if (output == NULL) {
			err(1, "fdopen failed");
		}

		/* read pipe output */
		while ((nread = fread(buf, 1, sizeof(buf), output)) != 0) {
			fwrite(buf, 1, nread, stdout);
		}
		close(pipedes[0]);
		fclose(output);

		wait(NULL); /* wait for child to terminate */

		exit(0);

	} else if (pid == 0) { /* child */
		dup2(pipedes[1], STDOUT_FILENO); /* set pipe output equal to stdout */
		close(pipedes[1]); /* no need this file descriptor : it is now stdout */
		execlp(cgicmd, cgicmd, NULL);
		/* if execlp is ok, this will never be reached */
		status(42, "text/plain");
		errlog("error when trying to execlp %s", cgicmd);
	}
}

int
main(int argc, char **argv)
{
	char 		request  [GEMINI_REQUEST_MAX] = {'\0'};
	char 		hostname [GEMINI_REQUEST_MAX] = {'\0'};
	char 		uri      [PATH_MAX]           = {'\0'};
	char 		user     [_SC_LOGIN_NAME_MAX] = "";
	int 		virtualhost = 0;
	int 		option = 0;
	char        *pos = NULL;

	while ((option = getopt(argc, argv, ":d:l:m:u:c:vi")) != -1) {
		switch (option) {
		case 'd':
			estrlcpy(chroot_dir, optarg, sizeof(chroot_dir));
			break;
		case 'l':
			estrlcpy(lang, "lang=", sizeof(lang));
			estrlcat(lang, optarg, sizeof(lang));
			break;
		case 'm':
			estrlcpy(default_mime, optarg, sizeof(default_mime));
			break;
		case 'u':
			estrlcpy(user, optarg, sizeof(user));
			break;
		case 'c':
			estrlcpy(cgibin, optarg, sizeof(cgibin));
			break;
		case 'v':
			virtualhost = 1;
			break;
		case 'i':
			doautoidx = 1;
			break;
		}
	}

	/*
	 * do chroot if an user is supplied run pledge/unveil if OpenBSD
	 */
	drop_privileges(user, chroot_dir);

	/*
	 * read 1024 chars from stdin
	 * to get the request
	 */
	if (fgets(request, GEMINI_REQUEST_MAX, stdin) == NULL) {
		status(59, "request is too long (1024 max)");
		errlog("request is too long (1024 max): %s", request);
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
		errlog("request «%s» doesn't match gemini://",
		       request);
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
		estrlcpy(uri, pos, strlen(pos)+1);
		/* just keep hostname in request */
		pos[0] = '\0';
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
	 * if virtualhost feature is actived looking under the chroot_path +
	 * hostname directory gemini://foobar/hello will look for
	 * chroot_path/foobar/hello
	 */
	if (virtualhost) {
		if (strlen(uri) == 0) {
			estrlcpy(uri, "/index.gmi", sizeof(uri));
		}
		char new_uri[PATH_MAX] = {'\0'};
		estrlcpy(new_uri, hostname, sizeof(new_uri));
		estrlcat(new_uri, uri, sizeof(new_uri));
		estrlcpy(uri, new_uri, sizeof(uri));
	}

	/* check if uri is cgibin */
	if ((strlen(cgibin) > 0) && 
		(strncmp(uri, cgibin, strlen(cgibin)) == 0)
		) {
		char cgipath[PATH_MAX] = {'\0'};
		estrlcpy(cgipath, chroot_dir, sizeof(cgipath));
		estrlcat(cgipath, uri, sizeof(cgipath));
		/* set env variables for CGI */
		/* see https://lists.orbitalfox.eu/archives/gemini/2020/000315.html */
		esetenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
		esetenv("SERVER_PROTOCOL", "GEMINI", 1);
		esetenv("SERVER_SOFTWARE", "vger/1", 1);

		/* look for "?" to set query */
		pos = strchr(cgipath, '?');
		if (pos != NULL) {
			char query[PATH_MAX] = {'\0'};
			estrlcpy(query, pos+1, sizeof(query));
			esetenv("QUERY_STRING", query, 1);
			pos[0] = '\0';
		}
		/* look for an extension to find PATH_INFO */
		pos = strrchr(cgipath, '.');
		if (pos != NULL) {
			/* found a dot */
			pos = strchr(pos, '/');
			if (pos != NULL) {
				setenv("PATH_INFO", pos, 1);
				pos[0] = '\0'; /* keep only script name */
			}
		}
		esetenv("SCRIPT_NAME", cgipath, 1);
		esetenv("SERVER_NAME", hostname, 1);

		cgi(cgipath);

	} else {
		//TODO: percent decoding here 
		/* open file and send it to stdout */
		display_file(uri);
	}

	return (0);
}
