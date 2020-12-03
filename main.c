/*
 * Copyright (c) 2020 Solène Rapenne <solene@openbsd.org>
 *
 * BSD 2-clause License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include "mimes.c"

#define BUFF_LEN_1	 1000
#define BUFF_LEN_2	 1025
#define BUFF_LEN_3	 1024
#define GEMINI_PART	 9
#define DEFAULT_LANG	 "en"
#define DEFAULT_CHROOT	 "/var/gemini/"


void 		display_file(const char *, const char *);
void 		status   (const int, const char *, const char *);
void 		get_file_mime(const char *, char *, const ssize_t);
int 		main      (int, char **);


void
status(const int code, const char *file_mime, const char *lang)
{
	printf("%i %s; lang=%s\r\n",
	       code, file_mime, lang);
}

void
get_file_mime(const char *path, char *type, const ssize_t type_size)
{
	struct mimes   *iterator = database;
	char           *extension;

	extension = strrchr(path, '.');

	/* look for the MIME in the database */
	for (int i = 0; i < sizeof(database) / sizeof(struct mimes); i++, iterator++) {
		if (strcmp(iterator->extension, extension + 1) == 0) {
			strlcpy(type, iterator->type, type_size);
			break;
		}
	}

	/* if no MIME have been found, set a default one */
	if (strlen(type) == 0)
		strlcpy(type, "text/gemini", type_size);
}

void
display_file(const char *path, const char *lang)
{
	size_t 		buflen = BUFF_LEN_1;
	char           *buffer[BUFF_LEN_1];
	char 		extension[10];
	char 		file_mime[50];
	ssize_t 	nread;
	struct stat 	sb;
	FILE           *fd;

	/* this is to check if path is a directory */
	stat(path, &sb);

	/* open the file requested */
	fd = fopen(path, "r");

	if (fd != NULL && S_ISDIR(sb.st_mode) != 1) {

		get_file_mime(path, file_mime, sizeof(file_mime));

		/* check if directory */
		status(20, file_mime, lang);

		/* read the file and write it to stdout */
		while ((nread = fread(buffer, sizeof(char), buflen, fd)) != 0)
			fwrite(buffer, sizeof(char), nread, stdout);
		fclose(fd);
	} else {
		status(40, "text/gemini", lang);
	}

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
	int 		virtualhost = 0;
	int 		option;
	int 		start_with_gemini;
	char           *pos;

	while ((option = getopt(argc, argv, ":d:l:v")) != -1) {
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
		}
	}


#ifdef __OpenBSD__
	/*
	 * prevent access to files other than the one in path
	 */
	if (unveil(path, "r") == -1)
		err(1, "unveil");

	/*
	 * prevent system calls other than requirement for fread file and
	 * write to stdio
	 */
	if (pledge("stdio rpath", NULL) == -1)
		err(1, "pledge");
#endif

	/*
	 * read 1024 chars from stdin
	 * to get the request
	 */
	fgets(request, BUFF_LEN_3, stdin);

	/* remove \r\n at the end of string */
	pos = strchr(request, '\r');
	if (pos != NULL)
		strlcpy(pos, "\0", 1);

	/*
	 * check if the beginning of the request starts with
	 * gemini://
	 */
	start_with_gemini = strncmp(request, "gemini://", 9);

	/* the request must start with gemini:// */
	if (start_with_gemini != 0) {
		/* error code url malformed */
		printf("request «%s» doesn't match gemini:// at index %i",
		       request, start_with_gemini);
		exit(1);
	}
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
			puts("error undefined");
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
