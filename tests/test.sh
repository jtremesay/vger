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
if ! [ $OUT = "d11e0c0ff074f5627f2d2af72fd07104" ] ; then echo "error" ; exit 1 ; fi

# default index.gmi file
OUT=$(printf "gemini://host.name\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "3edd48286850d386592403956aec770f" ] ; then echo "error" ; exit 1 ; fi

# default index.gmi file when using a trailing slash
OUT=$(printf "gemini://host.name/\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "3edd48286850d386592403956aec770f" ] ; then echo "error" ; exit 1 ; fi

# default index.gmi file when client specify port
OUT=$(printf "gemini://host.name:1965\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "3edd48286850d386592403956aec770f" ] ; then echo "error" ; exit 1 ; fi

# file from local directory with lang=fr and markdown MIME type
OUT=$(printf "gemini://perso.pw/file.md\r\n" | ../vger -d var/gemini/ -l fr | tee /dev/stderr | $MD5)
if ! [ $OUT = "e663f17730d5ddc24010c14a238e1e78" ] ; then echo "error" ; exit 1 ; fi

# file from local directory with lang=fr and unknwon MIME type (default to text/gemini)
OUT=$(printf "gemini://perso.pw/foobar.unknown\r\n" | ../vger -d var/gemini/ -l fr | tee /dev/stderr | $MD5)
if ! [ $OUT = "649a2e224632b679fd7599eafb13c001" ] ; then echo "error" ; exit 1 ; fi

# redirect file
OUT=$(printf "gemini://perso.pw/old_location\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "28262311ea814e6a481fa365894cfc3f" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts
OUT=$(printf "gemini://perso.pw/index.gmi\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "0d36a423a4e8be813fda4022f08b3844" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts without specifying a file
OUT=$(printf "gemini://perso.pw\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "0d36a423a4e8be813fda4022f08b3844" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts without specifying a file using lang = fr
OUT=$(printf "gemini://perso.pw\r\n" | ../vger -v -d var/gemini/ -l fr | tee /dev/stderr | $MD5)
if ! [ $OUT = "7db981ce93fee268f29324912800f00d" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts and IRI
OUT=$(printf "gemini://virtualhoßt/é è.gmi\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "bd30e2bb2dc2d7d18b5a3cb1af872c70" ] ; then echo "error" ; exit 1 ; fi

# file from local directory using virtualhosts and IRI both with emojis
OUT=$(printf "gemini://⛴//❤️.gmi\r\n" | ../vger -v -d var/gemini/ | tee /dev/stderr | $MD5)
if ! [ $OUT = "9bed6f0c92d9c86cb46a32e845fb7161" ] ; then echo "error" ; exit 1 ; fi

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
    if ! [ $OUT = "f78c481e1614f1713e077b89aba5ab94" ] ; then echo "error" ; exit 1 ; fi

fi

echo "SUCCESS"
