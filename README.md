# zodiac: a reverse proxy for Gemini

## Introduction
[Gemini](https://gemini.circumlunar.space/) is an internet protocol like Gopher, but modern.

This is a reverse proxy (and, soon, load balancer) for it, written in C++ (so, really really fast), which aims to be as simple as possible.

As a reverse proxy, zodiac only does three things:
- Soothes the Gemini backend creation, by abstracting everything regarding TLS.
- Demultiplexes requests using SNI.
- Returns status code 43 (`PROXY ERROR`) in case the backend doesn't answer.

The backend receives `<URL><CR><LF><Client's IP><CR><LF>`. This way, the specification is not broken (the second line can be ignored), but there's transparency across the proxy.

## Configuration file
An example configuration file is as follows:
```ini
[zodiac]
enabled = mycapsule, other

[mycapsule]
name = jlxip.net ; Common name for this capsule (gemini://jlxip.net), required
default = true ; This is the default capsule
cert = /tmp/cert.pem ; Default is "./cert.pem"
key = /tmp/key.pem ; Default is "./key.pem"
backend = 172.17.0.2 ; Backend IP or domain name. Default is "localhost"
port = 7000 ; Backend, required

[other]
name = other.arpa
port = 7001
```

- The `zodiac` section is the root of the configuration file. Its field `enabled` links to other sections, each describing a capsule.
- On each frontend section, only `name` and `port` are mandatory.
- One of the capsules should be the default, marked with `default = true`.
  - It will be the one provided for requests either without SNI or with an unrecognized server name.
  - It does not require a `name` field, but one can be given to use an existing capsule (see example above).
  - If no default capsule is specified, one will be chosen at random for its TLS certificate, and zodiac will return status code 41 (`NOT FOUND`).
- The `backend` field can be a domain name. In that case, it will be resolved once before accepting requests.

## Additional information
- The [first version](https://github.com/jlxip/zodiac/tree/0.1.0) was written in 4 hours, one hour after [knowing Gemini exists](https://youtu.be/K-en4nEV5Xc).
- Zodiac follows [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html).
- Dependencies: openssl and [iniparser](https://github.com/ndevilla/iniparser).

## Things to do before 1.0.0
- [x] Timeouts
- [x] Multiple capsules
- [ ] Worker threads (global to the server)
- [ ] More config (listening IP, port, timeouts)
- [ ] RR load balancing
- [ ] Config file in other place (`$ZODIAC_CONFIG`)
- [ ] CI/CD
