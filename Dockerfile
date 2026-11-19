FROM debian:trixie-slim AS build
RUN apt-get update && apt-get install -y --no-install-recommends cmake ninja-build g++ pkg-config libgtest-dev libcpp-httplib-dev libsqlite3-dev nlohmann-json3-dev libssl-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY CMakeLists.txt ./
COPY cmake cmake
COPY include include
COPY src src
COPY tests tests
COPY backend backend
RUN cmake -S . -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSANITIZER_MODE=None -DDURAK_BUILD_WEB=ON -DDURAK_BUILD_APP=OFF -DDURAK_ENABLE_FORMAT_CHECK=OFF -DDURAK_ENABLE_TIDY_CHECK=OFF && cmake --build /build --parallel 2 && ctest --test-dir /build --output-on-failure

FROM debian:trixie-slim
RUN apt-get update && apt-get install -y --no-install-recommends libcpp-httplib0.18 libsqlite3-0 libssl3t64 curl && rm -rf /var/lib/apt/lists/* && install -d -o 10001 -g 10001 -m 0700 /data && touch /data/.keep && chown 10001:10001 /data/.keep
COPY --from=build /build/backend/durak_server /usr/local/bin/durak_server
USER 10001:10001
EXPOSE 8080
CMD ["durak_server"]
