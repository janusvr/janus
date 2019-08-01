#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <fstream>
#include <iostream>

#include "OVR_Platform.h"
#include "OVR_Functions_Networking.h"
#include "OVR_NetworkingPeer.h"
#include "State.h"

void machineState::init(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {

		ovrUser * myUser = ovr_Message_GetUser(message);
		localUserID = ovr_User_GetID(myUser);

		currentState = IDLE;
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received get current logged in user failure: %s\n", ovr_Error_GetMessage(error));
		currentState = NOT_INIT;

		// We failed to get the current logged in user. Retry.
		ovrRequest req;
		req = ovr_User_GetLoggedInUser();
	}
}

void machineState::requestCreateRoom()
{
	switch (currentState)
	{
	case NOT_INIT:
		printf("The app has not initialized properly and we don't know your userID.\n");
		break;

	case IDLE:
		printf("\nTrying to create a matchmaking room\n");
		ovrRequest req;
		req = ovr_Matchmaking_CreateAndEnqueueRoom(FILTER_POOL, 8, true, nullptr);
		currentState = REQUEST_CREATE;
		break;

	case REQUEST_FIND:
		printf("You have already made a request to find a room.  Please wait for that request to complete.\n");
		break;

	case FINDING_ROOM:
		printf("You have already currently looking for a room.  Please wait for the match to be made.\n");
		break;

	case REQUEST_JOIN:
		printf("We are currently trying to join a room.  Please wait to see if we can join it.\n");
		break;

	case REQUEST_LEAVE:
		printf("We are currently trying to leave a room.  Please wait to see if we can leave it.\n");
		break;

	case REQUEST_CREATE:
		printf("You have already requested a matchmaking room to be created.  Please wait for the room to be made.\n");
		break;

	case IN_EMPTY_ROOM:
		printf("You have already in a matchmaking room.  Please wait for an opponent to join.\n");
		break;

	case IN_FULL_ROOM:
		printf("You have already in a match.\n");
		break;

	default:
		printf("You have hit an unknown state.\n");
		break;
	}
}

void machineState::requestCreateFilterRoom()
{
	switch (currentState)
	{
	case NOT_INIT:
		printf("The app has not initialized properly and we don't know your userID.\n");
		break;

	case IDLE:
		printf("\nTrying to create a matchmaking room\n");
		ovrRequest req;

		// We're going to create a room that has the following values set:
		// game_type_name = "CTF"
		// map_name = "Really_Big_Map"
		//

		ovrMatchmakingCustomQueryData roomCustomQueryData;
		ovrKeyValuePair filters[2];

		filters[0].key = "game_type_name";
		filters[0].valueType = ovrKeyValuePairType_String;
		filters[0].stringValue = "CTF";
		filters[1].key = "map_name";
		filters[1].valueType = ovrKeyValuePairType_String;
		filters[1].stringValue = "Really_Big_Map";

		roomCustomQueryData.customQueryDataArray = filters;
		roomCustomQueryData.customQueryDataArrayCount = 2;			  // I have two kv pairs
		roomCustomQueryData.customQueryCriterionArray = nullptr;	// Because I am not searching
		roomCustomQueryData.customQueryCriterionArrayCount = 0;		// Because I am not searching
		
		req = ovr_Matchmaking_CreateAndEnqueueRoom(FILTER_POOL, 8, true, &roomCustomQueryData);
		currentState = REQUEST_CREATE;
		break;

	case REQUEST_FIND:
		printf("You have already made a request to find a room.  Please wait for that request to complete.\n");
		break;

	case FINDING_ROOM:
		printf("You have already currently looking for a room.  Please wait for the match to be made.\n");
		break;

	case REQUEST_JOIN:
		printf("We are currently trying to join a room.  Please wait to see if we can join it.\n");
		break;

	case REQUEST_LEAVE:
		printf("We are currently trying to leave a room.  Please wait to see if we can leave it.\n");
		break;

	case REQUEST_CREATE:
		printf("You have already requested a matchmaking room to be created.  Please wait for the room to be made.\n");
		break;

	case IN_EMPTY_ROOM:
		printf("You have already in a matchmaking room.  Please wait for an opponent to join.\n");
		break;

	case IN_FULL_ROOM:
		printf("You have already in a match.\n");
		break;

	default:
		printf("You have hit an unknown state.\n");
		break;
	}
}


