find_package(PkgConfig REQUIRED)
pkg_check_modules(HTTPLIB REQUIRED IMPORTED_TARGET cpp-httplib)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)
find_package(PostgreSQL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(durak_server server.cpp scores.cpp database.cpp http.cpp accounts.cpp rooms.cpp match_state.cpp)
target_link_libraries(durak_server PRIVATE durak PkgConfig::HTTPLIB PkgConfig::SODIUM PostgreSQL::PostgreSQL
  nlohmann_json::nlohmann_json OpenSSL::Crypto project_options project_sanitizers)
