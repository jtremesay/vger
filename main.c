#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <ctype.h>
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

/* lenght of "gemini://" */
#define GEMINI_PART	 9

/* 
 * number of bytes to read with fgets() : 2014 + 1
 * fgets() reads at most size-1 (1024 here)
 * see https://gemini.circumlunar.space/docs/specification.html.
 */
#define GEMINI_REQUEST_MAX 1025

void        autoindex(const char *);
void        cgi(const char *cgicmd);
void 		display_file(const char *);
void 		drop_privileges(const char *, const char *);
void        echdir(const char *);
void 		status(const int, const char *);
void		status_redirect(const int, const char *);
void		status_error(const int, const char*);
int         uridecode(char *);


void
echdir(const char *path)
{
	if (chdir(path) == -1) {
		switch (errno) {
		case ENOTDIR: /* FALLTHROUGH */
		case ENOENT:
			status_error(51, "file not found");
			break;
		case EACCES:
			status_error(50, "Forbidden path");
			break;
		default:
			status_error(50, "Internal server error");
			break;
		}
		errlog("failed to chdir(%s)", path);
	}
}

int
uridecode(char *uri)
{
    int n = 0;
    char c = '\0';
	long l = 0;
    char *pos = NULL;

    if ((pos = strchr(uri, '%')) == NULL) {
        return n;
    }
    while ((pos = strchr(pos, '%')) != NULL) {
        if (strlen(pos) < 3) {
            return n;
        }

        char hex[3] = {'\0'};
        for (size_t i=0; i < 2; i++) {
            hex[i] = tolower(pos[i+1]);
        }
        errno = 0;
        l = strtol(hex, 0, 16);
        if (errno == ERANGE && (l == LONG_MAX || l == LONG_MIN)) {
            /* conversion failed */
            continue;
        }
        c = (char)l;
        pos[0] = c;
        /* rewind of two char to remove %hex */
        memmove(pos+1 , pos + 3, strlen(pos+3) + 1); /* +1 for \0*/
        n++;
        pos++; /* avoid infinite loop */
    }
	return n;
}

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
		echdir("/");
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
	/* permission to execute what's inside cgidir */
	if (strlen(cgidir) > 0) {
		eunveil(cgidir, "rx");
	}
	eunveil(NULL,NULL); /* no more call to unveil() */

	/* promise permissions */
	if (strlen(cgidir) > 0) {
		epledge("stdio rpath exec", NULL);
	} else {
		epledge("stdio rpath", NULL);
	}
#endif
}

void
status(const int code, const char *file_mime)
{
	if (strcmp(file_mime, "text/gemini") == 0) {
		printf("%i %s; %s\r\n", code, file_mime, lang);
	} else {
		printf("%i %s\r\n", code, file_mime);
	}
}

void
status_redirect(const int code, const char *url)
{
	printf("%i %s\r\n",
	       code, url);
}

void
status_error(const int code, const char *reason)
{
	printf("%i %s\r\n",
		code, reason);
}