void machineState::createRoomResponse(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received create matchmaking room success\n");

    ovrMatchmakingEnqueueResultAndRoomHandle roomAndData = ovr_Message_GetMatchmakingEnqueueResultAndRoom(message);
    ovrRoomHandle myRoom = ovr_MatchmakingEnqueueResultAndRoom_GetRoom(roomAndData);

	roomID = ovr_Room_GetID(myRoom);

		printf("RoomID: %llu\n", ovr_Room_GetID(myRoom));
		currentState = IN_EMPTY_ROOM;
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received create matchmaking room failure: %s\n", ovr_Error_GetMessage(error));
		printf("You can only create a matchmaking room for pools of mode Room.  Make sure you have an appropriate pool setup on the Developer portal.\n");
		currentState = IDLE;
	}
}

void machineState::requestFindMatch()
{
	switch (currentState)
	{
	case NOT_INIT:
		printf("The app has not initialized properly and we don't know your userID.\n");
		break;

	case IDLE:
		printf("\nTrying to find a matchmaking room\n");
		ovrRequest req;
		req = ovr_Matchmaking_Enqueue(BOUT_POOL, nullptr);
		currentState = REQUEST_FIND;
		break;

	case REQUEST_FIND:
		printf("You have already made a request to find a room.  Please wait for that request to complete.\n");
		break;

	case FINDING_ROOM:
		printf("You have already currently looking for a room.  Please wait for the match to be made.\n");
		break;

	case REQUEST_JOIN:
		printf("We are currently trying to join a room.  Please wait to see if we can join it.\n");
		break;

	case REQUEST_LEAVE:
		printf("We are currently trying to leave a room.  Please wait to see if we can leave it.\n");
		break;

	case REQUEST_CREATE:
		printf("You have already requested a matchmaking room to be created.  Please wait for the room to be made.\n");
		break;

	case IN_EMPTY_ROOM:
		printf("You have already in a matchmaking room.  Please wait for an opponent to join.\n");
		break;

	case IN_FULL_ROOM:
		printf("You have already in a match.\n");
		break;

	default:
		printf("You have hit an unknown state.\n");
		break;
	}
}

void machineState::findMatchResponse(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received matchmaking enqueue success.  We are now looking for a room\n");
		currentState = REQUEST_FIND;
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received matchmaking enqueue failure: %s\n", ovr_Error_GetMessage(error));
		printf("You can enque for Bout or Room pools.  Make sure you have an appropriate pool setup on the Developer portal.\n");
		currentState = IDLE;
	}
}

void machineState::requestFindRoom()
{
  switch (currentState)
  {
  case NOT_INIT:
    printf("The app has not initialized properly and we don't know your userID.\n");
    break;

  case IDLE:
    printf("\nTrying to find a matchmaking room\n");

    ovrRequest req;
    req = ovr_Matchmaking_Enqueue(FILTER_POOL, nullptr);
    currentState = REQUEST_FIND;
    break;

  case REQUEST_FIND:
    printf("You have already made a request to find a room.  Please wait for that request to complete.\n");
    break;

  case FINDING_ROOM:
    printf("You have already currently looking for a room.  Please wait for the match to be made.\n");
    break;

  case REQUEST_JOIN:
    printf("We are currently trying to join a room.  Please wait to see if we can join it.\n");
    break;

  case REQUEST_LEAVE:
    printf("We are currently trying to leave a room.  Please wait to see if we can leave it.\n");
    break;

  case REQUEST_CREATE:
    printf("You have already requested a matchmaking room to be created.  Please wait for the room to be made.\n");
    break;

  case IN_EMPTY_ROOM:
    printf("You have already in a matchmaking room.  Please wait for an opponent to join.\n");
    break;

  case IN_FULL_ROOM:
    printf("You have already in a match.\n");
    break;

  default:
    printf("You have hit an unknown state.\n");
    break;
  }
}

