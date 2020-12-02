#include <string.h>

struct mimes {
	char 		extension[10];
	char 		type     [50];
};

void 		make_mime_database(struct mimes *);

void
make_mime_database(struct mimes * database)
{
	strlcpy(database[0].type, "application/atom+xml", sizeof(database[0].type));
	strlcpy(database[0].extension, "atom", sizeof(database[0].extension));

	strlcpy(database[1].type, "application/font-woff", sizeof(database[1].type));
	strlcpy(database[1].extension, "woff", sizeof(database[1].extension));

	strlcpy(database[2].type, "application/java-archive", sizeof(database[2].type));
	strlcpy(database[2].extension, "jar", sizeof(database[2].extension));

	strlcpy(database[3].type, "application/java-archive", sizeof(database[3].type));
	strlcpy(database[3].extension, "war", sizeof(database[3].extension));

	strlcpy(database[4].type, "application/javascript", sizeof(database[4].type));
	strlcpy(database[4].extension, "js", sizeof(database[4].extension));

	strlcpy(database[5].type, "application/json", sizeof(database[5].type));
	strlcpy(database[5].extension, "json", sizeof(database[5].extension));

	strlcpy(database[6].type, "application/mac-binhex40", sizeof(database[6].type));
	strlcpy(database[6].extension, "hqx", sizeof(database[6].extension));

	strlcpy(database[7].type, "application/octet-stream", sizeof(database[7].type));
	strlcpy(database[7].extension, "deb", sizeof(database[7].extension));

	strlcpy(database[8].type, "application/octet-stream", sizeof(database[8].type));
	strlcpy(database[8].extension, "dmg", sizeof(database[8].extension));

	strlcpy(database[9].type, "application/pdf", sizeof(database[9].type));
	strlcpy(database[9].extension, "pdf", sizeof(database[9].extension));

	strlcpy(database[10].type, "application/rss+xml", sizeof(database[10].type));
	strlcpy(database[10].extension, "rss", sizeof(database[10].extension));

	strlcpy(database[11].type, "application/rtf", sizeof(database[11].type));
	strlcpy(database[11].extension, "rtf", sizeof(database[11].extension));

	strlcpy(database[12].type, "application/vnd.apple.mpegurl", sizeof(database[12].type));
	strlcpy(database[12].extension, "m3u8", sizeof(database[12].extension));

	strlcpy(database[13].type, "application/vnd.google-earth.kml+xml", sizeof(database[13].type));
	strlcpy(database[13].extension, "kml", sizeof(database[13].extension));

	strlcpy(database[14].type, "application/vnd.google-earth.kmz", sizeof(database[14].type));
	strlcpy(database[14].extension, "kmz", sizeof(database[14].extension));

	strlcpy(database[15].type, "application/vnd.ms-excel", sizeof(database[15].type));
	strlcpy(database[15].extension, "xls", sizeof(database[15].extension));

	strlcpy(database[16].type, "application/vnd.ms-fontobject", sizeof(database[16].type));
	strlcpy(database[16].extension, "eot", sizeof(database[16].extension));

	strlcpy(database[17].type, "application/vnd.ms-powerpoint", sizeof(database[17].type));
	strlcpy(database[17].extension, "ppt", sizeof(database[17].extension));

	strlcpy(database[18].type, "application/vnd.oasis.opendocument.chart", sizeof(database[18].type));
	strlcpy(database[18].extension, "odc", sizeof(database[18].extension));

	strlcpy(database[19].type, "application/vnd.oasis.opendocument.database", sizeof(database[19].type));
	strlcpy(database[19].extension, "odb", sizeof(database[19].extension));

	strlcpy(database[20].type, "application/vnd.oasis.opendocument.formula", sizeof(database[20].type));
	strlcpy(database[20].extension, "odf", sizeof(database[20].extension));

	strlcpy(database[21].type, "application/vnd.oasis.opendocument.graphics", sizeof(database[21].type));
	strlcpy(database[21].extension, "odg", sizeof(database[21].extension));

	strlcpy(database[22].type, "application/vnd.oasis.opendocument.image", sizeof(database[22].type));
	strlcpy(database[22].extension, "odi", sizeof(database[22].extension));

	strlcpy(database[23].type, "application/vnd.oasis.opendocument.presentation", sizeof(database[23].type));
	strlcpy(database[23].extension, "odp", sizeof(database[23].extension));

	strlcpy(database[24].type, "application/vnd.oasis.opendocument.presentation-template", sizeof(database[24].type));
	strlcpy(database[24].extension, "otp", sizeof(database[24].extension));

	strlcpy(database[25].type, "application/vnd.oasis.opendocument.spreadsheet", sizeof(database[25].type));
	strlcpy(database[25].extension, "ods", sizeof(database[25].extension));

	strlcpy(database[26].type, "application/vnd.oasis.opendocument.text", sizeof(database[26].type));
	strlcpy(database[26].extension, "odt", sizeof(database[26].extension));

	strlcpy(database[27].type, "application/vnd.oasis.opendocument.text-master", sizeof(database[27].type));
	strlcpy(database[27].extension, "odm", sizeof(database[27].extension));

	strlcpy(database[28].type, "application/vnd.oasis.opendocument.text-web", sizeof(database[28].type));
	strlcpy(database[28].extension, "oth", sizeof(database[28].extension));

	strlcpy(database[29].type, "application/vnd.wap.wmlc", sizeof(database[29].type));
	strlcpy(database[29].extension, "wmlc", sizeof(database[29].extension));

	strlcpy(database[30].type, "application/x-7z-compressed", sizeof(database[30].type));
	strlcpy(database[30].extension, "7z", sizeof(database[30].extension));

	strlcpy(database[31].type, "application/x-cocoa", sizeof(database[31].type));
	strlcpy(database[31].extension, "cco", sizeof(database[31].extension));

	strlcpy(database[32].type, "application/x-java-archive-diff", sizeof(database[32].type));
	strlcpy(database[32].extension, "jardiff", sizeof(database[32].extension));

	strlcpy(database[34].type, "application/x-java-jnlp-file", sizeof(database[34].type));
	strlcpy(database[34].extension, "jnlp", sizeof(database[34].extension));

	strlcpy(database[35].type, "application/x-makeself", sizeof(database[35].type));
	strlcpy(database[35].extension, "run", sizeof(database[35].extension));

	strlcpy(database[36].type, "application/x-ns-proxy-autoconfig", sizeof(database[36].type));
	strlcpy(database[36].extension, "pac", sizeof(database[36].extension));

	strlcpy(database[37].type, "application/x-rar-compressed", sizeof(database[37].type));
	strlcpy(database[37].extension, "rar", sizeof(database[37].extension));

	strlcpy(database[38].type, "application/x-redhat-package-manager", sizeof(database[38].type));
	strlcpy(database[38].extension, "rpm", sizeof(database[38].extension));

	strlcpy(database[39].type, "application/x-sea", sizeof(database[39].type));
	strlcpy(database[39].extension, "sea", sizeof(database[39].extension));

	strlcpy(database[40].type, "application/x-shockwave-flash", sizeof(database[40].type));
	strlcpy(database[40].extension, "swf", sizeof(database[40].extension));

	strlcpy(database[41].type, "application/x-stuffit", sizeof(database[41].type));
	strlcpy(database[41].extension, "sit", sizeof(database[41].extension));

	strlcpy(database[42].type, "application/x-xpinstall", sizeof(database[42].type));
	strlcpy(database[42].extension, "xpi", sizeof(database[42].extension));

	strlcpy(database[43].type, "application/xhtml+xml", sizeof(database[43].type));
	strlcpy(database[43].extension, "xhtml", sizeof(database[43].extension));

	strlcpy(database[44].type, "application/zip", sizeof(database[44].type));
	strlcpy(database[44].extension, "zip", sizeof(database[44].extension));

	strlcpy(database[45].type, "audio/mpeg", sizeof(database[45].type));
	strlcpy(database[45].extension, "mp3", sizeof(database[45].extension));

	strlcpy(database[46].type, "audio/ogg", sizeof(database[46].type));
	strlcpy(database[46].extension, "ogg", sizeof(database[46].extension));

	strlcpy(database[47].type, "audio/x-m4a", sizeof(database[47].type));
	strlcpy(database[47].extension, "m4a", sizeof(database[47].extension));

	strlcpy(database[48].type, "audio/x-realaudio", sizeof(database[48].type));
	strlcpy(database[48].extension, "ra", sizeof(database[48].extension));

	strlcpy(database[49].type, "image/gif", sizeof(database[49].type));
	strlcpy(database[49].extension, "gif", sizeof(database[49].extension));

	strlcpy(database[50].type, "image/png", sizeof(database[50].type));
	strlcpy(database[50].extension, "png", sizeof(database[50].extension));

	strlcpy(database[51].type, "image/vnd.wap.wbmp", sizeof(database[51].type));
	strlcpy(database[51].extension, "wbmp", sizeof(database[51].extension));

	strlcpy(database[52].type, "image/webp", sizeof(database[52].type));
	strlcpy(database[52].extension, "webp", sizeof(database[52].extension));

	strlcpy(database[53].type, "image/x-icon", sizeof(database[53].type));
	strlcpy(database[53].extension, "ico", sizeof(database[53].extension));

	strlcpy(database[54].type, "image/x-jng", sizeof(database[54].type));
	strlcpy(database[54].extension, "jng", sizeof(database[54].extension));

	strlcpy(database[55].type, "image/x-ms-bmp", sizeof(database[55].type));
	strlcpy(database[55].extension, "bmp", sizeof(database[55].extension));

	strlcpy(database[56].type, "text/css", sizeof(database[56].type));
	strlcpy(database[56].extension, "css", sizeof(database[56].extension));

	strlcpy(database[57].type, "text/mathml", sizeof(database[57].type));
	strlcpy(database[57].extension, "mml", sizeof(database[57].extension));

	strlcpy(database[58].type, "text/plain", sizeof(database[58].type));
	strlcpy(database[58].extension, "txt", sizeof(database[58].extension));

	strlcpy(database[59].type, "text/vnd.sun.j2me.app-descriptor", sizeof(database[59].type));
	strlcpy(database[59].extension, "jad", sizeof(database[59].extension));

	strlcpy(database[60].type, "text/vnd.wap.wml", sizeof(database[60].type));
	strlcpy(database[60].extension, "wml", sizeof(database[60].extension));

	strlcpy(database[61].type, "text/x-component", sizeof(database[61].type));
	strlcpy(database[61].extension, "htc", sizeof(database[61].extension));

	strlcpy(database[62].type, "text/xml", sizeof(database[62].type));
	strlcpy(database[62].extension, "xml", sizeof(database[62].extension));

	strlcpy(database[63].type, "video/mp2t", sizeof(database[63].type));
	strlcpy(database[63].extension, "ts", sizeof(database[63].extension));

	strlcpy(database[64].type, "video/mp4", sizeof(database[64].type));
	strlcpy(database[64].extension, "mp4", sizeof(database[64].extension));

	strlcpy(database[65].type, "video/quicktime", sizeof(database[65].type));
	strlcpy(database[65].extension, "mov", sizeof(database[65].extension));

	strlcpy(database[66].type, "video/webm", sizeof(database[66].type));
	strlcpy(database[66].extension, "webm", sizeof(database[66].extension));

	strlcpy(database[67].type, "video/x-flv", sizeof(database[67].type));
	strlcpy(database[67].extension, "flv", sizeof(database[67].extension));

	strlcpy(database[68].type, "video/x-m4v", sizeof(database[68].type));
	strlcpy(database[68].extension, "m4v", sizeof(database[68].extension));

	strlcpy(database[69].type, "video/x-matroska", sizeof(database[69].type));
	strlcpy(database[69].extension, "mkv", sizeof(database[69].extension));

	strlcpy(database[70].type, "video/x-mng", sizeof(database[70].type));
	strlcpy(database[70].extension, "mng", sizeof(database[70].extension));

	strlcpy(database[71].type, "video/x-ms-wmv", sizeof(database[71].type));
	strlcpy(database[71].extension, "wmv", sizeof(database[71].extension));

	strlcpy(database[72].type, "video/x-msvideo", sizeof(database[72].type));
	strlcpy(database[72].extension, "avi", sizeof(database[72].extension));

	strlcpy(database[73].type, "text/html", sizeof(database[73].type));
	strlcpy(database[73].extension, "html", sizeof(database[73].extension));

	strlcpy(database[74].type, "video/mpeg", sizeof(database[74].type));
	strlcpy(database[74].extension, "mpeg", sizeof(database[74].extension));

	strlcpy(database[75].type, "video/mpeg", sizeof(database[75].type));
	strlcpy(database[75].extension, "mpg", sizeof(database[75].extension));

	strlcpy(database[76].type, "image/jpeg", sizeof(database[76].type));
	strlcpy(database[76].extension, "jpg", sizeof(database[76].extension));

	strlcpy(database[77].type, "application/x-tcl", sizeof(database[77].type));
	strlcpy(database[77].extension, "tcl", sizeof(database[77].extension));

	strlcpy(database[78].type, "application/x-tcl", sizeof(database[78].type));
	strlcpy(database[78].extension, "tk", sizeof(database[78].extension));

	strlcpy(database[79].type, "application/x-perl", sizeof(database[79].type));
	strlcpy(database[79].extension, "pl", sizeof(database[79].extension));

	strlcpy(database[80].type, "application/x-perl", sizeof(database[80].type));
	strlcpy(database[80].extension, "pm", sizeof(database[80].extension));

	strlcpy(database[81].type, "application/octet-stream", sizeof(database[81].type));
	strlcpy(database[81].extension, "bin", sizeof(database[81].extension));

	strlcpy(database[82].type, "application/octet-stream", sizeof(database[82].type));
	strlcpy(database[82].extension, "exe", sizeof(database[82].extension));

	strlcpy(database[83].type, "application/octet-stream", sizeof(database[83].type));
	strlcpy(database[83].extension, "dll", sizeof(database[83].extension));

	strlcpy(database[84].type, "application/octet-stream", sizeof(database[84].type));
	strlcpy(database[84].extension, "iso", sizeof(database[84].extension));

	strlcpy(database[85].type, "application/octet-stream", sizeof(database[85].type));
	strlcpy(database[85].extension, "img", sizeof(database[85].extension));

	strlcpy(database[86].type, "application/octet-stream", sizeof(database[86].type));
	strlcpy(database[86].extension, "msp", sizeof(database[86].extension));

	strlcpy(database[87].type, "application/octet-stream", sizeof(database[87].type));
	strlcpy(database[87].extension, "msm", sizeof(database[87].extension));

	strlcpy(database[88].type, "application/octet-stream", sizeof(database[88].type));
	strlcpy(database[88].extension, "fs", sizeof(database[88].extension));

	strlcpy(database[89].type, "application/octet-stream", sizeof(database[89].type));
	strlcpy(database[89].extension, "msi", sizeof(database[89].extension));

	strlcpy(database[90].type, "application/postscript", sizeof(database[90].type));
	strlcpy(database[90].extension, "ps", sizeof(database[90].extension));

	strlcpy(database[91].type, "application/x-x509-ca-cert", sizeof(database[91].type));
	strlcpy(database[91].extension, "pem", sizeof(database[91].extension));

	strlcpy(database[92].type, "application/x-x509-ca-cert", sizeof(database[92].type));
	strlcpy(database[92].extension, "crt", sizeof(database[92].extension));

	strlcpy(database[93].type, "audio/midi", sizeof(database[93].type));
	strlcpy(database[93].extension, "mid", sizeof(database[93].extension));

	strlcpy(database[94].type, "audio/midi", sizeof(database[94].type));
	strlcpy(database[94].extension, "midi", sizeof(database[94].extension));

	strlcpy(database[95].type, "image/svg+xml", sizeof(database[95].type));
	strlcpy(database[95].extension, "svg", sizeof(database[95].extension));

	strlcpy(database[96].type, "image/svg+xml", sizeof(database[96].type));
	strlcpy(database[96].extension, "svgz", sizeof(database[96].extension));

	strlcpy(database[97].type, "image/tiff", sizeof(database[97].type));
	strlcpy(database[97].extension, "tif", sizeof(database[97].extension));

	strlcpy(database[98].type, "image/tiff", sizeof(database[98].type));
	strlcpy(database[98].extension, "tiff", sizeof(database[98].extension));

	strlcpy(database[99].type, "application/msword", sizeof(database[99].type));
	strlcpy(database[99].extension, "doc", sizeof(database[99].extension));

	strlcpy(database[100].type, "text/gemini", sizeof(database[100].type));
	strlcpy(database[100].extension, "gmi", sizeof(database[100].extension));

	strlcpy(database[101].type, "text/gemini", sizeof(database[101].type));
	strlcpy(database[101].extension, "gemini", sizeof(database[101].extension));

	strlcpy(database[102].type, "text/markdown", sizeof(database[102].type));
	strlcpy(database[102].extension, "md", sizeof(database[102].extension));

	strlcpy(database[103].type, "image/jpeg", sizeof(database[103].type));
	strlcpy(database[103].extension, "jpeg", sizeof(database[103].extension));

}
