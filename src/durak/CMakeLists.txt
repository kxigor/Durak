add_library(durak
  engine.cpp
)

target_include_directories(durak
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)
