#pragma once

class GameState;

class RandomGame
{
public:
  RandomGame() = default;
  ~RandomGame() = default;

  void Tick(GameState& gameState);
};
