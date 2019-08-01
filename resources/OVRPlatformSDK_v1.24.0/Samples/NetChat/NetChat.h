#define BUFFER_SIZE 512
#define BACKSPACE_KEY 8
#define ENTER_KEY '\r'

class NetChat
{
public:
	NetChat(){};
	~NetChat(){};

	int init(const char* appID);
	void mainLoop();

private:
	machineState currentState;

	bool exitGame = false;
	char commandBuffer[BUFFER_SIZE];
	int commandIndex = 0;

	// "Game" specific date
	void checkKeyboard();
	void processCommand();
	void processNetPackets();
	void pumpOVRMessages();

	void outputCommands();

	// Notification handlers
	void processNetworkingPeerConnect(ovrMessage *message);
	void processNetworkingStateChange(ovrMessage *message);
	void processGetEntitlement(ovrMessage *message);

	// Helper functions
	void outputRoomDetails(ovrRoom *room);
	void outputUserArray(ovrUserArray* users);
};
