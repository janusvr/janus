#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include <OVR_Platform.h>

#include "CloudStorageEntry.h"

class GameState;

enum class CloudStorageState { UNITIALIZED, LOADING_METADATA, LOADING_DATA, NORMAL_GAMEPLAY, DELETING_SAVES};

class CloudStorageManager
{
public:
  CloudStorageManager();
  ~CloudStorageManager() = default;

  void Tick(GameState& gameState);
  void ProcessMessageLoadMetadata(ovrMessageHandle message, GameState& gameState);
  void ProcessMessageLoad(ovrMessageHandle message, GameState& gameState);
  void ProcessMessageLoadConflictMetadata(ovrMessageHandle message);
  void ProcessMessageLoadHandle(ovrMessageHandle message, GameState& gameState);
  void ProcessMessageResolveKeepLocal(ovrMessageHandle message);
  void ProcessMessageSave(ovrMessageHandle message, GameState& gameState);
  void ProcessMessageDelete(ovrMessageHandle message);

private:
  // State machine to define the behavior of the Tick() method.
  CloudStorageState CurrentState;

  // Counts the number of load metadata completions so we know to transition to the next state.
  int LoadedBuckets;

  // map of local machine -> high-score which is saved in the MANUAL Cloud Storage Bucket.
  std::map<std::string, int64_t> LocalHighScores;

  // Tracks when we last saved to throttle how often we store to disk.
  std::chrono::time_point<std::chrono::steady_clock> LastSaveTime;

  // local hostname used to record the high score for a specific machine
  const std::string ComputerName;

  // map of (save request -> data) to make sure the memory remains untouched until the Save completes.
  std::map<ovrRequest, std::unique_ptr<uint8_t>> SaveRequests;

  void SaveGameStateToCloudStorage(const GameState& gameState);
  std::map<std::string, int64_t> MergeConflictingSaves(const std::vector<char>& local, const std::vector<char>& remote);
  std::string GetLocalComputerName() const;

  // Each cloud storage entry key will be unique within the bucket so you
  // can store them in a map-like data structure
  std::map<std::string, CloudStorageEntry> LatestHighscoreSave;
  std::map<std::string, CloudStorageEntry> HighscoreSave;
  std::map<std::string, CloudStorageEntry> LocalHighscoresSave;

  // the bucket names are configured on the Oculus Developer Dashboard
  static const std::string LATEST_HIGHSCORE_BUCKET;
  static const std::string HIGHSCORE_BUCKET;
  static const std::string LOCAL_HIGHSCORES_BUCKET;
  static const std::string KEY;
};
