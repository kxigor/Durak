add_gtest(DeckInit
  SOURCES deck_init.cpp
  LIBRARIES
    project_options
    project_coverage
    durak
)

add_gtest(FirstTurn
  SOURCES first_turn.cpp
  LIBRARIES
    project_options
    project_coverage
    durak
)

add_gtest(RoundStart
  SOURCES round_start.cpp
  LIBRARIES
    project_options
    project_coverage
    durak
)

add_gtest(RoundEnd
  SOURCES round_end.cpp
  LIBRARIES
    project_options
    project_coverage
    durak
)
