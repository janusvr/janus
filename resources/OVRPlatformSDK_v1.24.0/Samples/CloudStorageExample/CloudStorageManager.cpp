#include "CloudStorageManager.h"

#include <cassert>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#define NOMINMAX
#include <Windows.h>

#include <OVR_Platform.h>

#include "GameState.h"


const std::string CloudStorageManager::LATEST_HIGHSCORE_BUCKET = "Latest High Score";
const std::string CloudStorageManager::HIGHSCORE_BUCKET = "Overall Highest Score";
const std::string CloudStorageManager::LOCAL_HIGHSCORES_BUCKET = "Local High Scores";
const std::string CloudStorageManager::KEY = "Just some unique name for the save";

CloudStorageManager::CloudStorageManager()
  : CurrentState(CloudStorageState::UNITIALIZED)
  , ComputerName(GetLocalComputerName())
{
}

void CloudStorageManager::Tick(GameState& gameState)
{
  // Game Startup.
  // 1) Get the list of all saved entries in each bucket
  // 2) Perform Conflict Resolution if necessary
  // 3) Load the cloud Storage data and set the HighScores
  // 4) Transtion the game state to running
  if (gameState.GetRunState() == RunState::LOADING_SAVES)
  {
    switch (CurrentState) 
    {
    case CloudStorageState::UNITIALIZED:
    {
      CurrentState = CloudStorageState::LOADING_METADATA;
      LoadedBuckets = 0;

      // request the saves for each of the 3 storage buckets that should be setup on the dashboard
      ovr_CloudStorage_LoadBucketMetadata(LATEST_HIGHSCORE_BUCKET.c_str());
      ovr_CloudStorage_LoadBucketMetadata(HIGHSCORE_BUCKET.c_str());
      ovr_CloudStorage_LoadBucketMetadata(LOCAL_HIGHSCORES_BUCKET.c_str());
      break;
    }

    case CloudStorageState::LOADING_METADATA:
      // waiting for all metadata request responses
      if (LoadedBuckets == 3)
      {
        // If all the metadata has loaded, wait for the saved game data
        CurrentState = CloudStorageState::LOADING_DATA;
      }
      break;

    case CloudStorageState::LOADING_DATA:
    {
      // check to see if we've loaded all our saved data yet
      bool loadingData = false;
      for (const auto& entry : LatestHighscoreSave)
      {
        if (!entry.second.HasDataLoaded())
          loadingData = true;
      }
      for (const auto& entry : HighscoreSave)
      {
        if (!entry.second.HasDataLoaded())
          loadingData = true;
      }
      for (const auto& entry : LocalHighscoresSave)
      {
        if (!entry.second.HasDataLoaded())
          loadingData = true;
      }

      // If all the data has loaded, lets start playing the game data
      if (!loadingData) {
        std::cout << "Loaded Global High Score: " << gameState.GetGlobalHighScore() << std::endl;
        for (const auto& score : LocalHighScores)
        {
          std::cout << "Loaded Local High Score: " << score.second << " from " << score.first << std::endl;
        }
        gameState.SetRunState(RunState::PLAYING);
        CurrentState = CloudStorageState::NORMAL_GAMEPLAY;
      }
      break;
    }

    default:
      gameState.SetRunState(RunState::QUIT);
      std::cout << "Unhandled CloudStorageManager state: " << (int)CurrentState << std::endl;
    }
  }
  // Game is running.  
  // Check to see if we should send new saved data to the Platform Service
  else if (gameState.GetRunState() == RunState::PLAYING)
  {
    // See if we need to Save new data, but don't save more than once a minute.
    // There is no technical limitation to save more than once a minute - this is just a guard
    // against pathological random greatness where you quickly achieve Billions!!! of new high scores.
    auto secondsSinceLastSave = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - LastSaveTime);
    if ((gameState.DoesGlobalNeedSaving() || gameState.DoesLocalNeedSaving()) && secondsSinceLastSave.count() > 60)
    {
      SaveGameStateToCloudStorage(gameState);
      gameState.ClearNeedsSaving();
      LastSaveTime = std::chrono::steady_clock::now();
    }
  }
  // Resetting - delete the existing saves
  else if (gameState.GetRunState() == RunState::RESETTING)
  {
    switch (CurrentState)
    {
    case CloudStorageState::NORMAL_GAMEPLAY:
    {
      gameState.SetGlobalHighScore(std::numeric_limits<int64_t>::min());
      gameState.SetLocalHighScore(std::numeric_limits<int64_t>::min());
      LocalHighScores.clear();

      ovrRequest req = ovr_CloudStorage_Delete(LATEST_HIGHSCORE_BUCKET.c_str(), KEY.c_str());
      SaveRequests[req] = std::unique_ptr<uint8_t>();
      req = ovr_CloudStorage_Delete(HIGHSCORE_BUCKET.c_str(), KEY.c_str());
      SaveRequests[req] = std::unique_ptr<uint8_t>();
      req = ovr_CloudStorage_Delete(LOCAL_HIGHSCORES_BUCKET.c_str(), KEY.c_str());
      SaveRequests[req] = std::unique_ptr<uint8_t>();

      CurrentState = CloudStorageState::DELETING_SAVES;
      break;
    }
    case CloudStorageState::DELETING_SAVES:
      if (SaveRequests.empty())
      {
        CurrentState = CloudStorageState::NORMAL_GAMEPLAY;
        gameState.SetRunState(RunState::PLAYING);
      }
      break;

    default:
      gameState.SetRunState(RunState::QUIT);
      std::cout << "Unhandled CloudStorageManager state: " << (int)CurrentState << std::endl;
    }
  }
  // Quiting - make sure the data is saved
  else if (gameState.GetRunState() == RunState::QUITTING)
  {
    if (gameState.DoesGlobalNeedSaving())
    {
      SaveGameStateToCloudStorage(gameState);
      gameState.ClearNeedsSaving();
      gameState.SetRunState(RunState::QUITTING_WAITING_FOR_SAVE);
    }
    else
    {
      gameState.SetRunState(RunState::QUIT);
    }
  }
}

