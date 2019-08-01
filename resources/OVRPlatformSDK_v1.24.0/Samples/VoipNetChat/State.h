#define BUFFER_SIZE 512

// Pool name is defined on the Oculus developer portal
#define POOL_NAME "voip_pool"

typedef struct messageStruct
{
	int packetID;
	char textString[BUFFER_SIZE];
}chatMessage;

enum states{
	NOT_INIT = 0,
	IDLE,
	REQUEST_FIND,
	FINDING_ROOM,
	REQUEST_CREATE,
	REQUEST_JOIN,
	REQUEST_LEAVE,
	IN_EMPTY_ROOM,
	IN_FULL_ROOM
};

class machineState{
public:
	machineState() {
		currentState = NOT_INIT; 
		localUserID = 0;
		remoteUserID = 0;
		roomID = 0;
	};

	~machineState(){};

	void init(ovrMessage *message);
	int getCurrentState();

	void requestFindMatch();
	void findMatchResponse(ovrMessage *message);

	void foundMatch(ovrMessage *message);
	void joinRoomResponse(ovrMessage *message);
	void requestLeaveRoom();
	void leaveRoomResponse(ovrMessage *message);
	void updateRoom(ovrMessage *message);

	void outputMachineState();
	ovrID getRemoteUserID();

private:
	int	currentState;

	ovrID roomID;
	ovrID localUserID;
	ovrID remoteUserID;
};
