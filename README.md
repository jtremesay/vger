# A simplistic and secure Gemini server

**Vger** design is relying on inetd and a daemon to take care of
TLS.  The idea is to delegate TLS and network to daemons which
proved doing it correctly, so vger takes its request from stdin and
output the result to stdout.

The average setup should look like:

```
         client
           ↓           TCP request on port 1965
       relayd or haproxy
       or stunnel on inetd
           ↓           TCP request to a port of choice on localhost
       vger on inetd
```

**Vger** is perfectly secure if run on **OpenBSD**, using `unveil()`
the filesystem access is restricted to one directory (default to
`/var/gemini/`) and with `pledge()` only systems calls related to
reading files and reading input/output are allowed.


# Get the sources

```
git clone https://tildegit.org/solene/vger
```

# Running tests

**Vger** comes with a test suite you can use with `make test`.
It currently expects `md5` command to be available.

Some files under `/var/gemini/` are required to test the code path
without a `-d` parameter.


# How to configure Vger using relayd and inetd

Create directory `/var/gemini/` (I'd allow this to be configured
later), files will be served from there.

Add this line to inetd.conf:

```
11965 stream tcp nowait gemini_user /usr/local/bin/vger vger
```

Add this to relayd.conf
```
log connection
relay "gemini" {
    listen on 163.172.223.238 port 1965 tls
    forward to 127.0.0.1 port 11965
}
```

Make links to the certificates and key files according to relayd.conf documentation
```
# ln -s /etc/ssl/acme/cert.pem /etc/ssl/163.172.223.238\:1965.crt
# ln -s /etc/ssl/acme/private/privkey.pem /etc/ssl/private/163.172.223.238\:1965.key
```

Enable inetd and relayd and start them:
```
# rcctl enable relayd inetd
# rcctl start relayd inetd
```

# Todo

- handle MIME correctly (only output text/gemini currently)
- move things out of main()
- add syslog traces
- write a man page
