#include <string.h>
#include <unistd.h>
#include <string.h>

#include "mimes.h"

struct mimes {
	char 		extension[10];
	char 		type     [70];
};


struct mimes 	database[] = {
	{"7z", "application/x-7z-compressed"},
	{"atom", "application/atom+xml"},
	{"avi", "video/x-msvideo"},
	{"bin", "application/octet-stream"},
	{"bmp", "image/x-ms-bmp"},
	{"cco", "application/x-cocoa"},
	{"crt", "application/x-x509-ca-cert"},
	{"css", "text/css"},
	{"deb", "application/octet-stream"},
	{"dll", "application/octet-stream"},
	{"dmg", "application/octet-stream"},
	{"doc", "application/msword"},
	{"eot", "application/vnd.ms-fontobject"},
	{"exe", "application/octet-stream"},
	{"flv", "video/x-flv"},
	{"fs", "application/octet-stream"},
	{"gemini", "text/gemini"},
	{"gif", "image/gif"},
	{"gmi", "text/gemini"},
	{"hqx", "application/mac-binhex40"},
	{"htc", "text/x-component"},
	{"html", "text/html"},
	{"ico", "image/x-icon"},
	{"img", "application/octet-stream"},
	{"iso", "application/octet-stream"},
	{"jad", "text/vnd.sun.j2me.app-descriptor"},
	{"jar", "application/java-archive"},
	{"jardiff", "application/x-java-archive-diff"},
	{"jng", "image/x-jng"},
	{"jnlp", "application/x-java-jnlp-file"},
	{"jpeg", "image/jpeg"},
	{"jpg", "image/jpeg"},
	{"js", "application/javascript"},
	{"json", "application/json"},
	{"kml", "application/vnd.google-earth.kml+xml"},
	{"kmz", "application/vnd.google-earth.kmz"},
	{"m3u8", "application/vnd.apple.mpegurl"},
	{"m4a", "audio/x-m4a"},
	{"m4v", "video/x-m4v"},
	{"md", "text/markdown"},
	{"mid", "audio/midi"},
	{"midi", "audio/midi"},
	{"mkv", "video/x-matroska"},
	{"mml", "text/mathml"},
	{"mng", "video/x-mng"},
	{"mov", "video/quicktime"},
	{"mp3", "audio/mpeg"},
	{"mp4", "video/mp4"},
	{"mpeg", "video/mpeg"},
	{"mpg", "video/mpeg"},
	{"msi", "application/octet-stream"},
	{"msm", "application/octet-stream"},
	{"msp", "application/octet-stream"},
	{"odb", "application/vnd.oasis.opendocument.database"},
	{"odc", "application/vnd.oasis.opendocument.chart"},
	{"odf", "application/vnd.oasis.opendocument.formula"},
	{"odg", "application/vnd.oasis.opendocument.graphics"},
	{"odi", "application/vnd.oasis.opendocument.image"},
	{"odm", "application/vnd.oasis.opendocument.text-master"},
	{"odp", "application/vnd.oasis.opendocument.presentation"},
	{"ods", "application/vnd.oasis.opendocument.spreadsheet"},
	{"odt", "application/vnd.oasis.opendocument.text"},
	{"ogg", "audio/ogg"},
	{"oth", "application/vnd.oasis.opendocument.text-web"},
	{"otp", "application/vnd.oasis.opendocument.presentation-template"},
	{"pac", "application/x-ns-proxy-autoconfig"},
	{"pdf", "application/pdf"},
	{"pem", "application/x-x509-ca-cert"},
	{"pl", "application/x-perl"},
	{"pm", "application/x-perl"},
	{"png", "image/png"},
	{"ppt", "application/vnd.ms-powerpoint"},
	{"ps", "application/postscript"},
	{"ra", "audio/x-realaudio"},
	{"rar", "application/x-rar-compressed"},
	{"rpm", "application/x-redhat-package-manager"},
	{"rss", "application/rss+xml"},
	{"rtf", "application/rtf"},
	{"run", "application/x-makeself"},
	{"sea", "application/x-sea"},
	{"sit", "application/x-stuffit"},
	{"svg", "image/svg+xml"},
	{"svgz", "image/svg+xml"},
	{"swf", "application/x-shockwave-flash"},
	{"tcl", "application/x-tcl"},
	{"tif", "image/tiff"},
	{"tiff", "image/tiff"},
	{"tk", "application/x-tcl"},
	{"ts", "video/mp2t"},
	{"txt", "text/plain"},
	{"war", "application/java-archive"},
	{"wbmp", "image/vnd.wap.wbmp"},
	{"webm", "video/webm"},
	{"webp", "image/webp"},
	{"wml", "text/vnd.wap.wml"},
	{"wmlc", "application/vnd.wap.wmlc"},
	{"wmv", "video/x-ms-wmv"},
	{"woff", "application/font-woff"},
	{"xhtml", "application/xhtml+xml"},
	{"xls", "application/vnd.ms-excel"},
	{"xml", "text/xml"},
	{"xpi", "application/x-xpinstall"},
	{"zip", "application/zip"}
};

void
get_file_mime(const char *path, char *type, const ssize_t type_size)
{
	char           *extension;

	extension = strrchr(path, '.');

	/* look for the MIME in the database */
	for (int i = 0; i < sizeof(database) / sizeof(struct mimes); i++) {
		if (strcmp(database[i].extension, extension + 1) == 0) {
			strlcpy(type, database[i].type, type_size);
			break;
		}
	}

	/* if no MIME have been found, set a default one */
	if (strlen(type) == 0)
		strlcpy(type, "text/gemini", type_size);
}
