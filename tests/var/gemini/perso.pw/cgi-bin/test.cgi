#!/bin/sh

printf "%s %s: cgi_test\r\n" "20 text/plain"

echo "env vars:"
echo $GATEWAY_INTERFACE
echo $SERVER_SOFTWARE
echo $PATH_INFO
echo $QUERY_STRING
