#pragma once

#include <OVR_Platform.h>

#include "CloudStorageManager.h"

class GameState;

class PlatformManager {
public:
  PlatformManager();
  ~PlatformManager() = default;

  void Tick(GameState& gameState);

private:
  void ProcessMessageCheckEntitlement(ovrMessageHandle message, GameState& gameState);

  CloudStorageManager CloudStorage;
};
