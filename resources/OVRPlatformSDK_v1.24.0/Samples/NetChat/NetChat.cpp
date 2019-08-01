#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <inttypes.h>

#include "OVR_Platform.h"
#include "OVR_Functions_Networking.h"
#include "OVR_NetworkingPeer.h"
#include "State.h"
#include "NetChat.h"

int NetChat::init(const char* appID)
{
	if (ovr_PlatformInitializeWindows(appID) != ovrPlatformInitialize_Success)
	{
		return 1;
	}

	ovr_Entitlement_GetIsViewerEntitled();

	printf("Press h for list of commands.\nCommand > ");

    printf("User locale is %s\n", ovr_GetLoggedInUserLocale());
    printf("User ID is %" PRId64 "\n", ovr_GetLoggedInUserID());

	// Get the currently logged in user to get our user ID.
	ovrRequest req;
	req = ovr_User_GetLoggedInUser();

	return 0;
}

void NetChat::mainLoop()
{
	while (!exitGame)
	{
		checkKeyboard();
		pumpOVRMessages();
		processNetPackets();
	}
}

void NetChat::checkKeyboard()
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

void NetChat::outputCommands()
{
  printf("\nList of Commands\n----------------\n"
    "h - list commands\n"
    "c - create chat room for matchmaking(Room Mode)\n"
    "d - create filtered chat room for matchmaking(Room Mode)\n"
    "f - find chat room from matchmaking(Bout Mode)\n"
    "g - find chat room for matchmaking w / o filters(Room Mode)\n"
    "i - find chat room for matchmaking using filters(Room Mode)\n"
    "l - leave current room\n"
    "1 - Start a rated match\n"
    "2 - Report match results\n"
    "s!<chat message> -Send chat message\n"
    "q - quit\n\n");
}

void NetChat::processCommand()
{
	char *command = NULL;
	char *param1 = NULL;
	char *nextToken = NULL;
	char seps[] = "!";

	// Grab the command parameters
	command = strtok_s(commandBuffer, seps, &nextToken);
	param1 = strtok_s(NULL, seps, &nextToken);

	if (command) {
		switch (command[0])
		{
		case 'h':
			outputCommands();
			break;
		case 'c':
			currentState.requestCreateRoom();
			break;
    case 'd':
      currentState.requestCreateFilterRoom();
      break;
		case 'f':
			currentState.requestFindMatch();
			break;
    case 'g':
      currentState.requestFindRoom();
      break;
		case 'i':
			currentState.requestFindFilteredRoom();
			break;
		case 'l':
			currentState.requestLeaveRoom();
			break;
		case '1':
			currentState.requestStartRatedMatch();
			break;
		case '2':
			currentState.requestReportResults();
			break;
		case 's':
			if (param1){
				currentState.sendChat(param1);
			}
			else {
				printf("Missing Params\n");
			}
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

void NetChat::processNetPackets()
{
	ovrPacketHandle packet;
	while (packet = ovr_Net_ReadPacket()) {
		ovrID sender = ovr_Packet_GetSenderID(packet);
		size_t packetSize =  ovr_Packet_GetSize(packet);

		chatMessage chatPacket;
		memcpy_s(&chatPacket, sizeof(chatMessage), ovr_Packet_GetBytes(packet), ovr_Packet_GetSize(packet));

		printf("\nReceived Packet from UserID: %llu\nReceived PacketID: %d\nChat Text: %s\n\n", sender, chatPacket.packetID, chatPacket.textString);
		ovr_Packet_Free(packet);
	}
}


void NetChat::pumpOVRMessages()
{
	ovrMessage* message = nullptr;

	while ((message = ovr_PopMessage()) != nullptr) {
		switch (ovr_Message_GetType(message)) {

		case ovrMessage_Matchmaking_CreateAndEnqueueRoom:
			currentState.createRoomResponse(message);
			break;

		case ovrMessage_Notification_Matchmaking_MatchFound:
			currentState.foundMatch(message);
			break;

		case ovrMessage_Matchmaking_Enqueue:
			currentState.findMatchResponse(message);
			break;

		case ovrMessage_Notification_Networking_PeerConnectRequest:
			processNetworkingPeerConnect(message);
			break;

		case ovrMessage_Notification_Networking_ConnectionStateChange:
			processNetworkingStateChange(message);
			break;

		case ovrMessage_User_GetLoggedInUser:
			currentState.init(message);
			break;

		case ovrMessage_Room_Join:
			currentState.joinRoomResponse(message);
			break;

		case ovrMessage_Room_Leave:
			currentState.leaveRoomResponse(message);
			break;

		case ovrMessage_Notification_Room_RoomUpdate:
			currentState.updateRoom(message);
			break;

		case ovrMessage_Matchmaking_StartMatch:
			currentState.startRatedMatchResponse(message);
			break;

		case ovrMessage_Matchmaking_ReportResultInsecure:
			currentState.reportResultsResponse(message);
			break;

		case ovrMessage_Entitlement_GetIsViewerEntitled:
			processGetEntitlement(message);
			break;

		default:
			fprintf(stderr, "unknown message %d", ovr_Message_GetType(message));
		}
		ovr_FreeMessage(message);
	}
}

void NetChat::processNetworkingPeerConnect(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {
		ovrNetworkingPeer *netPeer = ovr_Message_GetNetworkingPeer(message);

		printf("Received peer connect request success from %llu\n", ovr_NetworkingPeer_GetID(netPeer));

		ovr_Net_Accept(ovr_NetworkingPeer_GetID(netPeer));
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received peer connect request failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void NetChat::processNetworkingStateChange(ovrMessage *message)
{
	if (!ovr_Message_IsError(message)) {
		ovrNetworkingPeer *netPeer = ovr_Message_GetNetworkingPeer(message);

		printf("Received networking state change from: %llu\n", ovr_NetworkingPeer_GetID(netPeer));

		ovrPeerConnectionState netState = ovr_NetworkingPeer_GetState(netPeer);
		switch (netState)
		{
		case ovrPeerState_Connected:
			printf("New State: Connected\n");
			break;

		case ovrPeerState_Timeout:
			printf("New State: Timeout\n");
			break;

		case ovrPeerState_Unknown:
		default:
			printf("New State: Unknown\n");
			break;
		}
	}
	else {
		const ovrErrorHandle error = ovr_Message_GetError(message);
		printf("Received networking state change failure: %s\n", ovr_Error_GetMessage(error));
	}
}

void NetChat::outputRoomDetails(ovrRoom *room)
{
	printf("Room ID: %llu, App ID: %llu, Description: %s\n", ovr_Room_GetID(room), ovr_Room_GetApplicationID(room), ovr_Room_GetDescription(room));
	size_t maxUsers = ovr_Room_GetMaxUsers(room);
	printf("maxUsers: %d\nusers in room:\n", maxUsers);
	ovrUser* owner = ovr_Room_GetOwner(room);
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
		printf("Testkey value: %s\n", ovr_DataStore_GetValue(dataStore, "testkey"));
	}

	ovrUserArray* users = ovr_Room_GetUsers(room);
	outputUserArray(users);
}

void NetChat::outputUserArray(ovrUserArray* users)
{
	size_t nUsers = ovr_UserArray_GetSize(users);
	for (size_t i = 0; i < nUsers; ++i) {
		ovrUser* user = ovr_UserArray_GetElement(users, i);
		printf("user %llu %s %s %s\n", ovr_User_GetID(user), ovr_User_GetOculusID(user), ovr_User_GetPresence(user), ovr_User_GetInviteToken(user));
	}
}

void NetChat::processGetEntitlement(ovrMessage *message)
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
