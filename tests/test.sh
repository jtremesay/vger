#!/bin/sh

set -x

# serving a file
OUT=$(printf "gemini://host.name/main.gmi\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | md5)
if ! [ $OUT = "d11e0c0ff074f5627f2d2af72fd07104" ] ; then echo "error" ; exit 1 ; fi

# default index.gmi file
OUT=$(printf "gemini://host.name\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | md5)
if ! [ $OUT = "3edd48286850d386592403956aec770f" ] ; then echo "error" ; exit 1 ; fi

# default index.gmi file when using a trailing slash
OUT=$(printf "gemini://host.name/\r\n" | ../vger -d var/gemini/ | tee /dev/stderr | md5)
if ! [ $OUT = "3edd48286850d386592403956aec770f" ] ; then echo "error" ; exit 1 ; fi

# file from /var/gemini/index.md
OUT=$(printf "gemini://host.name/index.md\r\n" | ../vger | tee /dev/stderr | md5)
if ! [ $OUT = "bdbb22f0d1f4dd9e31bfc91686e7441d" ] ; then echo "error" ; exit 1 ; fi

#### no -d parameter from here

if [ -d /var/gemini/ ]
then

    # file from /var/gemini/blog/
    OUT=$(printf "gemini://host.name/blog/\r\n" | ../vger | tee /dev/stderr | md5)
    if ! [ $OUT = "83bd01c9af0e44d5439b9ac95dc28132" ] ; then echo "error" ; exit 1 ; fi

    # file from /var/gemini/blog
    OUT=$(printf "gemini://host.name/blog\r\n" | ../vger | tee /dev/stderr | md5)
    if ! [ $OUT = "f78c481e1614f1713e077b89aba5ab94" ] ; then echo "error" ; exit 1 ; fi

fi
