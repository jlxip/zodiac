# zodiac

## Introduction
Zodiac is a low-footprint high-performance reverse proxy (and, very soon, load balancer) for [Gemini](https://gemini.circumlunar.space/), written in C++. Think of it like an nginx for Gemini.

This project:
- Soothes the Gemini backend creation, by abstracting everything regarding TLS.
- Performs demultiplexing of requests via SNI.
- Returns status code 43 (`PROXY ERROR`) in case the backend doesn't answer.
- Can handle many simultaneous connections with very little resources.

The backend receives `<URL><CR><LF><Client's IP><CR><LF>`. This way, the specification is not broken (the second line can be ignored), but there's transparency across the proxy.

## Configuration file
An example configuration file is as follows:
```ini
[zodiac]
;listenIP = 0.0.0.0 ; Listen on all interfaces. This is the default value
;listenPort = 1965 ; This is the default as well
frontTimeout = 3 ; Seconds until connections to zodiac time out. Default is 5 seconds
backTimeout = 10 ; Seconds until connections to backends time out. Default is 5 seconds
hsTimeout = 5 ; Seconds until TLS handshake times out. Default is 5 seconds
workers = 5 ; Read below
buffers = 512 ; Read below
enabled = mycapsule, other ; Read below

; The following is a capsule using all provided options
[mycapsule]
name = jlxip.net ; Common name for this capsule (gemini://jlxip.net), required
default = true ; This is the default capsule
cert = /tmp/cert.pem ; Default is "./cert.pem"
key = /tmp/key.pem ; Default is "./key.pem"
backend = 172.17.0.2 ; Backend IP or domain name. Default is "localhost"
port = 7000 ; Backend, required
frontTimeout = 1 ; You can override this value for a given capsule
backTimeout = 1  ; and this one

; The following is a capsule using the defaults
[other]
name = other.arpa
port = 7001
```

- The configuration file defaults to `./zodiac.conf`. A different one can be specified via `$ZODIAC_CONFIG`.
- The `[zodiac]` section is the root of the configuration file. Its field `enabled` links to other sections, each describing a capsule.
- The `workers` key in `[zodiac]` specifies the number of worker threads.
  - Default value is `0`, which means _as many as threads in the CPU_.
  - Do not use more than the number of cores: it will throttle. They are not directly related to how many simultaneous connections zodiac can handle.
- The `buffers` key specifies how many pages should be reserved for proxying.
  - This does actually put a limit to the number of simultaneous connections. If no buffers are available, new connections will be kept open waiting until there's one.
  - Pages are balanced across worker threads in order to maximize L1 cache hits, so `buffers` should be a multiple of `workers`. If it's not, it will be rounded down to the closest multiple.
  - In most architectures, regular pages are 4KiB long.
  - The default value is 256 (so 1MiB total).
- Regarding timeouts:
  - Timeouts refer to the whole duration of the process, not only idle time.
  - They cannot be disabled due to security issues. `0` will not disable them.
  - `frontTimeout` and `backTimeout` can be overriden in a capsule section, but not `hsTimeout`, since before the handshake finishes the capsule cannot be identified.
  - `backTimeout` is used both for the backend connection and the response reception, so bear in mind, if you want to dive this deep, that a connection to the backend could take at max 2x`backTimeout`.
- In each capsule section, only `name` and `port` are mandatory.
- One of the capsules can be the default, marked with `default = true`.
  - It will be the one provided for requests either without SNI or with an unrecognized server name.
  - It does not require a `name` field, but one can be given to use an existing capsule (see example above).
  - If no default capsule is specified, one will be chosen at random for its TLS certificate, and zodiac will return status code 41 (`NOT FOUND`).
- The `backend` field can be a domain name. In that case, it will be resolved once before accepting requests.
- If you want capsules on different ports, you can run two instances of zodiac without issues. I'm not implementing it since it makes things messy.

## Additional information
- The [first version](https://github.com/jlxip/zodiac/tree/0.1.0) was written in 4 hours, one hour after [knowing Gemini exists](https://youtu.be/K-en4nEV5Xc).
- Zodiac follows [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html).
- Dependencies: openssl and [iniparser](https://github.com/ndevilla/iniparser).

## Things to do before 1.0.0
- [x] Timeouts
- [x] Multiple capsules
- [x] Worker threads
- [x] More config (listening IP, port, timeouts)
- [ ] RR load balancing
- [x] Config file in other place (`$ZODIAC_CONFIG`)
- [ ] Move to epoll
- [ ] CI/CD
- [ ] Docker image
- [ ] Stress test

## Special thanks
- To @Ten0 for [their updatable priority queue implementation](https://github.com/Ten0/updatable_priority_queue), included as a submodule at `src/upq`, which makes the worker thread scheduler much simpler.
