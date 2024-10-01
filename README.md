# simple-http

_A simple HTTP server implementation in C_

## Usage

```bash
simple-http [-p <port>] [-d <directory>] [-o <allowed origin>] [-c <config file>]
```

> By default, the server listens on port `80` and allows connections from any origin.
>
> If no arguments are provided, the server will try to load `/usr/local/etc/simple-http.conf` or `/etc/simple-http.conf` if the former does not exist.

## Options

- `-c <config>`: Path to configuration file
- `-p <port>`: Port to listen on (default: `80`)
- `-d <directory>`: Directory to serve files from
- `-o <origin>`: Allow connections from this origin (default: `0.0.0.0`)

## Building

```bash
make
```

## License

This project is licensed under the MIT License.
See [LICENCE](LICENCE)
