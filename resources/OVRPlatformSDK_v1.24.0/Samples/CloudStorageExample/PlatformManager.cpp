#include "PlatformManager.h"

#include <iostream>

#include <OVR_Platform.h>

#include "GameState.h"

PlatformManager::PlatformManager()
{
  ovr_Entitlement_GetIsViewerEntitled();
}

void PlatformManager::Tick(GameState& gameState)
{
  // first give the cloud storage manager some processing time
  CloudStorage.Tick(gameState);

  // process all pending messages on the LibOVRPlatform message queue
  ovrMessageHandle message = nullptr;
  while ((message = ovr_PopMessage()) != nullptr)
  {
    if (ovr_Message_IsError(message))
    {
      auto error = ovr_Message_GetError(message);
      std::cout << "Unexpected Error Processing message " << ovr_Message_GetType(message) << std::endl;
      std::cout << "Error Code: " << ovr_Error_GetCode(error) << std::endl;
      std::cout << "Error Message: " << ovr_Error_GetMessage(error) << std::endl;

      if (ovr_Message_GetType(message) == ovrMessage_Entitlement_GetIsViewerEntitled) 
      {
        std::cout << "Error validating Application Entitlement.\n" << std::endl;
        std::cout << "Make sure OCULUS_APP_ID is set correctly." << std::endl;
        std::cout << "Make sure you are entitled from the Oculus Developer Dashboard." << std::endl << std::flush;

        // if the Entitlement check failed we might as well quit since no further Platform calls will work.
        gameState.SetRunState(RunState::QUIT);
      }
    }
    else
    {
      switch (ovr_Message_GetType(message))
      {
      case ovrMessage_Entitlement_GetIsViewerEntitled:
        ProcessMessageCheckEntitlement(message, gameState);
        break;

      case ovrMessage_CloudStorage_LoadBucketMetadata:
        CloudStorage.ProcessMessageLoadMetadata(message, gameState);
        break;

      case ovrMessage_CloudStorage_Load:
        CloudStorage.ProcessMessageLoad(message, gameState);
        break;

      case ovrMessage_CloudStorage_LoadConflictMetadata:
        CloudStorage.ProcessMessageLoadConflictMetadata(message);
        break;

      case ovrMessage_CloudStorage_LoadHandle:
        CloudStorage.ProcessMessageLoadHandle(message, gameState);
        break;

      case ovrMessage_CloudStorage_Save:
        CloudStorage.ProcessMessageSave(message, gameState);
        break;

      case ovrMessage_CloudStorage_ResolveKeepLocal:
        CloudStorage.ProcessMessageResolveKeepLocal(message);
        break;

      case ovrMessage_CloudStorage_Delete:
        CloudStorage.ProcessMessageDelete(message);
        break;

      default:
        std::cout << "Unhandled Platform Message: " << ovr_Message_GetType(message) << std::endl;
      }
    }
    ovr_FreeMessage(message);
  }
}

void PlatformManager::ProcessMessageCheckEntitlement(ovrMessageHandle message, GameState& gameState)
{
  if (gameState.GetRunState() == RunState::INITIALIZING)
  {
   // next step in startup is to load our saved games from cloud storage
    gameState.SetRunState(RunState::LOADING_SAVES);
  }
}
