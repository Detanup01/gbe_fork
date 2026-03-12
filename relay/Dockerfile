FROM golang:1.22-alpine AS build

WORKDIR /src

COPY go.mod ./
COPY cmd ./cmd
COPY internal ./internal
COPY pkg ./pkg

RUN CGO_ENABLED=0 GOOS=linux go build -trimpath -ldflags="-s -w" -o /out/goldberg-relay ./cmd/server

FROM alpine:3.20

RUN addgroup -S goldberg && adduser -S -G goldberg goldberg

WORKDIR /app

COPY --from=build /out/goldberg-relay /usr/local/bin/goldberg-relay
COPY config.example.json /app/config.json

USER goldberg

ENTRYPOINT ["/usr/local/bin/goldberg-relay"]
CMD ["-config", "/app/config.json"]