void CloudStorageManager::ProcessMessageLoadMetadata(ovrMessageHandle message, GameState& gameState)
{
  ovrCloudStorageMetadataArrayHandle response = ovr_Message_GetCloudStorageMetadataArray(message);

  for (size_t i = 0; i < ovr_CloudStorageMetadataArray_GetSize(response); i++)
  {
    ovrCloudStorageMetadataHandle metadatum = ovr_CloudStorageMetadataArray_GetElement(response, i);
    const char *bucket = ovr_CloudStorageMetadata_GetBucket(metadatum);
    const char *key = ovr_CloudStorageMetadata_GetKey(metadatum);
    ovrCloudStorageVersionHandle handle = ovr_CloudStorageMetadata_GetVersionHandle(metadatum);
    const char *extraData = ovr_CloudStorageMetadata_GetExtraData(metadatum);
    int64_t counter = ovr_CloudStorageMetadata_GetCounter(metadatum);
    size_t dataSize = ovr_CloudStorageMetadata_GetDataSize(metadatum);
    ovrCloudStorageDataStatus status = ovr_CloudStorageMetadata_GetStatus(metadatum);

    // For this example I'm only interested in one save per bucket.  
    // I'll just ignore the keys that don't match the one I'm looking for since 
    // they likely came from playing around with this code.
    if (KEY != key)
      continue;

    switch (status) {
    case ovrCloudStorageDataStatus_InSync:
      // in general we ignore when we know we're out of date with server
      // because knowning is the same as not knowing -
      // we'll deal with it on app stop or next start.
    case ovrCloudStorageDataStatus_NeedsUpload:
    case ovrCloudStorageDataStatus_LocalUploading:
    case ovrCloudStorageDataStatus_NeedsDownload:
    case ovrCloudStorageDataStatus_RemoteDownloading:
      break;

    case ovrCloudStorageDataStatus_InConflict:
      // being in conflict is expected for the MANUAL resolution bucket.
      // we'll deal with it after the metadata has loaded
      if (LOCAL_HIGHSCORES_BUCKET != bucket)
      {
        std::cout << "Only the Bucket named " << LOCAL_HIGHSCORES_BUCKET << " can have in-conflict data." << std::endl;
        std::cout << "Make sure the buckets are properly configured according to the Readme." << std::endl;
        gameState.SetRunState(RunState::QUIT);
        return;
      }
      break;

      // shouldn't happen, a safeguard against deserialization errors
    case ovrCloudStorageDataStatus_Unknown:
    default:
      std::cout << "Unexpected Cloud Storage Status " << status << std::endl;
      std::cout << "Bucket: " << bucket << " Key: " << key << std::endl << std::flush;
      gameState.SetRunState(RunState::QUIT);
      return;
    }

    // Store the metadata for later use and send the load-data request
    if (LATEST_HIGHSCORE_BUCKET == bucket)
    {
      LatestHighscoreSave.emplace(key, CloudStorageEntry(extraData, counter, status));
      ovr_CloudStorage_Load(bucket, key);
    }
    else if (HIGHSCORE_BUCKET == bucket)
    {
      HighscoreSave.emplace(key, CloudStorageEntry(extraData, counter, status));
      ovr_CloudStorage_Load(bucket, key);
    }
    else if (LOCAL_HIGHSCORES_BUCKET == bucket)
    {
      LocalHighscoresSave.emplace(key, CloudStorageEntry(extraData, counter, status));
      if (status == ovrCloudStorageDataStatus_InConflict)
      {
        ovr_CloudStorage_LoadConflictMetadata(bucket, key);
      }
      else
      {
        ovr_CloudStorage_Load(bucket, key);
      }
    }
  }

  // If the list is too long, the request could get paged and we'll need to request more results.
  // That should never happen in this example, but I wouldn't want you to copy this method and forget
  // to add it in your game!
  if (ovr_CloudStorageMetadataArray_HasNextPage(response))
  {
    ovr_CloudStorage_GetNextCloudStorageMetadataArrayPage(response);
  }
  else
  {
    LoadedBuckets += 1;
  }
}

