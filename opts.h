#include <limits.h> /* PATH_MAX */

#define DEFAULT_MIME	 "application/octet-stream"
#define DEFAULT_LANG	 ""
#define DEFAULT_CHROOT	 "/var/gemini"
#define DEFAULT_AUTOIDX  0

 /* longest is 56 so 64 should be enough */
static char            default_mime[64]     = DEFAULT_MIME;
static char            chroot_dir[PATH_MAX] = DEFAULT_CHROOT;
static char            lang[16]             = DEFAULT_LANG;
static unsigned int    doautoidx            = DEFAULT_AUTOIDX;
static char            cgibin[PATH_MAX]     = {'\0'};
