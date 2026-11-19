find_package(PkgConfig REQUIRED)
pkg_check_modules(HTTPLIB REQUIRED IMPORTED_TARGET cpp-httplib)
find_package(PostgreSQL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(durak_server server.cpp scores.cpp)
target_link_libraries(durak_server PRIVATE durak PkgConfig::HTTPLIB PostgreSQL::PostgreSQL
  nlohmann_json::nlohmann_json OpenSSL::Crypto project_options project_sanitizers)
