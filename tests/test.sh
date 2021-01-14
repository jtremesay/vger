#!/bin/sh

set -x

# md5 is BSD md5 binary
# Linux uses md5sum
MD5=md5
type md5 2>/dev/null
if [ $? -ne 0 ]; then
    MD5=md5sum
fi

# serving a file
OUT=$(printf "gemini://host.name/main.gmi\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "c7e352d6aae4ee7e7604548f7874fb9d" ] ; then echo "error" ; exit 1 ; fi

# default index.gmi file
OUT=$(printf "gemini://host.name\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "fcc5a293f316e01f7b3103f97eca26b1" ] ; then echo "error" ; exit 1 ; fi

# default index.gmi file when using a trailing slash
OUT=$(printf "gemini://host.name/\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "fcc5a293f316e01f7b3103f97eca26b1" ] ; then echo "error" ; exit 1 ; fi

# default index.gmi file when client specify port
OUT=$(printf "gemini://host.name:1965\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "fcc5a293f316e01f7b3103f97eca26b1" ] ; then echo "error" ; exit 1 ; fi

# redirect to uri with trailing / if directory
OUT=$(printf "gemini://host.name/subdir\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "84e5e7bb3eee0dfcc8db14865dc83e77" ] ; then echo "error" ; exit 1 ; fi

# file from local directory with lang=fr and markdown MIME type
OUT=$(printf "gemini://perso.pw/file.md\r\n" | ../vger -d var/gemini/ -l fr | tee /dev/stderr | $MD5)
if ! [ $OUT = "e663f17730d5ddc24010c14a238e1e78" ] ; then echo "error" ; exit 1 ; fi

# file from local directory with lang=fr and unknown MIME type (default to application/octet-stream)
OUT=$(printf "gemini://perso.pw/foobar.unknown\r\n" | ../vger -d var/gemini/ -l fr | tee /dev/stderr | $MD5)
if ! [ $OUT = "a23b0053d759863a45da4afbffd847d2" ] ; then echo "error" ; exit 1 ; fi

# file from local directory and unknown MIME type, default forced to text/plain
OUT=$(printf "gemini://perso.pw/foobar.unknown\r\n" | ../vger -d var/gemini/ -m text/plain | tee /dev/stderr | $MD5)
if ! [ $OUT = "383a5a5ddb7bb30e3553ecb666378ebc" ] ; then echo "error" ; exit 1 ; fi

# redirect file
OUT=$(printf "gemini://perso.pw/old_location\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "cb4597b6fcc82cbc366ac9002fb60dac" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts
OUT=$(printf "gemini://perso.pw/index.gmi\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "5e5fca557e79f4521b21d4b81dc964c6" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts without specifying a file
OUT=$(printf "gemini://perso.pw\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "5e5fca557e79f4521b21d4b81dc964c6" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts without specifying a file using lang = fr
OUT=$(printf "gemini://perso.pw\r\n" | ../vger -v -d var/gemini/ -l fr | tee /dev/stderr | $MD5)
if ! [ $OUT = "7db981ce93fee268f29324912800f00d" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts and IRI
OUT=$(printf "gemini://virtualhoßt/é è.gmi\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "282cee071d3bd20dbb6e6af38f217a29" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts and IRI both with emojis
OUT=$(printf "gemini://⛴//❤️.gmi\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "e354a1a29ea8273faaf0cdc29c1d8583" ] ; then echo "error" ; exit 1 ; fi

# auto index in directory without index.gmi must redirect
OUT=$(printf "gemini://host.name/autoidx\r\n" | ../vger -d var/gemini/ -i | tee /dev/stderr | $MD5)
if ! [ $OUT = "874f5e1af67eff6b93bedf8ac8033066" ] ; then echo "error" ; exit 1 ; fi

# auto index in directory
OUT=$(printf "gemini://host.name/autoidx/\r\n" | ../vger -d var/gemini/ -i | tee /dev/stderr | $MD5)
if ! [ $OUT = "9cb7ef77cbcd74dadafdbff47d864152" ] ; then echo "error" ; exit 1 ; fi

# cgi simple script
OUT=$(printf "gemini://host.name/cgi-bin/test.cgi\r\n" | ../vger -d var/gemini/ -c /cgi-bin | tee /dev/stderr | $MD5)
if ! [ $OUT = "ed3552892cf7edac4f81178467f6c48a" ] ; then echo "error" ; exit 1 ; fi

# cgi with use of variables
OUT=$(printf "gemini://host.name/cgi-bin/who.cgi?user=jean-mi\r\n" | ../vger -d var/gemini/ -c /cgi-bin | tee /dev/stderr | $MD5)
if ! [ $OUT = "06f710962d6504b19cdfe77f5fcff29e" ] ; then echo "error" ; exit 1 ; fi

# cgi with error
OUT=$(printf "gemini://host.name/cgi-bin/nope\r\n" | ../vger -d var/gemini/ -c /cgi-bin | tee /dev/stderr | $MD5)
if ! [ $OUT = "babaa82d5b2bd4e8d21ab412e060c890" ] ; then echo "error" ; exit 1 ; fi

# must fail only on OpenBSD !
# try to escape from unveil
if [ -f /bsd ]
then
    OUT=$(printf "gemini://fail_on_openbsd/../../test.sh\r\n" | ../vger -d var/gemini/ -l fr | tee /dev/stderr | $MD5)
    if [ $OUT = "$( ( printf '20 text/gemini; lang=fr\r\n' ; cat $0) | $MD5)" ] ; then echo "error" ; exit 1 ; fi
fi

#type doas 2>/dev/null
#if [ $? -eq 0 ]; then
#    # file from local directory chroot
#    OUT=$(printf "gemini://perso.pw\r\n" | doas ../vger -v -d var/gemini/ -u solene -l fr | tee /dev/stderr | $MD5)
#    if ! [ $OUT = "7db981ce93fee268f29324912800f00d" ] ; then echo "error" ; exit 1 ; fi
#fi

#### no -d parameter from here

if [ -d /var/gemini/ ]
then

    # file from /var/gemini/index.md
    OUT=$(printf "gemini://host.name/index.md\r\n" | ../vger | tee /dev/stderr | $MD5)
    if ! [ $OUT = "1f7ed3966d50b08ea138b7d8c0a08ec6" ] ; then echo "error" ; exit 1 ; fi


    # file from /var/gemini/blog/
    OUT=$(printf "gemini://host.name/blog/\r\n" | ../vger | tee /dev/stderr | $MD5)
    if ! [ $OUT = "83bd01c9af0e44d5439b9ac95dc28132" ] ; then echo "error" ; exit 1 ; fi

    # file from /var/gemini/blog
    OUT=$(printf "gemini://host.name/blog\r\n" | ../vger | tee /dev/stderr | $MD5)
    if ! [ $OUT = "83bd01c9af0e44d5439b9ac95dc28132" ] ; then echo "error" ; exit 1 ; fi

fi

echo "SUCCESS"
