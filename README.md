# zodiac

## Introduction
Zodiac is a low-footprint high-performance reverse proxy and load balancer for [Gemini](https://gemini.circumlunar.space/), written in C++. Think of it like an nginx for Gemini, meant to connect to FastCGI-like local servers, or _backends_.

This project:
- Soothes the Gemini backend creation, since they don't need to worry about:
  - TLS
  - Timeouts
  - CRLF, meaning that it's guaranteed to be there
- Performs demultiplexing of requests via SNI.
- Returns status code 43 (`PROXY ERROR`) in case the backend doesn't answer.
- Can handle many simultaneous connections with very little resources.

## Configuration file
The configuration file defaults to `./zodiac.conf`, but a different one can be specified via the `ZODIAC_CONFIG` environment variable.

An example config is given in the [example.conf](https://github.com/jlxip/zodiac/blob/master/example.conf) file at the root of the repository.

## FastCGI-like protocol
The backend receives `<URL><CR><LF><Client's IP><CR><LF>`. This way, the proxy is transparent while the specification is not broken, since the second line can be ignored.

## Docker
A Dockerfile is available at the root of this repository. It is automatically built and pushed to the Docker Hub on each release, with the name [jlxip/zodiac](https://hub.docker.com/r/jlxip/zodiac). To use it, run something along the lines of:

```bash
docker volume create zodiac_config
sudo cp yourconfig.conf /var/lib/docker/volumes/zodiac_config/_data/zodiac.conf
docker run -d -p 1965:1965 --name myserver --restart=unless-stopped -v zodiac_config:/config jlxip/zodiac:latest
```

## Additional information
- The [first version](https://github.com/jlxip/zodiac/tree/0.1.0) was written in 4 hours, one hour after [knowing Gemini exists](https://youtu.be/K-en4nEV5Xc).
- Zodiac follows [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html).
- Dependencies: openssl, [iniparser](https://github.com/ndevilla/iniparser), and [ssockets](https://github.com/jlxip/ssockets).

## Backends for zodiac
- [bsgemini](https://github.com/jlxip/bsgemini), by me, for static servers, written in C++.

## Things to do before 1.0.0
- [x] Timeouts
- [x] Multiple capsules
- [x] Worker threads
- [x] More config (listening IP, port, timeouts)
- [x] RR load balancing
- [x] Config file in other place (`$ZODIAC_CONFIG`)
- [x] Move to epoll
- [x] CI/CD
- [x] Docker image
