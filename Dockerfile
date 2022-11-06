FROM alpine:3.16

ENV ZODIAC_CONFIG=/config/zodiac.conf

RUN ["wget", "https://github.com/jlxip/ssockets/releases/latest/download/libssockets.so", "-O", "/usr/lib/libssockets.so"]

RUN ["mkdir", "/app"]
RUN ["chown", "nobody:nobody", "/app"]
COPY --chown=nobody:nobody ./zodiac /app/zodiac

RUN ["apk", "add", "--no-cache", "gcompat", "libstdc++", "openssl", "iniparser"]

USER nobody
CMD ["/app/zodiac"]