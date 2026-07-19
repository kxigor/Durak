add_library(durak)

add_subdirectory(engine)
add_subdirectory(rules)

target_link_libraries(durak
  PRIVATE
    project_options
    project_sanitizers
    project_coverage
)

target_include_directories(durak
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)

if(MYPROJ_BUILD_TESTS)
  target_compile_definitions(durak PUBLIC UNIT_TESTING)
endif()