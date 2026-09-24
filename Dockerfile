FROM gcc:14-bookworm AS builder

WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libedit-dev \
    && rm -rf /var/lib/apt/lists/*

COPY Makefile ./
COPY include ./include
COPY src ./src
COPY vendor ./vendor

RUN make release


FROM gcc:14-bookworm

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        bash \
        coreutils \
        libedit2 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/bin/run /usr/local/bin/rush

ENTRYPOINT ["/usr/local/bin/rush"]
