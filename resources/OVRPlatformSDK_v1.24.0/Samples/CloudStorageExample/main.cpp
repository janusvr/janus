#include <chrono>
#include <iostream>
#include <thread>
#include <windows.h>

#include <OVR_Platform.h>

#include "GameState.h"
#include "InputHandler.h"
#include "PlatformManager.h"
#include "RandomGame.h"

#ifndef OCULUS_APP_ID
// Get your App ID from the Oculus Developer Dashboard
#error "Insert the Oculus Application ID from the Developer Dashboard, then delete this error line"
#define OCULUS_APP_ID "put the ID here"
#endif

#include <stdio.h>

int main()
{
  // connect to the local OVRServer process
  if (ovr_PlatformInitializeWindows(OCULUS_APP_ID) == ovrPlatformInitialize_Success)
  {
    InputHandler input;
    PlatformManager platform;
    GameState gameState;
    RandomGame game;

    while (gameState.GetRunState() != RunState::QUIT)
    {
      input.ProcessInput(gameState);

      platform.Tick(gameState);

      game.Tick(gameState);

      using namespace std::chrono_literals;
      std::this_thread::sleep_for(100ms);
    }
  }
  else 
  {
    std::cerr << "Failed to Initialize with Oculus Platform." << std::endl;
  }

  return 0;
}

#include <OVR_PlatformLoader.cpp>
