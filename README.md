# simple-http

_A simple HTTP server implementation in C_

## Usage

```bash
simple-http [-c <config file>] [-d <directory>] [-h <host>] [-p <port>] [-t <timeout]
```

> By default, the server listens on host `0.0.0.0` port `80` and serves files from `./www`
>
> If no arguments are provided, the server will try to load `/usr/local/etc/simple-http.conf` or `/etc/simple-http.conf` if the former does not exist.

## Options

- `-c <config>`: Path to configuration file (_will ignore other options_)
- `-d <directory>`: Directory to serve files from (default: `./www`)
- `-h <host>`: Host to listen on (default: `0.0.0.0`)
- `-p <port>`: Port to listen on (default: `80`)
- `-t <timeout>`: Timeout in milliseconds (default: `0`, no timeout)

## Building

First, install the required dependencies:

```bash
# Debian/Ubuntu
sudo apt-get install libmagic-dev
```

> On other systems, you may need to install `libmagic-devel` or `file-devel` instead.
> Please refer to your package manager's documentation.

Then, build the project:

```bash
make
```

## License

This project is licensed under the MIT License.
See [LICENCE](LICENCE)
