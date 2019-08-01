#include "GameState.h"

#include <limits>

GameState::GameState()
  : CurrentRunState(RunState::INITIALIZING)
  , GlobalHighScore(std::numeric_limits<int64_t>::min())
  , LocalHighScore(std::numeric_limits<int64_t>::min())
  , GlobalNeedsSaving(false)
  , LocalNeedsSaving(false)
{
}

void GameState::SetGlobalHighScore(int64_t score)
{
  GlobalHighScore = score;
  GlobalNeedsSaving = true;
}

void GameState::SetLocalHighScore(int64_t score)
{
  LocalHighScore = score;
  LocalNeedsSaving = true;
}


