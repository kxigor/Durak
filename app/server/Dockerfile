FROM debian:trixie-slim AS build
RUN apt-get update && apt-get install -y --no-install-recommends cmake ninja-build g++ pkg-config libgtest-dev libcpp-httplib-dev libpq-dev nlohmann-json3-dev libssl-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY CMakeLists.txt ./
COPY cmake cmake
COPY include include
COPY src src
COPY tests tests
COPY app/server app/server
RUN cmake -S . -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSANITIZER_MODE=None -DDURAK_BUILD_WEB=ON -DDURAK_BUILD_APP=OFF -DDURAK_ENABLE_FORMAT_CHECK=OFF -DDURAK_ENABLE_TIDY_CHECK=OFF && cmake --build /build --parallel 2 && ctest --test-dir /build --output-on-failure

FROM debian:trixie-slim
RUN apt-get update && apt-get install -y --no-install-recommends libcpp-httplib0.18 libpq5 libssl3t64 curl && rm -rf /var/lib/apt/lists/*
COPY --from=build /build/app/server/durak_server /usr/local/bin/durak_server
USER 65534:65534
EXPOSE 8080
CMD ["durak_server"]
