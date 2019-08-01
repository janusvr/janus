#define BUFFER_SIZE 512
#define BACKSPACE_KEY 8
#define ENTER_KEY '\r'
#define CLOUD_BUCKET "TestBucket"
#define CLOUD_KEY "TestBlob"

class SampleGame
{
public:
	SampleGame() {};
	~SampleGame() {};

	int init(const char* appID);
	void gameLoop();

private:
	bool exitGame = false;
	char commandBuffer[BUFFER_SIZE];
	int commandIndex = 0;

	void checkKeyboard();
	void processCommand();
	void pumpOVRMessages();

	void outputCommands();

	// Command methods
	void createAndJoinPrivateRoom(ovrRoomJoinPolicy joinType, unsigned int maxUsers);
	void getCurrentRoom();
	void getRoom(ovrID roomID);
	void leaveRoom(ovrID roomID);
	void joinRoom(ovrID roomID);
	void kickUser(ovrID roomID, ovrID userID);
	void getInvitableUsers();
	void inviteUser(ovrID roomID, char* inviteToken);
	void setRoomDescription(ovrID roomID, char* description);

	void getLoggedInUser();
	void getUser(ovrID userID);
	void getLoggedInUserFriends();
	void checkEntitlement();
	void generateUserProof();
	void getUserToken();

	void updateRoomDataStore(ovrID roomID, char* key, char* value);

	void getAchievementDefinition(char* cheevoName);
	void getAchievementProgress(char* cheevoName);
	void unlockAchievement(char* cheevoName);
	void addAchievementCount(char* cheevoName, char* value);
	void addAchievementBitfield(char* cheevoName, char* value);

	void writeLeaderboardEntry(char* leaderboardName, char* value);
	void getLeaderboardEntries(char* leaderboardName);

  void writeCloudData();
  void getCloudData();
  void deleteCloudData();
  void getCloudMetaData();

	// Notification handlers
	void processCreateAndJoinPrivateRoom(ovrMessageHandle message);
	void processGetCurrentRoom(ovrMessageHandle message);
	void processGetRoom(ovrMessageHandle message);
	void processLeaveRoom(ovrMessageHandle message);
	void processJoinRoom(ovrMessageHandle message);
	void processKickUser(ovrMessageHandle message);
	void processGetInvitableUsers(ovrMessageHandle message);
	void processInviteUser(ovrMessageHandle message);
	void processSetRoomDescription(ovrMessageHandle message);

	void processGetLoggedInUser(ovrMessageHandle message);
	void processGetUser(ovrMessageHandle message);
	void processGetFriends(ovrMessageHandle message);

	void processUpdateRoomDataStore(ovrMessageHandle message);
	void processRoomUpdate(ovrMessageHandle message);
	void processCheckEntitlement(ovrMessageHandle message);
	void processGenerateUserProof(ovrMessageHandle message);
	void processGetUserToken(ovrMessageHandle message);

	void processGetAchievementDefinition(ovrMessageHandle message);
	void processGetAchievementProgress(ovrMessageHandle message);
	void processUnlockAchievement(ovrMessageHandle message);
	void processAddAchievementCount(ovrMessageHandle message);
	void processAddAchievementBitfield(ovrMessageHandle message);

	void processWriteLeaderboardEntry(ovrMessageHandle message);
	void processGetLeaderboardEntries(ovrMessageHandle message);

  void processCloudStorageLoad(ovrMessageHandle message);
  void processCloudStorageSave(ovrMessageHandle message);
  void processCloudStorageDelete(ovrMessageHandle message);
  void processCloudMetaData(ovrMessageHandle message);

	// Helper methods
	void outputRoomDetails(ovrRoomHandle room);
	void outputUserArray(ovrUserArrayHandle users);
};