void CloudStorageManager::ProcessMessageLoad(ovrMessageHandle message, GameState& gameState)
{
  ovrCloudStorageDataHandle response = ovr_Message_GetCloudStorageData(message);

  std::string bucket(ovr_CloudStorageData_GetBucket(response));
  assert(KEY == ovr_CloudStorageData_GetKey(response));
  const void* data = ovr_CloudStorageData_GetData(response);
  assert(data != nullptr);
  auto dataSize = ovr_CloudStorageData_GetDataSize(response);

  if (LOCAL_HIGHSCORES_BUCKET == bucket)
  {
    const char *c_str = static_cast<const char *>(data);
    assert(c_str[dataSize-1] == '\0');
    LocalHighScores = CloudStorageEntry::DeserializeScores(c_str);
    if (LocalHighScores.find(ComputerName) != LocalHighScores.end())
    {
      gameState.SetLocalHighScore(LocalHighScores[ComputerName]);
    }
    LocalHighscoresSave[KEY].DataLoaded = true;
  }
  else if (HIGHSCORE_BUCKET == bucket)
  {
    assert(dataSize == sizeof(int64_t));
    auto score = CloudStorageEntry::DeserializeScore(data);
    assert(score == HighscoreSave[KEY].Counter);
    gameState.SetGlobalHighScore(score);
    HighscoreSave[KEY].DataLoaded = true;
  }
  else if (LATEST_HIGHSCORE_BUCKET == bucket)
  {
    assert(dataSize == sizeof(int64_t));
    auto score = CloudStorageEntry::DeserializeScore(data);
    assert(score == LatestHighscoreSave[KEY].Counter);
    LatestHighscoreSave[KEY].DataLoaded = true;
  }
}

void CloudStorageManager::ProcessMessageLoadConflictMetadata(ovrMessageHandle message)
{
  ovrCloudStorageConflictMetadataHandle response = ovr_Message_GetCloudStorageConflictMetadata(message);

  // load the metadata for the local and remote versions
  ovrCloudStorageMetadataHandle local = ovr_CloudStorageConflictMetadata_GetLocal(response);
  ovrCloudStorageMetadataHandle remote = ovr_CloudStorageConflictMetadata_GetRemote(response);

  assert(LOCAL_HIGHSCORES_BUCKET == ovr_CloudStorageMetadata_GetBucket(local));
  assert(LOCAL_HIGHSCORES_BUCKET == ovr_CloudStorageMetadata_GetBucket(remote));
  assert(KEY == ovr_CloudStorageMetadata_GetKey(local));
  assert(KEY == ovr_CloudStorageMetadata_GetKey(remote));

  // if the metadata contained enough information to decide how to resolve the conflict, we
  // could immediately do that now.  However, for this game we need to load both versions of
  // of the data to resolve the conflict.
  LocalHighscoresSave[KEY].RemoteHandle = ovr_CloudStorageMetadata_GetVersionHandle(remote);
  LocalHighscoresSave[KEY].LocalDataRequest = ovr_CloudStorage_LoadHandle(ovr_CloudStorageMetadata_GetVersionHandle(local));
  LocalHighscoresSave[KEY].RemoteDataRequest = ovr_CloudStorage_LoadHandle(ovr_CloudStorageMetadata_GetVersionHandle(remote));
}

