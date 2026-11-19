add_executable(durak_app main.cpp)
set_target_properties(durak_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/app")

target_link_libraries(durak_app
  PRIVATE
    durak
    project_options
    project_sanitizers
    project_coverage
)