void
display_file(const char *fname)
{
	FILE		*fd = NULL;
	struct stat	 sb = {0};
	ssize_t		 nread = 0;
	const char	*file_mime;
	char		*buffer[BUFSIZ];
	char		target[FILENAME_MAX] = {'\0'};
	char        tmp[PATH_MAX] = {'\0'}; /* used to build temporary path */

	/* special case : fname empty. The user requested just the directory name */
	if (strlen(fname) == 0) {
		if (stat("index.gmi", &sb) == 0) {
			/* there is index.gmi in the current directory */
			display_file("index.gmi");
			return;
		} else if (doautoidx) {
			/* no index.gmi, so display autoindex if enabled */
			autoindex(".");
			return;
		} else {
			goto err;
		}
	}

	/* this is to check if path exists and obtain metadata later */
	if (stat(fname, &sb) == -1) {
		/* check if fname is a symbolic link
		 * if so, redirect using its target */
		if (lstat(fname, &sb) != -1 && S_ISLNK(sb.st_mode) == 1)
		goto redirect;
		else
		goto err;
	}

	/* check if directory */
	if (S_ISDIR(sb.st_mode) != 0) {
		/* no ending "/", redirect to "fname/" */
		estrlcpy(tmp, fname, sizeof(tmp));
		estrlcat(tmp, "/", sizeof(tmp));
		status_redirect(31, tmp);
		return;
	}

	/* open the file requested */
	if ((fd = fopen(fname, "r")) == NULL) { goto err; }

	file_mime = get_file_mime(fname, default_mime);

	status(20, file_mime);

	/* read the file byte after byte in buffer and write it to stdout */
	while ((nread = fread(buffer, 1, sizeof(buffer), fd)) != 0)
		fwrite(buffer, 1, nread, stdout);
	goto closefd; /* close file descriptor */
	syslog(LOG_DAEMON, "path served %s", fname);

	return;

err:
	/* return an error code and no content */
	status_error(51, "file not found");
	syslog(LOG_DAEMON, "path invalid %s", fname);
	goto closefd;

redirect:
	/* read symbolic link target to redirect */
	if (readlink(fname, target, FILENAME_MAX) == -1) {
		goto err;
	}

	status_redirect(30, target);
	syslog(LOG_DAEMON, "redirection from %s to %s", fname, target);

closefd:
	if (S_ISREG(sb.st_mode) != 0) {
		fclose(fd);
	}
}

