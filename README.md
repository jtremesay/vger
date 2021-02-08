# A simplistic and secure Gemini server

**Vger** is a gemini server supporting chroot, virtualhosts, CGI, default language choice, redirections and MIME types detection.

**Vger** design is relying on inetd and a daemon to take care of TLS.  The idea is to delegate TLS and network to daemons which proved doing it correctly, so vger takes its request from stdin and output the result to stdout.

The average setup should look like:

```
         client
           ↓           TCP request on port 1965
       relayd or haproxy
       or stunnel on inetd
           ↓           TCP request to a port of choice on localhost
       vger on inetd
```

**Vger** is perfectly secure if run on **OpenBSD**, using `unveil()` the filesystem access is restricted to one directory (default to `/var/gemini/`) and with `pledge()` only systems calls related to reading files and reading input/output are allowed. More explanations about Vger security can be found [on this link](https://dataswamp.org/~solene/2021-01-14-vger-security.html).

For all supported OS, it's possible to run **Vger** in a chroot and drop privileges to a dedicated user.


# Install

```
git clone https://tildegit.org/solene/vger.git
cd vger
make
doas make install
```

On GNU/Linux, make sure you installed `libbsd`.

# Running tests

**Vger** comes with a test suite you can use with `make test`.

Some files under `/var/gemini/` are required to test the code path without a `-d` parameter.


# Command line parameters

**Vger**  has a few parameters you can use in inetd configuration.

- `-d PATH`: use `PATH` as the data directory to serve files from. Default is `/var/gemini`
- `-l LANG`: change the language in the status return code. Default is no language specified.
- `-v`: enable virtualhost support, the hostname in the query will be considered as a directory name.
- `-u username`: enable chroot to the data directory and drop privileges to `username`.
- `-m MIME` : use MIME as default instead of "application/octet-stream".
- `-i` : Enable auto index if no "index.gmi" file is found in a directory.
- `-c CGI_PATH` : files in CGI_PATH are executed and their output is returned to the client.


# How to configure Vger using relayd and inetd

Create directory `/var/gemini/` (I'd allow this to be configured later), files will be served from there.

Create an user `gemini_user`.

Add this line to inetd.conf:

```
127.0.0.1:11965 stream tcp nowait gemini_user /usr/local/bin/vger vger
```

Add this to relayd.conf
```
log connection
tcp protocol "gemini" {
    tls keypair hostname.example
}

relay "gemini" {
    listen on hostname.example port 1965 tls
    protocol "gemini"
    forward to 127.0.0.1 port 11965
}
```

Make sure certificates files match hostname:
`/etc/ssl/private/hostname.example.key` and
`/etc/ssl/hostname.example.crt`.

On OpenBSD, enable inetd and relayd and start them:
```
# rcctl enable relayd inetd
# rcctl start relayd inetd
```

Don't forget to open the TCP port 1965 in your firewall.

Vger will serve files named `index.gmi` if no explicit filename is given. If this file doesn't exist and auto index is enabled, an index file with a link to every file in the directory will be served.
