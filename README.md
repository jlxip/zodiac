# zodiac: a reverse proxy for Gemini

## Introduction
[Gemini](https://gemini.circumlunar.space/) is an internet protocol like Gopher, but modern.

This is a reverse proxy (and, soon, load balancer) for it, written in C++ (so, really really fast), which aims to be as simple as possible. As a reverse proxy, it only does two things:
- Soothe the Gemini backend server creation, by not having to worry about TLS.
- Return status code 43 (`PROXY ERROR`) in case the backend doesn't answer.

The backend receives `<URL><CR><LF><Client's IP><CR><LF>`. This way, the specification is not broken (the second line can be ignored), but there's also transparency across the proxy.

## Configuration file
A simple configuration file is as follows:
```ini
[server]
cert = /tmp/cert.pem ; Default is "./cert.pem"
key = /tmp/key.pem ; Default is "./key.pem"
server = 172.17.0.2 ; Backend. Default is "localhost"
port = 7000 ; Backend, required
```

The `server` field can be a domain name. In that case, it will be resolved once before accepting requests.

## Additional information
- The [first version](https://github.com/jlxip/zodiac/tree/0.1.0) was written in 4 hours, one hour after [knowing it exists](https://youtu.be/K-en4nEV5Xc).
- Zodiac follows [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html).
- Dependencies: openssl and [iniparser](https://github.com/ndevilla/iniparser).

## Things to do before 1.0.0
- [x] Timeouts
- [ ] More than one server
- [ ] Worker threads (global to the server)
- [ ] More config (listening IP, port, timeouts)
- [ ] RR load balancing
- [ ] Config file in other place (`$ZODIAC_CONFIG`)
- [ ] CI/CD