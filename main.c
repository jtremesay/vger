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
#define GEMINI_REQUEST_MAX 1025  /* https://gemini.circumlunar.space/docs/specification.html + ending \0 */



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

		/* base directory is now / */
                estrlcpy(chroot_dir, "/", sizeof(chroot_dir));
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
	/* permission to execute what's inside cgipath */
	if (strlen(cgibin) > 0) {
		/* first, build the full path of cgi (not in chroot) */
		char cgifullpath[PATH_MAX] = {'\0'};
		estrlcpy(cgifullpath, path, sizeof(cgifullpath));
		estrlcat(cgifullpath, cgibin, sizeof(cgifullpath));

		eunveil(cgifullpath, "rx");
	}
	/* forbid more unveil */
	eunveil(NULL, NULL);

	/*
	 * prevent system calls other parsing queryfor fread file and
	 * write to stdio
	 */
	if (strlen(cgibin) > 0) {
		/* cgi need execlp() (exec) and fork() (proc) */
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
	char        tmp[PATH_MAX] = {'\0'}; /* used to build temporary path */

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
			estrlcpy(tmp, uri,  sizeof(tmp));
			estrlcat(tmp, "/", sizeof(tmp));
			status_redirect(31, tmp);
			return;

		} else {
			/* there is a leading "/", display index.gmi */
			estrlcpy(tmp, fp, sizeof(tmp));
			estrlcat(tmp, "index.gmi", sizeof(tmp));

			/* check if index.gmi exists or show autoindex */
			if (stat(tmp, &sb) == 0) {
				estrlcpy(fp, tmp, sizeof(fp));
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

	/* read the file byte after byte in buffer and write it to stdout */
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
	int n = 0;
	char *pos = NULL;
	struct dirent **namelist; /* this must be freed at last */


	syslog(LOG_DAEMON, "autoindex: %s", path);

	status(20, "text/gemini");

	/* display link to parent */
	char parent[PATH_MAX] = {'\0'};
	/* parent is "path" without chroot_dir */
	estrlcpy(parent, path+strlen(chroot_dir), sizeof(parent));
	/* remove ending '/' */
	while (parent[strlen(parent)-1] == '/') {
		parent[strlen(parent)-1] = '\0';
	}
	/* remove last part after '/' */
	pos = strrchr(parent, '/');
	if (pos != NULL) {
		pos[1] = '\0'; /* at worse, parent is now "/" */
	}
	printf("=> %s ../\n", parent);

	/* use alphasort to always have the same order on every system */
	if ((n = scandir(path, &namelist, NULL, alphasort)) < 0) {
		status(51, "text/gemini");
		errlog("Can't scan %s", path);
	} else {
		for(int j = 0; j < n; j++) {
			/* skip self and parent */
			if ((strcmp(namelist[j]->d_name, ".") == 0) ||
			    (strcmp(namelist[j]->d_name, "..") == 0)) {
				continue;
			}
			/* add "/" at the end of a directory path */
			if (namelist[j]->d_type == DT_DIR) {
				printf("=> ./%s/ %s/\n", namelist[j]->d_name, namelist[j]->d_name);
			} else {
				printf("=> ./%s %s\n", namelist[j]->d_name, namelist[j]->d_name);
			}
			free(namelist[j]);
		}
		free(namelist);
	}
}

void
cgi(const char *cgicmd)
{

	int pipedes[2] = {0};
	pid_t pid;

	/* get a pipe to get stdout */
	if (pipe(pipedes) != 0) {
		status(42, "text/gemini");
		err(1, "pipe failed");
	}

	pid = fork();

	if (pid < 0) {
		close(pipedes[0]);
		close(pipedes[1]);
		status(42, "text/gemini");
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
			status(42, "text/gemini");
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
		status(42, "text/gemini");
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
	char        query[PATH_MAX]               = {'\0'};
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

	size_t reqlen = 0;
	/*
	 * read 1024 chars from stdin
	 * to get the request
	 */
	reqlen = fread(request, 1, GEMINI_REQUEST_MAX, stdin);

	/* now should end of file, or request is too long */
	if (feof(stdin) == 0) {
		status(59, "request too long");
		errlog("request too long");
	}

	/* remove \r\n at the end of string */
	pos = strstr(request, "\r\n");
	if (pos != NULL) {
		*pos = '\0';
	} else {
		status(59, "malformed request, must end with \r\n");
		errlog("malformed request, must end with \r\n");
	}

	/*
	 * check if the beginning of the request starts with
	 * gemini://
	 */
	if (strncmp(request, "gemini://", GEMINI_PART) != 0) {
		/* error code url malformed */
		status(59, "request malformed : must start with gemini://");
		errlog("request «%s» doesn't match gemini://",
		       request);
	}
	syslog(LOG_DAEMON, "request %s", request);

	/* remove the gemini:// part */
	memmove(request, request + GEMINI_PART, strlen(request) +1 - GEMINI_PART);

	/* remove all "/.." for safety reasons */
    while ((pos = strstr(request, "/..")) != NULL ) {
		memmove(request, pos+3, strlen(pos) +1 - 3); /* "/.." = 3 */
	}

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

	/* look for "?" if any to set query for cgi, or remove it*/
	pos = strchr(uri, '?');
	if (pos != NULL) {
		estrlcpy(query, pos+1, sizeof(query));
		esetenv("QUERY_STRING", query, 1);
		pos[0] = '\0';
	}

	/*
	 * if virtualhost feature is actived looking under the chroot_path +
	 * hostname directory gemini://foobar/hello will look for
	 * chroot_path/foobar/hello
	 */
	if (virtualhost) {
		if (strlen(uri) == 0) {
			estrlcpy(uri, "/index.gmi", sizeof(uri));
		}
		char tmp[PATH_MAX] = {'\0'};
		estrlcpy(tmp, hostname, sizeof(tmp));
		estrlcat(tmp, uri, sizeof(tmp));
		estrlcpy(uri, tmp, sizeof(uri));
	}

	/* check if uri is cgibin */
	if ((strlen(cgibin) > 0) &&
		(strncmp(uri, cgibin, strlen(cgibin)) == 0)) {

		/* cgipath with chroot_dir at the beginning */
		char cgipath[PATH_MAX] = {'\0'};
		estrlcpy(cgipath, chroot_dir, sizeof(cgipath));
		estrlcat(cgipath, uri, sizeof(cgipath));
		/* set env variables for CGI */
		/* see https://lists.orbitalfox.eu/archives/gemini/2020/000315.html */
		esetenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
		esetenv("SERVER_PROTOCOL", "GEMINI", 1);
		esetenv("SERVER_SOFTWARE", "vger/1", 1);

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