void
autoindex(const char *path)
{
	/* display liks to files in path + a link to parent (..) */

	int n = 0;
	struct dirent **namelist; /* this must be freed at last */

	syslog(LOG_DAEMON, "autoindex: %s", path);

	/* use alphasort to always have the same order on every system */
	if ((n = scandir(path, &namelist, NULL, alphasort)) < 0) {
		status_error(50, "Internal server error");
		errlog("Can't scan %s", path);
	} else {
		status(20, "text/gemini");
		printf("=> .. ../\n"); /* display link to parent */
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
	/* run cgicmd replacing current process */
	execl(cgicmd, cgicmd, NULL);
	/* if execl is ok, this will never be reached */
	status(42, "Couldn't execute CGI script");
	errlog("error when trying to execl %s", cgicmd);
	exit(1);
}

int
main(int argc, char **argv)
{
	char 		request   [GEMINI_REQUEST_MAX] = {'\0'};
	char 		user      [_SC_LOGIN_NAME_MAX] = "";
	char 		hostname  [GEMINI_REQUEST_MAX] = {'\0'};
	char        query     [PATH_MAX]           = {'\0'};
	char        chroot_dir[PATH_MAX]           = DEFAULT_CHROOT;
	char        file      [FILENAME_MAX]       = DEFAULT_INDEX;
	char        dir       [PATH_MAX]           = {'\0'};
	char        *pos                           = NULL;
	int 		option                         = 0;
	int         virtualhost                    = 0;
	int         docgi                          = 0;

	/*
     * request : contain the whole request from client : gemini://...\r\n
	 * user : username, used in drop_privileges()
	 * hostname : extracted from hostname. used with virtualhosts and cgi SERVER_NAME
	 * query : file requested in cgi : gemini://...?query
	 * file : file basename to display. Emtpy is a directory has been requested
	 * dir : directory requested. vger will chdir() in to find file
	 * pos : used to parse request and split into interesting parts
	 */

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
			estrlcpy(cgidir, optarg, sizeof(cgidir));
			docgi = 1;
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
	 * do chroot if an user is supplied
	 */
	drop_privileges(user, chroot_dir);

	/*
	 * read 1024 chars from stdin
	 * to get the request
	 * (actually 1024 + \0)
	 */
	if (fgets(request, GEMINI_REQUEST_MAX, stdin) == NULL) {
		/* EOF reached before reading anything */
		if (feof(stdin)) {
			status(59, "request is too short and probably empty");
			errlog("request is too short and probably empty");

		/* error before reading anything */
		} else if (ferror(stdin)) {
			status(59, "Error while reading request");
			errlog("Error while reading request: %s", request);
		}
	}

	/* check if string ends with '\n', or to long */
	if (request[strnlen(request, GEMINI_REQUEST_MAX) - 1] != '\n') {
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
	memmove(request, request + GEMINI_PART, strlen(request) +1 - GEMINI_PART);

	/* remove all "/.." for safety reasons */
    while ((pos = strstr(request, "/..")) != NULL ) {
		memmove(request, pos+3, strlen(pos) +1 - 3); /* "/.." = 3 */
	}

	echdir(chroot_dir); /* move to chroot */

	/* look for hostname in request : first thing before first / if any */
	pos = strchr(request, '/');
	if (pos != NULL) {
		/* copy what's after hostname in dir */
		estrlcpy(dir, pos, strlen(pos)+1);
		/* just keep hostname in request : stop the string with \0 */
		pos[0] = '\0';
	}

	/* check if client added :port at end of hostname and remove it */
	pos = strchr(request, ':');
	if (pos != NULL) {
		/* end string at :*/
		pos[0] = '\0';
	}

	/* copy hostname from request */
	estrlcpy(hostname, request, sizeof(hostname));

	/* remove leading '/' in dir */
	while (dir[0] == '/') {
		memmove(dir, dir+1, strlen(dir+1)+1);
	}

	if (virtualhost) {
		/* add hostname at the beginning of the dir path */
		char tmp[PATH_MAX] = {'\0'};
		estrlcpy(tmp, hostname, sizeof(tmp));
		estrlcat(tmp, "/", sizeof(tmp));
		estrlcat(tmp, dir, sizeof(tmp));
		estrlcpy(dir, tmp, sizeof(dir));
	}

	/* percent decode */
	uridecode(dir);

	/* 
	 * split dir and filename.
	 * file is last part after last '/'.
	 * if none found, then requested file is actually a directory
	 */
	if (strlen(dir) > 0) {
		pos = strrchr(dir, '/');
		if (pos != NULL) {
			estrlcpy(file, pos+1, sizeof(file)); /* +1 : no leading '/' */
			pos[0] = '\0';
			if (strlen(dir) > 0) {
				echdir(dir); /* change directory to requested directory */
			}
		} else {
			estrlcpy(file, dir, sizeof(file));
		}
	}

	if (docgi) {
		/* check if directory is cgidir */
		char cgifp[PATH_MAX] = {'\0'};
		estrlcpy(cgifp, chroot_dir, sizeof(cgifp));
		if (cgifp[strlen(cgifp)-1] != '/') {
			estrlcat(cgifp, "/", sizeof(cgifp));
		}
		estrlcat(cgifp, dir, sizeof(cgifp));
		if (strcmp(cgifp, cgidir) != 0) {
			/* not cgipath, display file content */
			goto file_to_stdout;
		}
		/* set env variables for CGI */
		/* see https://lists.orbitalfox.eu/archives/gemini/2020/000315.html */
		esetenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
		esetenv("SERVER_PROTOCOL", "GEMINI", 1);
		esetenv("SERVER_SOFTWARE", "vger/1", 1);

		/* look for "?" if any to set query for cgi, remove it*/
		pos = strchr(file, '?');
		if (pos != NULL) {
			estrlcpy(query, pos+1, sizeof(query));
			esetenv("QUERY_STRING", query, 1);
			pos[0] = '\0';
		}

		/* look for an extension to find PATH_INFO */
		pos = strrchr(file, '.');
		if (pos != NULL) {
			/* found a dot */
			pos = strchr(pos, '/');
			if (pos != NULL) {
				setenv("PATH_INFO", pos, 1);
				pos[0] = '\0'; /* keep only script name */
			}
		}
		esetenv("SCRIPT_NAME", file, 1);
		esetenv("SERVER_NAME", hostname, 1);

		cgi(file);
		return 0;
	}

file_to_stdout:
	/* regular file  to stdout */
	display_file(file);

	return (0);
}
