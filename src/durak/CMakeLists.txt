add_library(durak
  engine.cpp
  table.cpp
  player.cpp
  player_view.cpp
  game_rule_view.cpp
)

target_include_directories(durak
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)