void CloudStorageManager::ProcessMessageLoadHandle(ovrMessageHandle message, GameState& gameState)
{
  ovrRequest requestId = ovr_Message_GetRequestID(message);
  ovrCloudStorageDataHandle response = ovr_Message_GetCloudStorageData(message);

  const void *tmpdata = ovr_CloudStorageData_GetData(response);
  auto dataSize = ovr_CloudStorageData_GetDataSize(response);

  if (requestId == LocalHighscoresSave[KEY].LocalDataRequest)
  {
    LocalHighscoresSave[KEY].LocalData.resize(dataSize);
    memcpy(LocalHighscoresSave[KEY].LocalData.data(), tmpdata, dataSize);
    LocalHighscoresSave[KEY].LocalDataRequest = 0;
  }
  else
  {
    LocalHighscoresSave[KEY].RemoteData.resize(dataSize);
    memcpy(LocalHighscoresSave[KEY].RemoteData.data(), tmpdata, dataSize);
    LocalHighscoresSave[KEY].RemoteDataRequest = 0;
  }

  // Once we have both the local and remote data loaded, we can resolve the conflict
  if (!LocalHighscoresSave[KEY].LocalDataRequest && !LocalHighscoresSave[KEY].RemoteDataRequest)
  {
    LocalHighScores = MergeConflictingSaves(LocalHighscoresSave[KEY].LocalData, LocalHighscoresSave[KEY].RemoteData);
    if (LocalHighScores.find(ComputerName) != LocalHighScores.end())
    {
      gameState.SetLocalHighScore(LocalHighScores[ComputerName]);
    }

    // since we merged the data, we need to save a new local copy
    auto data = CloudStorageEntry::SerializeScores(LocalHighScores);
    ovrRequest req = ovr_CloudStorage_Save(LOCAL_HIGHSCORES_BUCKET.c_str(), KEY.c_str(), data.first.get(), data.second, 0, "foo");
    SaveRequests[req] = std::move(data.first);

    // mark the conflict resolved by choosing the local copy we just saved
    ovr_CloudStorage_ResolveKeepLocal(LOCAL_HIGHSCORES_BUCKET.c_str(), KEY.c_str(), LocalHighscoresSave[KEY].RemoteHandle.c_str());
  }
}

// merges two save files by preserving the highest score from each machine
std::map<std::string, int64_t> CloudStorageManager::MergeConflictingSaves(const std::vector<char>& local, const std::vector<char>& remote)
{
  std::map<std::string, int64_t> localHighScores;

  for (const auto& score : CloudStorageEntry::DeserializeScores(local.data()))
  {
    localHighScores[score.first] = score.second;
  }
  for (const auto& score : CloudStorageEntry::DeserializeScores(remote.data()))
  {
    if (localHighScores.find(score.first) == localHighScores.end() || localHighScores[score.first] < score.second)
    {
      localHighScores[score.first] = score.second;
    }
  }

  return localHighScores;
}

