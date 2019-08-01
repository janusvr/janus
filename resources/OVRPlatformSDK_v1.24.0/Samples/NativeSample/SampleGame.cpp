#include "OVR_Platform.h"
#include "SampleGame.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

unsigned char dataBlob[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0xDE, 0xAD, 0xBE, 0xEF };

int SampleGame::init(const char* appID)
{
	if (ovr_PlatformInitializeWindows(appID) != ovrPlatformInitialize_Success)
	{
		return 1;
	}

	ovr_Entitlement_GetIsViewerEntitled();

	printf("Press h for list of commands.\nCommand > ");

	return 0;
}

void SampleGame::gameLoop()
{
	while (!exitGame)
	{
		checkKeyboard();
		pumpOVRMessages();
	}
}

void SampleGame::checkKeyboard()
{

	if (_kbhit())
	{
		int key = _getch();

		switch (key){
		case BACKSPACE_KEY:
			if (commandIndex > 0)
			{
				commandIndex--;
				commandBuffer[commandIndex] = '\0';
				printf("%c %c", key, key);
			}
			break;
		case ENTER_KEY:
			commandBuffer[commandIndex] = '\0';
			printf("\n");
			processCommand();
			break;

		default:
			if (commandIndex < BUFFER_SIZE)
			{
				commandBuffer[commandIndex] = key;
				printf("%c", key);
				commandIndex++;
			}
			break;
		}
	}
}

