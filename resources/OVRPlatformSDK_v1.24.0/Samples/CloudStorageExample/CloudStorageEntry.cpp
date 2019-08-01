#include "CloudStorageEntry.h"

#include <iostream>
#include <sstream>
#include <string>

CloudStorageEntry::CloudStorageEntry(const char *extraData, int64_t counter, ovrCloudStorageDataStatus status)
  : ExtraData(extraData != nullptr ? extraData : "")
  , Counter(counter)
  , Status(status)
  , DataLoaded(false)
{
}

std::pair<std::unique_ptr<uint8_t>, uint32_t> CloudStorageEntry::SerializeScore(int64_t score)
{
  uint32_t size = sizeof(score);
  std::unique_ptr<uint8_t> data(new uint8_t[size]);
  memcpy(data.get(), &score, size);

  return std::make_pair(std::move(data), size);
}

int64_t CloudStorageEntry::DeserializeScore(const void* scores)
{
  return static_cast<const uint64_t*>(scores)[0];
}

std::pair<std::unique_ptr<uint8_t>, uint32_t> CloudStorageEntry::SerializeScores(std::map<std::string, int64_t> &scores)
{
  std::stringstream ss;
  for (const auto& score : scores)
  {
    ss << score.second << "/" << score.first << "\n";
  }
  const std::string data_str(ss.str());

  uint32_t size = (uint32_t)data_str.size() + 1;
  std::unique_ptr<uint8_t> data(new uint8_t[size]);
  memcpy(data.get(), data_str.c_str(), size);

  return std::make_pair(std::move(data), size);
}

std::map<std::string, int64_t> CloudStorageEntry::DeserializeScores(const char *scoreLines)
{
  std::map<std::string, int64_t> scores;

  if (scoreLines != nullptr)
  {
    std::stringstream ss(scoreLines);
    std::string line;
    std::string::size_type splitPos;
    while (std::getline(ss, line, '\n')) {
      if ((splitPos = line.find("/")) != std::string::npos)
      {
        std::string hostname(line.substr(splitPos + 1));
        std::string score_str(line.substr(0, splitPos));
        int64_t score = std::stoll(score_str);
        scores[hostname] = score;
      }
      else
      {
        std::cout << "Error parsing line: " << line << std::endl;
      }
    }
  }

  return scores;
}
