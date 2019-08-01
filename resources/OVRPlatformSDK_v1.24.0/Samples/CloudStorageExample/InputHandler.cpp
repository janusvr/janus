#include "InputHandler.h"

#include <conio.h>
#include <iostream>

#include "GameState.h"

void InputHandler::ProcessInput(GameState &gameState) {
  if (_kbhit())
  {
    int key = _getch();
    switch (key)
    {
    case 27: // escape
      if (gameState.GetRunState() == RunState::PLAYING)
      {
        gameState.SetRunState(RunState::QUITTING);
      }
      break;

    case 'D':
      if (gameState.GetRunState() == RunState::PLAYING)
      {
        gameState.SetRunState(RunState::RESETTING);
      }
      break;

    default:
        std::cout << "Global High Score: " << gameState.GetGlobalHighScore() << std::endl;
        std::cout << "Local High Score: " << gameState.GetLocalHighScore() << std::endl;
        if (gameState.GetRunState() == RunState::PLAYING)
        {
          std::cout << "Press 'D' to delete all saved data and reset the game." << std::endl;
          std::cout << "Press ESC to quit." << std::endl;
        }
    }
  }
}