void machineState::requestFindFilteredRoom()
{
	switch (currentState)
	{
	case NOT_INIT:
		printf("The app has not initialized properly and we don't know your userID.\n");
		break;

	case IDLE:
		printf("\nTrying to find a matchmaking room\n");

		// Our search filter criterion
		//
		// We're filtering using two different queries setup on the developer portal
		//
    // map - query to filter by map.  The query allows you to filter with up to two different maps using keys called 'map_1' and 'map_2'
    // game_type - query to filter by game type.  The query allows you to filter with up to two different game types using keys called 'type_1' and 'type_2'
		//
		// In the example below we are filtering for matches that are of type CTF and on either Big_Map or Really_Big_Map.
		//

    ovrMatchmakingCriterion queries[2];
    ovrMatchmakingCustomQueryData userCustomQueryData;

    // Map query
    // We are looking for a match using either "Really Big Map" or "Big Map"
    ovrKeyValuePair custom_params_map[2];
    custom_params_map[0].key = "map_param_1";
    custom_params_map[0].valueType = ovrKeyValuePairType_String;
    custom_params_map[0].stringValue = "Really_Big_Map";
    custom_params_map[1].key = "map_param_2";
    custom_params_map[1].valueType = ovrKeyValuePairType_String;
    custom_params_map[1].stringValue = "Big_Map";

    queries[0].key = "map";
    queries[0].importance = ovrMatchmaking_ImportanceRequired;
    queries[0].parameterArray = custom_params_map;
    queries[0].parameterArrayCount = 2;

    // Game type query
    // We are looking for a "CTF" game type.
    ovrKeyValuePair custom_params_gametype[1];
    custom_params_gametype[0].key = "game_type_param";
    custom_params_gametype[0].valueType = ovrKeyValuePairType_String;
    custom_params_gametype[0].stringValue = "CTF";

    queries[1].key = "game_type";
    queries[1].importance = ovrMatchmaking_ImportanceRequired;
    queries[1].parameterArray = custom_params_gametype;
    queries[1].parameterArrayCount = 1;

    userCustomQueryData.customQueryDataArray = nullptr;       // Because I am searching
    userCustomQueryData.customQueryDataArrayCount = 0;        // Because I am searching
    userCustomQueryData.customQueryCriterionArray = &queries[0];
    userCustomQueryData.customQueryCriterionArrayCount = 2;

		ovrRequest req;
		req = ovr_Matchmaking_Enqueue(FILTER_POOL, &userCustomQueryData);
		currentState = REQUEST_FIND;
		break;

	case REQUEST_FIND:
		printf("You have already made a request to find a room.  Please wait for that request to complete.\n");
		break;

	case FINDING_ROOM:
		printf("You have already currently looking for a room.  Please wait for the match to be made.\n");
		break;

	case REQUEST_JOIN:
		printf("We are currently trying to join a room.  Please wait to see if we can join it.\n");
		break;

	case REQUEST_LEAVE:
		printf("We are currently trying to leave a room.  Please wait to see if we can leave it.\n");
		break;

	case REQUEST_CREATE:
		printf("You have already requested a matchmaking room to be created.  Please wait for the room to be made.\n");
		break;

	case IN_EMPTY_ROOM:
		printf("You have already in a matchmaking room.  Please wait for an opponent to join.\n");
		break;

	case IN_FULL_ROOM:
		printf("You have already in a match.\n");
		break;

	default:
		printf("You have hit an unknown state.\n");
		break;
	}

}