// in the LATEST_TIMESTAMP and HIGHEST_COUNTER buckets, the actual save data we'll store is
// the 8 bytes to store the long integer.  For the MANUAL merge bucket we'll store the highest
// score per machine.  This gives is us an excuse to demonstrate merging conflicting saves
// from different machines.
void CloudStorageManager::SaveGameStateToCloudStorage(const GameState& gameState)
{
  auto globalHighScore = gameState.GetGlobalHighScore();
  auto localHighScore = gameState.GetLocalHighScore();

  {
    // The LATEST_TIMESTAMP bucket preserves the save with the latest save timestamp.
    // This will almost always also be the save with the highest score.  For it to not be
    // the highest score, something like this has to happen:  
    //  At the epoch of this example the high score is 100.
    //  Computer A gets a 100,000 score, but looses its internet connection before the data is uploaded.
    //  Computer B later gets 1000 score and it's saved to the cloud.
    //  Computer A reconnects to the cloud, and its 100,000 score is rejected because it has an earlier timestamp.
    LatestHighscoreSave[KEY] = CloudStorageEntry(nullptr, globalHighScore, ovrCloudStorageDataStatus_Unknown);
    auto data = CloudStorageEntry::SerializeScore(globalHighScore);

    ovrRequest req = ovr_CloudStorage_Save(LATEST_HIGHSCORE_BUCKET.c_str(), KEY.c_str(), data.first.get(), data.second, globalHighScore, "foo");

    // protect the save data until we get the message that the Save call was processed
    SaveRequests[req] = std::move(data.first);
  }

  {
    // The HIGHEST_COUNTER bucket will always preserve the highest score since we store
    // the score in the metadata parameter as well
    HighscoreSave[KEY] = CloudStorageEntry(nullptr, globalHighScore, ovrCloudStorageDataStatus_Unknown);
    auto data = CloudStorageEntry::SerializeScore(globalHighScore);

    ovrRequest req = ovr_CloudStorage_Save(HIGHSCORE_BUCKET.c_str(), KEY.c_str(), data.first.get(), data.second, globalHighScore, "foo");

    // protect the save data until we get the message that the Save call was processed
    SaveRequests[req] = std::move(data.first);
  }

  {
    // For the MANUAL bucket we'll save a string where each line holds the high score on a particular machine
    // Serialize all high scores to one save buffer.
    // Even if we have the wrong high score for another machine, we're fine because it will get
    // corrected during conflict resolution the next time the save is loaded.
    LocalHighScores[ComputerName] = localHighScore;
    LocalHighscoresSave[KEY] = CloudStorageEntry(nullptr, 0, ovrCloudStorageDataStatus_Unknown);
    auto data = CloudStorageEntry::SerializeScores(LocalHighScores);

    ovrRequest req = ovr_CloudStorage_Save(LOCAL_HIGHSCORES_BUCKET.c_str(), KEY.c_str(), data.first.get(), data.second, globalHighScore, "foo");

    // protect the save data until we get the message that the Save call was processed
    SaveRequests[req] = std::move(data.first);
  }
}

void CloudStorageManager::ProcessMessageResolveKeepLocal(ovrMessageHandle message)
{
  ovrCloudStorageUpdateResponseHandle response = ovr_Message_GetCloudStorageUpdateResponse(message);

  assert(LOCAL_HIGHSCORES_BUCKET == ovr_CloudStorageUpdateResponse_GetBucket(response));
  assert(KEY == ovr_CloudStorageUpdateResponse_GetKey(response));
  assert(ovrCloudStorageUpdateStatus_Ok == ovr_CloudStorageUpdateResponse_GetStatus(response));
}

void CloudStorageManager::ProcessMessageSave(ovrMessageHandle message, GameState& gameState)
{
  // delete the saved data buffer
  auto requestId = ovr_Message_GetRequestID(message);
  auto it = SaveRequests.find(requestId);
  if (it != SaveRequests.end())
  {
    (*it).second.reset(nullptr);
    SaveRequests.erase(it);
  }
  else
  {
    std::cout << "Unexpectedly failed to match Save request: " << requestId << std::endl;
  }

  // if we are trying to quit, and all the Save Requests have been handled, we can transition to quit
  if (gameState.GetRunState() == RunState::QUITTING_WAITING_FOR_SAVE && SaveRequests.empty())
  {
    gameState.SetRunState(RunState::QUIT);
  }
}

void CloudStorageManager::ProcessMessageDelete(ovrMessageHandle message)
{
  auto requestId = ovr_Message_GetRequestID(message);
  auto it = SaveRequests.find(requestId);
  if (it != SaveRequests.end())
  {
    SaveRequests.erase(it);
  }
}

std::string CloudStorageManager::GetLocalComputerName() const 
{
#ifdef WIN32
  char buffer[MAX_COMPUTERNAME_LENGTH];
  DWORD len = MAX_COMPUTERNAME_LENGTH;

  if (!GetComputerNameA(buffer, &len)) 
  {
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    std::cout << "GetComputerNameA Error: " << message << std::endl;

    return "Mysterious unknown computer";
  }
  else 
  {
    return buffer;
  }

#else
#error GetLocalComputerName not implement yet for GearVR
#endif
}
