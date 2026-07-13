add_gtest(Modulo
  SOURCES modulo.cpp
  LIBRARIES
    project_options
    project_coverage
    durak
)

add_gtest(Table
  SOURCES table.cpp
  LIBRARIES
    project_options
    project_coverage
    durak
)

add_gtest(Player
  SOURCES player.cpp
  LIBRARIES
    project_options
    project_coverage
    durak
)

add_gtest(GameRuleView
  SOURCES game_rule_view.cpp
  LIBRARIES
    project_options
    project_coverage
    durak
)

add_gtest(PlayerView
  SOURCES player_view.cpp
  LIBRARIES
    project_options
    project_coverage
    durak
)

add_gtest(Engine
  SOURCES engine.cpp
  LIBRARIES
    project_options
    project_coverage
    durak
)