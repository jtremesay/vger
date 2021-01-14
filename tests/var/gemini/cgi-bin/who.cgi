#!/bin/sh

printf "%s %s: cgi_test\r\n" "20 text/plain"

u=""
if [ -n "${QUERY_STRING}" ]; then
	u="$(printf "%s" "${QUERY_STRING}" | cut -d'=' -f2)" #yeah, it's awful..
fi

echo "hello $u"
