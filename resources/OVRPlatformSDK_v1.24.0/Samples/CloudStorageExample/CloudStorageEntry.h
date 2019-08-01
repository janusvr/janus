#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <OVR_Platform.h>

class CloudStorageEntry
{
public:
  CloudStorageEntry(const char *extraData, int64_t counter, ovrCloudStorageDataStatus status);
  CloudStorageEntry() = default;
  ~CloudStorageEntry() = default;
  CloudStorageEntry(const CloudStorageEntry&) = default;
  CloudStorageEntry(CloudStorageEntry&&) = default;
  CloudStorageEntry &CloudStorageEntry::operator =(const CloudStorageEntry &) = default;

  bool HasDataLoaded() const { return DataLoaded; }

  static std::pair<std::unique_ptr<uint8_t>, uint32_t> SerializeScore(int64_t score);
  static std::pair<std::unique_ptr<uint8_t>, uint32_t> SerializeScores(std::map<std::string, int64_t>& scores);
  static int64_t DeserializeScore(const void* scores);
  static std::map<std::string, int64_t> DeserializeScores(const char* scores);

private:
  std::string ExtraData;
  int64_t Counter;
  ovrCloudStorageDataStatus Status;
  bool DataLoaded;

  // for conflict resolution, the request to get a copy of the local data
  ovrRequest LocalDataRequest;
  // for conflict resolution, the request to get a copy of the remote data
  ovrRequest RemoteDataRequest;
  // for conflict resolution, the copy of the local data
  std::vector<char> LocalData;
  // for conflict resolution, the copy of the remote data
  std::vector<char> RemoteData;
  // for conflict resolution, the handle to the remote data we resolved against
  std::string RemoteHandle;

  friend class CloudStorageManager;
};
