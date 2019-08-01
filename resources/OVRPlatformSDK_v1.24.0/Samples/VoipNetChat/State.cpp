#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <fstream>
#include <iostream>

#include "OVR_Platform.h"
#include "OVR_Functions_Voip.h"
#include "OVR_Functions_Networking.h"
#include "OVR_NetworkingPeer.h"
#include "State.h"

void machineState::init(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {

		ovrUser * myUser = ovr_Message_GetUser(message);
		localUserID = ovr_User_GetID(myUser);
    remoteUserID = 0;

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

int machineState::getCurrentState()
{
	return currentState;
}

ovrID machineState::getRemoteUserID()
{
	return remoteUserID;
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
		req = ovr_Matchmaking_Enqueue(POOL_NAME, nullptr);
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
        ovr_Voip_Start(remoteUserID);
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
		if (remoteUserID != 0)
		{
      ovr_Voip_Stop(remoteUserID);
		}
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
          ovr_Voip_Start(remoteUserID);
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
        ovr_Voip_Stop(remoteUserID);
			}
		}
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received room Update failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void machineState::outputMachineState()
{
	switch (currentState)
	{
	case NOT_INIT:
	case IDLE:
	case REQUEST_FIND:
	case FINDING_ROOM:
	case REQUEST_JOIN:
	case REQUEST_CREATE:
	case REQUEST_LEAVE:
	case IN_EMPTY_ROOM:
		printf("\nNot in a chat room with another user.  Mic is in local loopback mode.\n");
		break;

	case IN_FULL_ROOM:
		printf("\nIn a chat room with another user.  Mic is sending VoIP.\n");
		break;

	default:
		printf("You have hit an unknown state.\n");
		break;
	}
}