void machineState::foundMatch(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received find matchmaking room success. We are now going to request to join the room.\n");

		ovrRoom *myRoom = ovr_Message_GetRoom(message);
		printf("RoomID: %llu\n", ovr_Room_GetID(myRoom));
		ovr_Room_Join(ovr_Room_GetID(myRoom), true);
		currentState = REQUEST_JOIN;
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received find matchmaking room failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void machineState::joinRoomResponse(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received join room success\n");

		// Try to pull out remote user's ID if they have already joined
		ovrRoom *newRoom = ovr_Message_GetRoom(message);
		roomID = ovr_Room_GetID(newRoom);

		ovrUserArray *users = ovr_Room_GetUsers(newRoom);

		currentState = IN_EMPTY_ROOM;

		for (size_t x = 0; x < ovr_UserArray_GetSize(users); x++)
		{
			ovrUser *nextUser = ovr_UserArray_GetElement(users, x);

			if (localUserID != ovr_User_GetID(nextUser)) {
				remoteUserID = ovr_User_GetID(nextUser);
				printf("Opponent's user ID: %llu\n", remoteUserID);
				currentState = IN_FULL_ROOM;
			}
		}

	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received join room failure: %s\n", ovr_Error_GetMessage(error));
		printf("It's possible that the room filled up before you could join it.\n");
		currentState = IDLE;
	}
}

void machineState::requestLeaveRoom()
{
	switch (currentState)
	{
	case NOT_INIT:
		printf("The app has not initialized properly and we don't know your userID.\n");
		break;

	case IDLE:
	case REQUEST_FIND:
	case FINDING_ROOM:
	case REQUEST_JOIN:
	case REQUEST_CREATE:
		printf("You are currently not in a room to leave.\n");
		break;

	case REQUEST_LEAVE:
		printf("We are currently trying to leave a room.  Please wait to see if we can leave it.\n");
		break;

	case IN_EMPTY_ROOM:
	case IN_FULL_ROOM:
		printf("\nTrying to leave room\n");
		ovrRequest req;
		req = ovr_Room_Leave(roomID);
		break;

	default:
		printf("You have hit an unknown state.\n");
		break;
	}
}

void machineState::leaveRoomResponse(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {
		printf("We were able to leave room %llu\n", roomID);

		roomID = 0;
		remoteUserID = 0;
		currentState = IDLE;
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received leave room failure: %s\n", ovr_Error_GetMessage(error));

	}
}

void machineState::sendChat(char* chatText)
{
	switch (currentState)
	{
	case NOT_INIT:
		printf("The app has not initialized properly and we don't know your userID.\n");
		break;

	case IDLE:
	case REQUEST_FIND:
	case FINDING_ROOM:
	case REQUEST_JOIN:
	case REQUEST_CREATE:
	case REQUEST_LEAVE:
	case IN_EMPTY_ROOM:
		printf("You need to be in a room with another player to send a message.\n");
		break;

	case IN_FULL_ROOM:
		{
			 chatMessage newMessage;

			 // Create a packet to send with the packet ID and string payload
			 lastPacketID++;
			 newMessage.packetID = lastPacketID;
			 strncpy_s(newMessage.textString, BUFFER_SIZE, chatText, strlen(chatText));

			 char buffer[sizeof(chatMessage)];
			 memcpy_s(buffer, sizeof(chatMessage), &newMessage, sizeof(chatMessage));

			 bool req;

			 req = ovr_Net_SendPacket(remoteUserID, sizeof(buffer), buffer, ovrSend_Reliable);

			 if (!req) {
				 printf("Failed to send packet.\n");
			 }
		}
		break;

	default:
		printf("You have hit an unknown state.\n");
		break;
	}
}

