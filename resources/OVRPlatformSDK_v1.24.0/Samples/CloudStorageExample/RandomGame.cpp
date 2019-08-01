#include "RandomGame.h"

#include <random>
#include <iostream>

#include "GameState.h"

std::random_device rd;
std::mt19937_64 random(rd());
std::normal_distribution<> dist(0, std::numeric_limits<int32_t>::max());

void RandomGame::Tick(GameState& gameState)
{
  if (gameState.GetRunState() == RunState::PLAYING)
  {
    int64_t score = std::llround(dist(random));

    if (score > gameState.GetLocalHighScore())
    {
      gameState.SetLocalHighScore(score);
      std::cout << "New Local High Score: " << score << std::endl;
    }
    if (score > gameState.GetGlobalHighScore())
    {
      gameState.SetGlobalHighScore(score);
      std::cout << "New Global High Score: " << score << " / " << std::numeric_limits<int64_t>::max() << std::endl;
    }
  }
}
