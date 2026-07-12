add_executable(durak_app main.cpp)

target_link_libraries(durak_app
  PRIVATE
    durak
    project_options
    project_sanitizers
    project_coverage
)