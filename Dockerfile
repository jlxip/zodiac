FROM jlxip/ssockets:0.1.3-docker2 as build

ENV ZODIAC_CONFIG=/config/zodiac.conf

# Make dependencies
RUN apk add --no-cache git make g++ openssl-dev iniparser-dev
# Execution dependencies
RUN apk add --no-cache openssl iniparser libstdc++

RUN git clone https://github.com/jlxip/zodiac ~/zodiac
RUN sed -i 's/\/bin\/bash/\/bin\/sh/g' ~/zodiac/Makefile
RUN make -C ~/zodiac
RUN mkdir /app && mv ~/zodiac/zodiac /app/
RUN chmod -R o+rx /app

# Cleanup
RUN rm -rf ~/zodiac
RUN apk del git make g++ openssl-dev iniparser-dev

# Flatten time
FROM scratch
COPY --from=build / /

USER nobody
CMD ["/app/zodiac"]