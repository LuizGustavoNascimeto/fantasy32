FROM alpine:latest
WORKDIR /app
RUN apk update && apk add --no-cache \
    g++ \
    make \
    cmake
COPY . .
RUN make -j$(nproc)          
CMD ["./build/app"]          