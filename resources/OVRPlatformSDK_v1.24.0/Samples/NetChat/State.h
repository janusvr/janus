#define BUFFER_SIZE 512

// Pools are defined on the Oculus developer portal
//
// For this test we have a pool created with the pool key set as 'filter_pool'
// Mode is set to 'Room'
// Skill Pool is set to 'None'
// We are not considering Round Trip Time
// The following Data Settings are set:
//  key: map_name, Type: STRING, String options: Small_Map, Big_Map, Really_Big_Map
//  key: game_type, Type: STRING, String Options: deathmatch, CTF
//
// We also have the following two queries defined:
//  Query Key: map
//  Template: Set (String)
//  Key: map_name
//  Wildcards: map_param_1, map_param_2
//
//  Query Key: game_type
//  Template: Set (String)
//  Key: game_type_name
//  Wildcards: game_type_param
//
#define FILTER_POOL "filter_pool"


// For this test we have a pool created with the pool key set as 'bout_pool'
// Mode is set to 'Bout'
// Skill Pool is set to 'None'
// We are not considering Round Trip Time
// No Data Settings are set:
//
#define BOUT_POOL "bout_pool"

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

	void requestCreateRoom();
	void createRoomResponse(ovrMessage *message);
	void requestCreateFilterRoom();
	void requestFindMatch();
	void findMatchResponse(ovrMessage *message);
  void requestFindRoom();
  void requestFindFilteredRoom();

	void foundMatch(ovrMessage *message);
	void joinRoomResponse(ovrMessage *message);
	void requestLeaveRoom();
	void leaveRoomResponse(ovrMessage *message);
	void requestStartRatedMatch();
	void startRatedMatchResponse(ovrMessage *message);
	void requestReportResults();
	void reportResultsResponse(ovrMessage *message);
	void sendChat(char* chatMessage);
	void updateRoom(ovrMessage *message);

private:
	int	currentState;
	int lastPacketID = 0;
	bool ratedMatchStarted = false;

	ovrID roomID;
	ovrID localUserID;
	ovrID remoteUserID;
};
