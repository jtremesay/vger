#include <limits.h> /* PATH_MAX */

/* Defaults values */
#define DEFAULT_MIME	 "application/octet-stream"
#define DEFAULT_LANG	 ""
#define DEFAULT_CHROOT	 "/var/gemini"
#define DEFAULT_INDEX    "index.gmi"
#define DEFAULT_AUTOIDX  0

/* 
 * Options used later
 */
/* longest hardcoded mimetype is 56 long so 64 should be enough */
static char            default_mime[64]     = DEFAULT_MIME;
static char            lang[16]             = DEFAULT_LANG;
static unsigned int    doautoidx            = DEFAULT_AUTOIDX;
static char            cgidir[PATH_MAX]     = {'\0'};
static int             chrooted             = 0;
