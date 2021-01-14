#!/bin/sh

echo "20 cgi_test"
u=""
if [ -n "${QUERY_STRING}" ]; then
	u="$(printf "%s" "${QUERY_STRING}" | cut -d'=' -f2)" #yeah, it's awful..
fi

echo "hello $u"