void machineState::requestStartRatedMatch()
{
	switch (currentState)
	{
	case NOT_INIT:
		printf("The app has not initialized properly and we don't know your userID.\n");
		break;

	case IDLE:
	case REQUEST_FIND:
	case FINDING_ROOM:
	case REQUEST_JOIN:
	case REQUEST_CREATE:
	case REQUEST_LEAVE:
	case IN_EMPTY_ROOM:
		printf("You need to be in a room with another player to start a rated match.\n");
		break;

	case IN_FULL_ROOM:
		printf("\nTrying to start a rated match.  This call should be made once a rated match begins so we will be able to submit results after the game is done.\n");
		ovrRequest req;

		req = ovr_Matchmaking_StartMatch(roomID);
		break;

	default:
		printf("You have hit an unknown state.\n");
		break;
	}
}

void machineState::startRatedMatchResponse(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Started a rated match\n");
		ratedMatchStarted = true;
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received starting rated match failure: %s\n", ovr_Error_GetMessage(error));
		printf("Your matchmaking pool needs to have a skill pool associated with it to play rated matches\n");
	}
}

void machineState::requestReportResults()
{
	switch (currentState)
	{
	case NOT_INIT:
		printf("The app has not initialized properly and we don't know your userID.\n");
		break;

	case IDLE:
	case REQUEST_FIND:
	case FINDING_ROOM:
	case REQUEST_JOIN:
	case REQUEST_CREATE:
	case REQUEST_LEAVE:
		printf("You need to be in a room with another player to report results on a rated match.\n");
		break;

	case IN_EMPTY_ROOM:
	case IN_FULL_ROOM:
		if (ratedMatchStarted){
			printf("\nSubmitting rated match results.\n");
			ovrRequest req;
			ovrKeyValuePair results[2];

			char localUserBuffer[21];
			char remoteUserBuffer[21];

			// copy to buffer
			sprintf_s(localUserBuffer, 21, "%llu", localUserID);
			sprintf_s(localUserBuffer, 21, "%llu", remoteUserID);

			results[0].intValue = 1;
			results[0].valueType = ovrKeyValuePairType_Int;
			results[0].key = (char*)&localUserBuffer;

			results[1].intValue = 2;
			results[1].valueType = ovrKeyValuePairType_Int;
			results[1].key = (char*)&remoteUserBuffer;

			req = ovr_Matchmaking_ReportResultInsecure(roomID, results, 2);
		}
		else {
			printf("\nYou can't report results unless you've already started a rated match\n");
		}
		break;

	default:
		printf("You have hit an unknown state.\n");
		break;
	}
}

void machineState::reportResultsResponse(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Rated match results reported.\nNow attempting to leave room.\n");
		ratedMatchStarted = false;
		requestLeaveRoom();
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received reporting rated match results failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void machineState::updateRoom(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {
		printf("Received room update notification\n");

		if (currentState == IN_EMPTY_ROOM)
		{
			// Check to see if this update is another user joining
			ovrRoom *updatedRoom = ovr_Message_GetRoom(message);
			ovrUserArray *users = ovr_Room_GetUsers(updatedRoom);

			for (size_t x = 0; x < ovr_UserArray_GetSize(users); x++)
			{
				ovrUser *nextUser = ovr_UserArray_GetElement(users, x);

				if (localUserID != ovr_User_GetID(nextUser)) {
					remoteUserID = ovr_User_GetID(nextUser);
					printf("User ID: %llu has joined.\n", remoteUserID);
					currentState = IN_FULL_ROOM;
				}
			}
		}
		else {
			// Check to see if this update is the other user leaving
			ovrRoom *updatedRoom = ovr_Message_GetRoom(message);
			ovrUserArray *users = ovr_Room_GetUsers(updatedRoom);

			if (ovr_UserArray_GetSize(users) == 1)
			{
				printf("User ID: %llu has left.\n", remoteUserID);
				remoteUserID = 0;
				currentState = IN_EMPTY_ROOM;
			}
		}
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received room Update failure: %s\n", ovr_Error_GetMessage(error));
	}
}
