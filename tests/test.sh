#!/bin/sh

set -x

# md5 is BSD md5 binary
# Linux uses md5sum
which md5 && MD5CMD="md5" || MD5CMD="md5sum"

MD5()
{
	$MD5CMD | awk '{print $1}'
}

# serving a file
OUT=$(printf "gemini://host.name/main.gmi\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | MD5)
if ! [ $OUT = "c7e352d6aae4ee7e7604548f7874fb9d" ] ; then echo "error" ; exit 1 ; fi

# default index.gmi file
OUT=$(printf "gemini://host.name\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | MD5)
if ! [ $OUT = "fcc5a293f316e01f7b3103f97eca26b1" ] ; then echo "error" ; exit 1 ; fi

# default index.gmi file when using a trailing slash
OUT=$(printf "gemini://host.name/\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | MD5)
if ! [ $OUT = "fcc5a293f316e01f7b3103f97eca26b1" ] ; then echo "error" ; exit 1 ; fi

# default index.gmi file when client specify port
OUT=$(printf "gemini://host.name:1965\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | MD5)
if ! [ $OUT = "fcc5a293f316e01f7b3103f97eca26b1" ] ; then echo "error" ; exit 1 ; fi

# redirect to uri with trailing / if directory
OUT=$(printf "gemini://host.name/subdir\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | MD5)
if ! [ $OUT = "84e5e7bb3eee0dfcc8db14865dc83e77" ] ; then echo "error" ; exit 1 ; fi

# redirect to uri with trailing / if directory and vhost enabled
OUT=$(printf "gemini://perso.pw/cgi-bin\r\n" | ../vger -vd var/gemini | tee /dev/stderr | MD5)
if ! [ $OUT = "c782da4173898f57033a0804b8e96fc3" ] ; then echo "error" ; exit 1 ; fi

# file from local directory with lang=fr and markdown MIME type
OUT=$(printf "gemini://perso.pw/file.md\r\n" | ../vger -d var/gemini/ -l fr | tee /dev/stderr | MD5)
if ! [ $OUT = "e663f17730d5ddc24010c14a238e1e78" ] ; then echo "error" ; exit 1 ; fi

# file from local directory with lang=fr and unknown MIME type (default to application/octet-stream)
OUT=$(printf "gemini://perso.pw/foobar.unknown\r\n" | ../vger -d var/gemini/ -l fr | tee /dev/stderr | MD5)
if ! [ $OUT = "a23b0053d759863a45da4afbffd847d2" ] ; then echo "error" ; exit 1 ; fi

# file from local directory and unknown MIME type, default forced to text/plain
OUT=$(printf "gemini://perso.pw/foobar.unknown\r\n" | ../vger -d var/gemini/ -m text/plain | tee /dev/stderr | MD5)
if ! [ $OUT = "383a5a5ddb7bb30e3553ecb666378ebc" ] ; then echo "error" ; exit 1 ; fi

# redirect file
OUT=$(printf "gemini://perso.pw/old_location\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | MD5)
if ! [ $OUT = "cb4597b6fcc82cbc366ac9002fb60dac" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts
OUT=$(printf "gemini://perso.pw/index.gmi\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | MD5)
if ! [ $OUT = "5e5fca557e79f4521b21d4b81dc964c6" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts without specifying a file
OUT=$(printf "gemini://perso.pw\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | MD5)
if ! [ $OUT = "5e5fca557e79f4521b21d4b81dc964c6" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts without specifying a file using lang = fr
OUT=$(printf "gemini://perso.pw\r\n" | ../vger -v -d var/gemini/ -l fr | tee /dev/stderr | MD5)
if ! [ $OUT = "7db981ce93fee268f29324912800f00d" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts and IRI
OUT=$(printf "gemini://virtualhoßt/é è.gmi\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | MD5)
if ! [ $OUT = "282cee071d3bd20dbb6e6af38f217a29" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts and IRI both with emojis
OUT=$(printf "gemini://⛴//❤️.gmi\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | MD5)
if ! [ $OUT = "e354a1a29ea8273faaf0cdc29c1d8583" ] ; then echo "error" ; exit 1 ; fi

# auto index in directory without index.gmi must redirect
OUT=$(printf "gemini://host.name/autoidx\r\n" | ../vger -d var/gemini/ -i | tee /dev/stderr | MD5)
if ! [ $OUT = "874f5e1af67eff6b93bedf8ac8033066" ] ; then echo "error" ; exit 1 ; fi

# auto index in directory
OUT=$(printf "gemini://host.name/autoidx/\r\n" | ../vger -d var/gemini/ -i | tee /dev/stderr | MD5)
if ! [ $OUT = "515bcb4ba5f8869360f53afe2841e044" ] ; then echo "error" ; exit 1 ; fi

# cgi simple script
OUT=$(printf "gemini://host.name/cgi-bin/test.cgi\r\n" | ../vger -d var/gemini/ -c /cgi-bin | tee /dev/stderr | MD5)
if ! [ $OUT = "666e48200f90018b5e96c2cf974882dc" ] ; then echo "error" ; exit 1 ; fi

# cgi with use of variables
OUT=$(printf "gemini://host.name/cgi-bin/who.cgi?user=jean-mi\r\n" | ../vger -d var/gemini/ -c /cgi-bin | tee /dev/stderr | MD5)
if ! [ $OUT = "fa065a67d1f7c973501d4a9e3ca2ea57" ] ; then echo "error" ; exit 1 ; fi

# cgi with error
OUT=$(printf "gemini://host.name/cgi-bin/nope\r\n" | ../vger -d var/gemini/ -c /cgi-bin | tee /dev/stderr | MD5)
if ! [ $OUT = "4156170c2aa8a6a8a0892ff5a61bf5f5" ] ; then echo "error" ; exit 1 ; fi

# remove ?.* if any
OUT=$(printf "gemini://host.name/main.gmi?anything-here\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | MD5)
if ! [ $OUT = "c7e352d6aae4ee7e7604548f7874fb9d" ] ; then echo "error" ; exit 1 ; fi

# virtualhost + cgi
OUT=$(printf "gemini://perso.pw/cgi-bin/test.cgi\r\n" | ../vger -v -d var/gemini/ -c perso.pw/cgi-bin | tee /dev/stderr | MD5)
if ! [ $OUT = "666e48200f90018b5e96c2cf974882dc" ] ; then echo "error" ; exit 1 ; fi

# must fail only on OpenBSD !
# try to escape from unveil
if [ -f /bsd ]
then
    OUT=$(printf "gemini://fail_on_openbsd/../../test.sh\r\n" | ../vger -d var/gemini/ -l fr | tee /dev/stderr | MD5)
    if [ $OUT = "$( ( printf '20 text/gemini; lang=fr\r\n' ; cat $0) | MD5)" ] ; then echo "error" ; exit 1 ; fi
fi

#type doas 2>/dev/null
#if [ $? -eq 0 ]; then
#    # file from local directory chroot
#    OUT=$(printf "gemini://perso.pw\r\n" | doas ../vger -v -d var/gemini/ -u solene -l fr | tee /dev/stderr | MD5)
#    if ! [ $OUT = "7db981ce93fee268f29324912800f00d" ] ; then echo "error" ; exit 1 ; fi
#fi

echo "SUCCESS"
