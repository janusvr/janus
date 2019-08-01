#pragma once

#define FREQ 48000              // Sample rate
#define BUFFER_SIZE 512
#define BACKSPACE_KEY 8
#define ENTER_KEY '\r'

#define CAP_SIZE 4800           // Capture frame size
#define NUM_BUFFERS 16          // Number of audio buffers to use
#define CONVERSION 32768        // OpenAL to OVR conversion
#define HZ 90					          // Target frame rate

class Audio
{
public:
	Audio() {};
	~Audio() {};

	int initialize(const char* appID, bool ovrMic);
	void shutdown();

	void processAudio();
	void mainLoop();

private:
	machineState currentState;

	bool exitGame = false;
	bool netConnected = false;
	char commandBuffer[BUFFER_SIZE];
	int commandIndex = 0;

	bool usingOVRMic;

	std::list<ALuint> bufferQueue;

	ALenum errorCode;
	ALuint outputBuffer[NUM_BUFFERS];
	ALuint audioSource;

	ALCdevice* audioDevice;
	ALCcontext* audioContext;
	ALCdevice* inputDevice;
	short buffer[FREQ * 2];
	float ovrBuffer[FREQ * 2];
	float micBuffer[FREQ * 2];

	ovrMicrophoneHandle micHandle = nullptr;

	void checkKeyboard();
	void processCommand();
	void pumpOVRMessages();

	void initializeAudio();

  void initOpenALMic();
  void initOVRMic();

	void sampleOpenALMic();
	void sampleOVRMic();

  void getVoipAudio();

	// OpenAL output methods
	void recaptureAudioBuffers();
	void addAudioToBuffer(size_t dataSize);
	void outputAudio();

	// Notification handlers
	void processGetEntitlement(ovrMessage *message);

  void processVoipPeerConnect(ovrMessage *message);
  void processVoipStateChange(ovrMessage *message);

	// Helper functions
	void outputCommands();
};