void SampleGame::processCommand()
{
	char *command = NULL;
	char *param1 = NULL;
	char *param2 = NULL;
	char *param3 = NULL;

	ovrID param1ID = 0;
	ovrID param2ID = 0;
	ovrID param3ID = 0;

	char *nextToken = NULL;
	char seps[] = " ";

	// Grab the command parameters
	command = strtok_s(commandBuffer, seps, &nextToken);
	param1 = strtok_s(NULL, seps, &nextToken);
	if (param1)
	{
		param1ID = _strtoui64(param1, NULL, 10);
	}
	param2 = strtok_s(NULL, seps, &nextToken);
	if (param2)
	{
		param2ID = _strtoui64(param2, NULL, 10);
	}
	param3 = strtok_s(NULL, seps, &nextToken);
	if (param3)
	{
		param3ID = _strtoui64(param3, NULL, 10);
	}

	if (command) {
		switch (command[0])
		{
		case 'h':
			outputCommands();
			break;
		case 'p':
			if (param1&&param2) {
				createAndJoinPrivateRoom((ovrRoomJoinPolicy)atoi(param1), atoi(param2));
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case 'c':
			getCurrentRoom();
			break;
		case 'g':
			if (param1) {
				getRoom(param1ID);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case 'j':
			if (param1) {
				joinRoom(param1ID);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case 'k':
			if (param1&&param2) {
				kickUser(param1ID, param2ID);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case 'l':
			if (param1) {
				leaveRoom(param1ID);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case 'm':
			getLoggedInUser();
			break;
		case 'u':
			if (param1){
				getUser(param1ID);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case 'd':
			getLoggedInUserFriends();
			break;
		case 'i':
			getInvitableUsers();
			break;
		case 'o':
			if (param1&&param2) {
				inviteUser(param1ID, param2);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case 's':
			if (param1&&param2) {
				setRoomDescription(param1ID, param2);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case 'w':
			if (param1&&param2&&param3) {
				updateRoomDataStore(param1ID, param2, param3);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case 'e':
			checkEntitlement();
			break;
		case 'n':
			generateUserProof();
			break;
		case 't':
			getUserToken();
			break;
		case 'a':
			if (param1){
				getAchievementDefinition(param1);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case 'b':
			if (param1){
				getAchievementProgress(param1);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case '3':
			if (param1){
				unlockAchievement(param1);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case '4':
			if (param1&&param2) {
				addAchievementCount(param1,param2);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case '5':
			if (param1&&param2) {
				addAchievementBitfield(param1, param2);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case '1':
			if (param1&&param2) {
				writeLeaderboardEntry(param1,param2);
			}
			else {
				printf("Missing Params\n");
			}
			break;
		case '2':
			if (param1){
				getLeaderboardEntries(param1);
			}
			else {
				printf("Missing Params\n");
			}
			break;
    case '6':
      writeCloudData();
      break;
    case '7':
      getCloudData();
      break;
    case '8':
      deleteCloudData();
      break;
    case '9':
      getCloudMetaData();
      break;
    case 'q':
			exitGame = true;
			break;
		default:
			printf("Invalid Command\n");
			break;
		}

		memset(commandBuffer, 0, sizeof(char)*BUFFER_SIZE);
		commandIndex = 0;
	}

	printf("Command > ");
}

void SampleGame::pumpOVRMessages()
{
	ovrMessageHandle message = nullptr;

	while ((message = ovr_PopMessage()) != nullptr) {
		switch (ovr_Message_GetType(message)) {
		case ovrMessage_Room_CreateAndJoinPrivate:
			processCreateAndJoinPrivateRoom(message);
			break;
		case ovrMessage_Room_GetCurrent:
			processGetCurrentRoom(message);
			break;
		case ovrMessage_Room_Get:
			processGetRoom(message);
			break;
		case ovrMessage_Room_Leave:
			processLeaveRoom(message);
			break;
		case ovrMessage_Room_Join:
			processJoinRoom(message);
			break;
		case ovrMessage_Room_KickUser:
			processKickUser(message);
			break;
		case ovrMessage_User_GetLoggedInUser:
			processGetLoggedInUser(message);
			break;
		case ovrMessage_User_Get:
			processGetUser(message);
			break;
		case ovrMessage_User_GetLoggedInUserFriends:
			processGetFriends(message);
			break;
		case ovrMessage_Room_GetInvitableUsers:
			processGetInvitableUsers(message);
			break;
		case ovrMessage_Room_InviteUser:
			processInviteUser(message);
			break;
		case ovrMessage_Room_SetDescription:
			processSetRoomDescription(message);
			break;
		case ovrMessage_Room_UpdateDataStore:
			processUpdateRoomDataStore(message);
			break;
		case ovrMessage_Notification_Room_RoomUpdate:
			processRoomUpdate(message);
			break;
		case ovrMessage_User_GetUserProof:
			processGenerateUserProof(message);
			break;
		case ovrMessage_User_GetAccessToken:
			processGetUserToken(message);
			break;
		case ovrMessage_Achievements_GetDefinitionsByName:
			processGetAchievementDefinition(message);
			break;
		case ovrMessage_Achievements_GetProgressByName:
			processGetAchievementProgress(message);
			break;
		case ovrMessage_Achievements_Unlock:
			processUnlockAchievement(message);
			break;
		case ovrMessage_Achievements_AddCount:
			processAddAchievementCount(message);
			break;
		case ovrMessage_Achievements_AddFields:
			processAddAchievementBitfield(message);
			break;
		case ovrMessage_Leaderboard_WriteEntry:
			processWriteLeaderboardEntry(message);
			break;
		case ovrMessage_Leaderboard_GetEntries:
			processGetLeaderboardEntries(message);
			break;
		case ovrMessage_Entitlement_GetIsViewerEntitled:
			processCheckEntitlement(message);
			break;
    case ovrMessage_CloudStorage_Load:
      processCloudStorageLoad(message);
      break;
    case ovrMessage_CloudStorage_Save:
      processCloudStorageSave(message);
      break;
    case ovrMessage_CloudStorage_Delete:
      processCloudStorageDelete(message);
      break;
    case ovrMessage_CloudStorage_LoadBucketMetadata:
      processCloudMetaData(message);
      break;
		default:
			fprintf(stderr, "unknown message %d", ovr_Message_GetType(message));
		}
		printf("\nCommand > %s",commandBuffer);
		ovr_FreeMessage(message);
	}

}

void SampleGame::outputCommands()
{
	printf("\nList of Commands\n----------------\n"
    "h - list commands\np - Create and Join Private Room <join Policy> <Max Users> (ovr_Room_CreateAndJoinPrivate)\n"
    "c - Get Current room (ovr_Room_GetCurrent)\n"
    "g <roomid> - get room (ovr_Room_Get)\n"
    "j <roomid> - Join room (ovr_Room_Join)\n"
    "l <roomid> - Leave room (ovr_Room_Leave)\n"
    "k <roomid> <userid> - kick user from room (ovr_Room_KickUser)\n"
    "m - Get logged in user (ovr_User_GetLoggedInUser)\n"
    "u <userid> - Get user (ovr_User_Get)\n"
    "d - Get logged in users friends (ovr_User_GetLoggedInUserFriends)\n"
    "i - Get invitable users (ovr_Room_GetInvitableUsers)\n"
    "o <roomid> <invitetoken> - Invite user (ovr_Room_InviteUser)\n"
    "s <roomid> <description> - Set room description\n"
    "w <roomid> <key> <value> - Set room data\n"
    "e - check if current user is entitled for this appid\n"
    "n - generate a user nonce for the current user\n"
    "t - generate a user token for the current user\n"
    "a <achievement name> - get achivement definition\n"
    "b <achievement name> - get achievement progress\n"
    "3 <achievement name> - unlock achievement\n"
    "4 <achievement name> <count> add count to achievement\n"
    "5 <achievement name> <fields> add fields to achievement\n"
    "1 <leaderboard name> <value> - write leaderboard entry\n"
    "2 <leaderboard name> - get leaderboard entries\n"
    "6 - save test cloud data\n"
    "7 - load test cloud data\n"
    "8 - delete test cloud data\n"
    "9 - Enumerate cloud bucket metadata\n"
    "q - quit\n\n");
}

void SampleGame::writeCloudData()
{
  printf("\nSaving cloud data.\n");

  ovrRequest req;

  req = ovr_CloudStorage_Save(CLOUD_BUCKET, CLOUD_KEY, &dataBlob, sizeof(dataBlob), 0, "Test Data");

  // IMPORTANT NOTE:
  // ovr_CloudStorage_Save is only passing in a pointer to your data in an async call.  Your data will need to stay
  // alive and unmodified until you receive a ovrMessage_CloudStorage_Save message indicating that the save was successful.  If the
  // data is destroyed or modified prior to receiving that message there's a good chance the proper data won't be saved.
}
void SampleGame::processCloudStorageSave(ovrMessageHandle message)
{
  if (!ovr_Message_IsError(message))
  {
    printf("Cloud data was saved!\n");
  }
  else
  {
    printf("Could NOT save cloud data\n");
    ovrErrorHandle error = ovr_Message_GetError(message);
    printf("Error Code: %d\n", ovr_Error_GetCode(error));
    printf("Error Message: %s\n", ovr_Error_GetMessage(error));
  }
}

void SampleGame::getCloudData()
{
  printf("\nLoading cloud data.\n");

  ovrRequest req;

  req = ovr_CloudStorage_Load(CLOUD_BUCKET, CLOUD_KEY);
}

void SampleGame::processCloudStorageLoad(ovrMessageHandle message)
{
  if (!ovr_Message_IsError(message))
  {
    printf("Cloud data was loaded!\n");
    ovrCloudStorageDataHandle response = ovr_Message_GetCloudStorageData(message);
    const void* data = ovr_CloudStorageData_GetData(response);
    unsigned int data_size = ovr_CloudStorageData_GetDataSize(response);
  }
  else
  {
    printf("Could NOT load cloud data\n");
    ovrErrorHandle error = ovr_Message_GetError(message);
    printf("Error Code: %d\n", ovr_Error_GetCode(error));
    printf("Error Message: %s\n", ovr_Error_GetMessage(error));
  }
}

void SampleGame::deleteCloudData()
{
  printf("\nDeleting cloud data.\n");

  ovrRequest req;

  req = ovr_CloudStorage_Delete(CLOUD_BUCKET, CLOUD_KEY);
}

void SampleGame::processCloudStorageDelete(ovrMessageHandle message)
{
  if (!ovr_Message_IsError(message))
  {
    printf("Cloud data was deleted!\n");
  }
  else
  {
    ovrErrorHandle error = ovr_Message_GetError(message);
    printf("Received cloud delete failure: %s\n", ovr_Error_GetMessage(error));
  }
}

void SampleGame::getCloudMetaData()
{
  printf("\nLoading Cloud metadata\n");

  ovrRequest req;
  req = ovr_CloudStorage_LoadBucketMetadata(CLOUD_BUCKET);
}

void SampleGame::processCloudMetaData(ovrMessageHandle message)
{
  if (!ovr_Message_IsError(message))
  {
    printf("Received the following cloud metadata:\n");

    ovrCloudStorageMetadataArrayHandle metaDataArray = ovr_Message_GetCloudStorageMetadataArray(message);

    size_t numElements = ovr_CloudStorageMetadataArray_GetSize(metaDataArray);

    for (size_t x = 0; x < numElements; x++)
    {
      ovrCloudStorageMetadataHandle metaDataHandle =  ovr_CloudStorageMetadataArray_GetElement(metaDataArray, x);

      unsigned int data_size = ovr_CloudStorageMetadata_GetDataSize(metaDataHandle);
      unsigned long long saved_time = ovr_CloudStorageMetadata_GetSaveTime(metaDataHandle);
      ovrCloudStorageDataStatus status = ovr_CloudStorageMetadata_GetStatus(metaDataHandle);
      const char *extraData = ovr_CloudStorageMetadata_GetExtraData(metaDataHandle);
      int64_t counter = ovr_CloudStorageMetadata_GetCounter(metaDataHandle);

      printf("%u %I64u %s %lld ", data_size, saved_time, extraData, counter);
      switch (status)
      {
      case ovrCloudStorageDataStatus_InSync:
        printf("ovrCloudStorageDataStatus_InSync\n");
        break;
      case ovrCloudStorageDataStatus_NeedsDownload:
        printf("ovrCloudStorageDataStatus_NeedsDownload\n");
        break;
      case ovrCloudStorageDataStatus_RemoteDownloading:
        printf("ovrCloudStorageDataStatus_RemoteDownloading\n");
        break;
      case ovrCloudStorageDataStatus_NeedsUpload:
        printf("ovrCloudStorageDataStatus_NeedsUpload\n");
        break;
      case ovrCloudStorageDataStatus_LocalUploading:
        printf("ovrCloudStorageDataStatus_LocalUploading\n");
        break;
      case ovrCloudStorageDataStatus_InConflict:
        printf("ovrCloudStorageDataStatus_InConflict\n");
        break;
      case ovrCloudStorageDataStatus_Unknown:
      default:
        printf("ovrCloudStorageDataStatus_Unknown\n");
        break;
      }
    }
  }
  else
  {
    printf("Could NOT get cloud metadata\n");
  }

}


void SampleGame::checkEntitlement()
{
	printf("\nChecking for an entitlement\n");

	ovrRequest req;

	req = ovr_Entitlement_GetIsViewerEntitled();
}

void SampleGame::processCheckEntitlement(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message))
	{
		printf("User has an entitlement\n");
	}
	else
	{
		printf("Could NOT get an entitlement\n");
		exitGame = true;
	}
}


void SampleGame::createAndJoinPrivateRoom(ovrRoomJoinPolicy joinType, unsigned int maxUsers)
{
	printf("\nTrying to get create and join private room\n");

	ovrRequest req;

	req = ovr_Room_CreateAndJoinPrivate(joinType, maxUsers, true);
}

void SampleGame::processCreateAndJoinPrivateRoom(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message))
	{
		printf("Created and joined private room\n");

		ovrRoomHandle room = ovr_Message_GetRoom(message);
		outputRoomDetails(room);
	}
	else
	{
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Error creating private room: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::getCurrentRoom()
{
	printf("\nTrying to get current room\n");

	ovrRequest req;

	req = ovr_Room_GetCurrent();
}

void SampleGame::processGetCurrentRoom(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received get current room success\n");

		ovrRoomHandle room = ovr_Message_GetRoom(message);
		outputRoomDetails(room);
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received get room failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::getRoom(ovrID roomID)
{
	printf("\nTrying to get room %llu\n", roomID);

	ovrRequest req;

	req = ovr_Room_Get(roomID);
}

void SampleGame::processGetRoom(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received get room success\n");

		ovrRoomHandle room = ovr_Message_GetRoom(message);
		outputRoomDetails(room);
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received get room failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::leaveRoom(ovrID roomID)
{
	printf("\nTrying to leave room %llu\n", roomID);

	ovrRequest req;

	req = ovr_Room_Leave(roomID);
}

void SampleGame::processLeaveRoom(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received leave room success\n");

		ovrRoomHandle room = ovr_Message_GetRoom(message);
		outputRoomDetails(room);
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received leave room failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::joinRoom(ovrID roomID)
{
	printf("\nTrying to join room %llu\n", roomID);

	ovrRequest req;

	req = ovr_Room_Join(roomID, true);
}

void SampleGame::processJoinRoom(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received join room success\n");

		ovrRoomHandle room = ovr_Message_GetRoom(message);
		outputRoomDetails(room);
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received get room failure: %s\n", ovr_Error_GetMessage(error));
	}

}

void SampleGame::kickUser(ovrID roomID, ovrID userID)
{
	printf("\nTrying to get kick user %llu from room %llu\n", userID, roomID);

	ovrRequest req;

	req = ovr_Room_KickUser(roomID, userID, 10 /*kickDurationSeconds*/);
}

void SampleGame::processKickUser(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received kick user success\n");

		ovrRoomHandle room = ovr_Message_GetRoom(message);
		outputRoomDetails(room);
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received kick user failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::getInvitableUsers()
{
	printf("\nTrying to get invitable users\n");

	ovrRequest req;

	req = ovr_Room_GetInvitableUsers();
}

void SampleGame::processGetInvitableUsers(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received get invitable users success\n");

		ovrUserArrayHandle users = ovr_Message_GetUserArray(message);
		outputUserArray(users);
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received get invitable users failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::inviteUser(ovrID roomID, char* inviteToken)
{
	printf("\nTrying to get invite token %s to room %llu\n", inviteToken, roomID);

	ovrRequest req;

	req = ovr_Room_InviteUser(roomID, inviteToken);
}

void SampleGame::processInviteUser(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received invite user success\n");

		ovrRoomHandle room = ovr_Message_GetRoom(message);
		outputRoomDetails(room);
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received invite user failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::setRoomDescription(ovrID roomID, char* description)
{
	printf("\nTrying to set description of room %llu to %s\n", roomID, description);

	ovrRequest req;

	req = ovr_Room_SetDescription(roomID, description);
}

void SampleGame::processSetRoomDescription(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received set description success\n");

		ovrRoomHandle room = ovr_Message_GetRoom(message);
		outputRoomDetails(room);
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received set description failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::getLoggedInUser()
{
	printf("\nTrying to get logged in user\n");

	ovrRequest req;

	req = ovr_User_GetLoggedInUser();
}

void SampleGame::processGetLoggedInUser(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received get logged in user success\n");

		ovrUserHandle user = ovr_Message_GetUser(message);
		printf("user %llu %s\n", ovr_User_GetID(user), ovr_User_GetOculusID(user));
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received get user failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::getUser(ovrID userID)
{
	printf("\nTrying to get user %llu\n", userID);

	ovrRequest req;

	req = ovr_User_Get(userID);
}

void SampleGame::processGetUser(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received get user success\n");

		ovrUser* user = ovr_Message_GetUser(message);
		printf("user %s %s\n", ovr_User_GetID(user), ovr_User_GetOculusID(user));
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received get user failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::processGetFriends(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received get friends success\n");

		ovrUserArrayHandle users = ovr_Message_GetUserArray(message);
		outputUserArray(users);
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received get friends failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::getLoggedInUserFriends()
{
	printf("\nTrying to get friends for logged in user\n");

	ovrRequest req;

	req = ovr_User_GetLoggedInUserFriends();

}

void SampleGame::updateRoomDataStore(ovrID roomID, char* key, char* value)
{
	printf("\nTrying to update data store for room %llu\n", roomID);

	ovrRequest req;

	ovrKeyValuePair newKVPair;
	newKVPair.key = key;
	newKVPair.valueType = ovrKeyValuePairType_String;
	newKVPair.stringValue = value;

	req = ovr_Room_UpdateDataStore(roomID, &newKVPair, 1);
}

void SampleGame::processUpdateRoomDataStore(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received update data store success\n");

		ovrRoomHandle room = ovr_Message_GetRoom(message);
		outputRoomDetails(room);
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received update room data failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::processRoomUpdate(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received room update Notification\n");

		ovrRoomHandle room = ovr_Message_GetRoom(message);
		outputRoomDetails(room);
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received room update failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::outputRoomDetails(ovrRoomHandle room)
{
	printf("Room ID: %llu, App ID: %llu, Description: %s\n", ovr_Room_GetID(room), ovr_Room_GetApplicationID(room), ovr_Room_GetDescription(room));
	size_t maxUsers = ovr_Room_GetMaxUsers(room);
	printf("maxUsers: %d\nusers in room:\n", maxUsers);
	ovrUserHandle owner = ovr_Room_GetOwner(room);
	printf("Room owner: %llu %s\n", ovr_User_GetID(owner), ovr_User_GetOculusID(owner));
	printf("Join Policy: ");
	switch (ovr_Room_GetJoinPolicy(room))
	{
	case ovrRoom_JoinPolicyNone:
		printf("ovrRoom_JoinPolicyNone\n");
		break;
	case ovrRoom_JoinPolicyEveryone:
		printf("ovrRoom_JoinPolicyEveryone\n");
		break;
	case ovrRoom_JoinPolicyFriendsOfMembers:
		printf("ovrRoom_JoinPolicyFriendsOfMembers\n");
		break;
	case ovrRoom_JoinPolicyFriendsOfOwner:
		printf("ovrRoom_JoinPolicyFriendsOfOwner\n");
		break;
	case ovrRoom_JoinPolicyInvitedUsers:
		printf("ovrRoom_JoinPolicyInvitedUsers\n");
		break;
	default:
		printf("Unknown\n");
		break;
	}
	printf("Room Type: ");
	switch (ovr_Room_GetType(room))
	{
	case ovrRoom_TypeUnknown:
		printf("ovrRoom_TypeUnknown\n");
		break;
	case ovrRoom_TypeMatchmaking:
		printf("ovrRoom_TypeMatchmaking\n");
		break;
	case ovrRoom_TypeModerated:
		printf("ovrRoom_TypeModerated\n");
		break;
	case ovrRoom_TypePrivate:
		printf("ovrRoom_TypePrivate\n");
		break;
	case ovrRoom_TypeSolo:
		printf("ovrRoom_TypeSolo\n");
		break;
	default:
		printf("Unknown\n");
		break;
	}

	ovrDataStoreHandle dataStore = ovr_Room_GetDataStore(room);
	if (ovr_DataStore_GetValue(dataStore, "testkey"))
	{
		printf("Testkey value: %s\n",ovr_DataStore_GetValue(dataStore, "testkey"));
	}

	ovrUserArrayHandle users = ovr_Room_GetUsers(room);
	outputUserArray(users);
}

void SampleGame::outputUserArray(ovrUserArrayHandle users)
{
	size_t nUsers = ovr_UserArray_GetSize(users);
	for (size_t i = 0; i < nUsers; ++i) {
		ovrUserHandle user = ovr_UserArray_GetElement(users, i);
		printf("user %llu %s %s %s\n", ovr_User_GetID(user), ovr_User_GetOculusID(user), ovr_User_GetPresence(user), ovr_User_GetInviteToken(user));
	}
}

void SampleGame::generateUserProof()
{
	printf("\nTrying to generate a user nonce\n");

	ovrRequest req;

	req = ovr_User_GetUserProof();
}

void SampleGame::processGenerateUserProof(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		ovrUserProofHandle proof = ovr_Message_GetUserProof(message);
		printf("Received user nonce %s\n", ovr_UserProof_GetNonce(proof));
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received user nonce failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::getUserToken()
{
	printf("\nTrying to generate a user token\n");

	ovrRequest req;

	req = ovr_User_GetAccessToken();
}

void SampleGame::processGetUserToken(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received user token %s\n", ovr_Message_GetString(message));
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received user token failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::getAchievementDefinition(char* cheevoName)
{
	printf("\nTrying to get definition for achievement %s\n", cheevoName);

	ovrRequest req;

	req = ovr_Achievements_GetDefinitionsByName((const char**)&cheevoName, 1);
}

void SampleGame::processGetAchievementDefinition(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		ovrAchievementDefinitionArrayHandle cheevos =  ovr_Message_GetAchievementDefinitionArray(message);

		if (ovr_AchievementDefinitionArray_GetSize(cheevos) > 0) {
			ovrAchievementDefinitionHandle singleCheevo = ovr_AchievementDefinitionArray_GetElement(cheevos, 0);

			printf("Received get achievement definition for %s\n", ovr_AchievementDefinition_GetName(singleCheevo));

			switch (ovr_AchievementDefinition_GetType(singleCheevo))
			{
			case ovrAchievement_TypeSimple:
				printf("Type: Simple\n");
				break;

			case ovrAchievement_TypeBitfield:
				printf("Type: Bitfield\n");
				printf("Bitfield Length: %d\n", ovr_AchievementDefinition_GetBitfieldLength(singleCheevo));
				printf("Cheevo Target: %ld\n", ovr_AchievementDefinition_GetTarget(singleCheevo));
				break;

			case ovrAchievement_TypeCount:
				printf("Type: Count\n");
				printf("Cheevo Target: %ld\n", ovr_AchievementDefinition_GetTarget(singleCheevo));
				break;

			case ovrAchievement_TypeUnknown:
			default:
				printf("Type: Unknown\n");
				break;
			}
		}
		else {
			printf("Achievement not found\n");
		}
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received get achievement definition failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::getAchievementProgress(char* cheevoName)
{
	printf("\nTrying to get progress for achievement %s\n", cheevoName);

	ovrRequest req;

	req = ovr_Achievements_GetProgressByName((const char**)&cheevoName, 1);
}

void SampleGame::processGetAchievementProgress(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		ovrAchievementProgressArrayHandle cheevos =  ovr_Message_GetAchievementProgressArray(message);
		if (ovr_AchievementProgressArray_GetSize(cheevos) > 0) {

			ovrAchievementProgressHandle singleCheevo = ovr_AchievementProgressArray_GetElement(cheevos, 0);

			printf("Received get achievement progress for %s\n", ovr_AchievementProgress_GetName(singleCheevo));

			if (ovr_AchievementProgress_GetIsUnlocked(singleCheevo)) {
				printf("Cheevo is currently unlocked\n");
				printf("Cheevo was unlocked at %ld\n", ovr_AchievementProgress_GetUnlockTime(singleCheevo));
			}
			else {
				printf("Cheevo is NOT currently unlocked\n");
			}

			const char *bitField = ovr_AchievementProgress_GetBitfield(singleCheevo);
			unsigned long long count = ovr_AchievementProgress_GetCount(singleCheevo);

			printf("Current Progress: bitfield: %s count: %llu\n", bitField, count);

		}
		else {
			printf("No progress to report.\n");
		}
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received get achievement progress failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::unlockAchievement(char* cheevoName)
{
	printf("\nTrying to unlock achievement %s\n", cheevoName);

	ovrRequest req;

	req = ovr_Achievements_Unlock((const char*)cheevoName);
}

void SampleGame::processUnlockAchievement(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Cheevo is now unlocked\n");
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received unlock achievement failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::addAchievementCount(char* cheevoName, char* value)
{

	printf("\nTrying to add %lld to achievement %s\n", _atoi64(value), cheevoName);

	ovrRequest req;

	req = ovr_Achievements_AddCount((const char*)cheevoName, _atoi64(value));
}

void SampleGame::processAddAchievementCount(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Cheevo count added.\n");
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received add count achievement failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::addAchievementBitfield(char* cheevoName, char* value)
{
	printf("\nTrying to add %s to achievement %s\n", value, cheevoName);

	ovrRequest req;

	req = ovr_Achievements_AddFields((const char*)cheevoName, (const char*)value);
}

void SampleGame::processAddAchievementBitfield(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Cheevo field added.\n");
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received add field achievement failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::writeLeaderboardEntry(char* leaderboardName, char* value)
{
	printf("\nTrying to write leaderboard entry to  %s\n", leaderboardName);

	ovrRequest req;

	char DummyData[] {'T', 'e', 's', 't'};
	req = ovr_Leaderboard_WriteEntry(leaderboardName, _atoi64(value), &DummyData, sizeof(DummyData), false);
}

void SampleGame::processWriteLeaderboardEntry(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		ovrLeaderboardUpdateStatusHandle updateResult = ovr_Message_GetLeaderboardUpdateStatus(message);
		if (ovr_LeaderboardUpdateStatus_GetDidUpdate(updateResult)) {
			printf("Leaderboard entry was updated.\n");
		}
		else
		{
			printf("Leaderboard entry was NOT updated.\n");
		}
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received leaderboard write failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void SampleGame::getLeaderboardEntries(char* leaderboardName)
{
	printf("\nTrying to get leaderboard entries for  %s\n", leaderboardName);

	ovrRequest req;

	req = ovr_Leaderboard_GetEntries(leaderboardName, 10 /* limit to return */, ovrLeaderboard_FilterNone, ovrLeaderboard_StartAtTop);
}

void SampleGame::processGetLeaderboardEntries(ovrMessageHandle message)
{
	if (!ovr_Message_IsError(message)) {
		ovrLeaderboardEntryArrayHandle results = ovr_Message_GetLeaderboardEntryArray(message);

		unsigned long long totalRows = ovr_LeaderboardEntryArray_GetTotalCount(results);

		size_t count = ovr_LeaderboardEntryArray_GetSize(results);

		printf("Total Rows in Leaderboard: %llu\n", totalRows);

		for (size_t x = 0; x < count; x++)
		{
			ovrLeaderboardEntryHandle entry = ovr_LeaderboardEntryArray_GetElement(results, x);

			ovrUserHandle user = ovr_LeaderboardEntry_GetUser(entry);
			long long score = ovr_LeaderboardEntry_GetScore(entry);
			unsigned int ranking = ovr_LeaderboardEntry_GetRank(entry);
			const char* extraData = ovr_LeaderboardEntry_GetExtraData(entry);
			unsigned long long timestamp = ovr_LeaderboardEntry_GetTimestamp(entry);

			printf("%d. %s %llu %s %llu", ranking, ovr_User_GetOculusID(user), score, extraData, timestamp);
		}
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received leaderboard fetch failure: %s\n", ovr_Error_GetMessage(error));
	}
}
