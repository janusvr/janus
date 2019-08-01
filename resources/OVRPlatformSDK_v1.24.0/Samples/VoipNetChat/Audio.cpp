#include <windows.h>
#include <conio.h>
#include <al.h>    // OpenAL header files
#include <alc.h>
#include <list>
#include <chrono>

#include "OVR_Platform.h"
#include "OVR_Functions_Voip.h"
#include "State.h"
#include "Audio.h"

int Audio::initialize(const char* appID, bool ovrMic)
{
  usingOVRMic = ovrMic;

  if (ovr_PlatformInitializeWindows(appID) != ovrPlatformInitialize_Success) {
    return 1;
  }

  ovr_Entitlement_GetIsViewerEntitled();

  // Get the currently logged in user to get our user ID.
  ovrRequest req;
  req = ovr_User_GetLoggedInUser();

  initializeAudio();

  return 0;
}

void Audio::mainLoop()
{
  using namespace std::chrono;

  double frameLength = 1.0 / HZ;

  high_resolution_clock::time_point frameStart;
  high_resolution_clock::time_point frameEnd;
  duration<double> time_span;

  while (!exitGame) {
    frameStart = high_resolution_clock::now();

    checkKeyboard();
    pumpOVRMessages();
    processAudio();

    do {
      frameEnd = high_resolution_clock::now();
      time_span = duration_cast<duration<double>>(frameEnd - frameStart);
    } while (time_span.count() < frameLength);
  }

  shutdown();
}

void Audio::checkKeyboard()
{

  if (_kbhit()) {
    int key = _getch();

    switch (key) {
    case BACKSPACE_KEY:
      if (commandIndex > 0) {
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
      if (commandIndex < BUFFER_SIZE) {
        commandBuffer[commandIndex] = key;
        printf("%c", key);
        commandIndex++;
      }
      break;
    }
  }
}

void Audio::processCommand()
{
  char *command = NULL;
  char *nextToken = NULL;
  char seps[] = "!";

  // Grab the command parameter
  command = strtok_s(commandBuffer, seps, &nextToken);

  if (command) {
    switch (command[0])
    {
    case 'h':
      outputCommands();
      break;
    case 'f':
      currentState.requestFindMatch();
      break;
    case 'l':
      currentState.requestLeaveRoom();
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

void Audio::pumpOVRMessages()
{
  ovrMessage* message = nullptr;

  while ((message = ovr_PopMessage()) != nullptr) {
    switch (ovr_Message_GetType(message)) {

    case ovrMessage_Notification_Matchmaking_MatchFound:
      currentState.foundMatch(message);
      break;

    case ovrMessage_Matchmaking_Enqueue:
    case ovrMessage_Matchmaking_Enqueue2:
      currentState.findMatchResponse(message);
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

    case ovrMessage_Entitlement_GetIsViewerEntitled:
      processGetEntitlement(message);
      break;

    case ovrMessage_Notification_Voip_ConnectRequest:
      processVoipPeerConnect(message);
      break;

    case ovrMessage_Notification_Voip_StateChange:
      processVoipStateChange(message);
      break;

    default:
      fprintf(stderr, "unknown message %d", ovr_Message_GetType(message));
    }
    ovr_FreeMessage(message);
  }
}

void Audio::initializeAudio()
{
  // Get the default audio device
  audioDevice = alcOpenDevice(NULL);
  errorCode = alcGetError(audioDevice);

  // Create the audio context
  audioContext = alcCreateContext(audioDevice, NULL);
  alcMakeContextCurrent(audioContext);
  errorCode = alcGetError(audioDevice);

  if (usingOVRMic) {
    initOVRMic();
  } else {
    initOpenALMic();
  }

  // Create output buffers
  alGenBuffers(16, &outputBuffer[0]);
  errorCode = alGetError();

  // Queue our buffers
  for (int i = 0; i < NUM_BUFFERS; i++) {
    bufferQueue.push_back(outputBuffer[i]);
  }

  // Create a sound source
  alGenSources(1, &audioSource);
  errorCode = alGetError();

}

void Audio::initOpenALMic()
{
  // Request the default capture device with a half-second buffer
  inputDevice = alcCaptureOpenDevice(NULL, FREQ, AL_FORMAT_MONO16, FREQ / 2);
  errorCode = alcGetError(inputDevice);

  // Start capturing
  alcCaptureStart(inputDevice);
  errorCode = alcGetError(inputDevice);
}

void Audio::initOVRMic()
{
  CoInitialize(NULL);

  micHandle = ovr_Microphone_Create();
  ovr_Microphone_Start(micHandle);
}

void Audio::shutdown()
{
  if (usingOVRMic) {
    ovr_Microphone_Stop(micHandle);
    ovr_Microphone_Destroy(micHandle);
  } else {
    alcCaptureStop(inputDevice);
    alcCaptureCloseDevice(inputDevice);
  }

  // Stop the sources
  alSourceStopv(1, &audioSource);
  alSourcei(audioSource, AL_BUFFER, 0);

  // Clean-up 
  alDeleteSources(1, &audioSource);
  alDeleteBuffers(16, &outputBuffer[0]);
  errorCode = alGetError();
  alcMakeContextCurrent(NULL);
  errorCode = alGetError();
  alcDestroyContext(audioContext);
  alcCloseDevice(audioDevice);

}

void Audio::sampleOVRMic()
{
  recaptureAudioBuffers();
  size_t copied = ovr_Microphone_ReadData(micHandle, micBuffer, CAP_SIZE);

  // Convert OVR data to OpenAL format
  for (int i = 0; i < copied; i++) {  
    buffer[i] = micBuffer[i] * CONVERSION;
  }
  addAudioToBuffer(copied);
}

void Audio::sampleOpenALMic()
{
  ALCint samplesIn = 0;

  recaptureAudioBuffers();
  // Poll for captured audio
  alcGetIntegerv(inputDevice, ALC_CAPTURE_SAMPLES, 1, &samplesIn);
  
  if (samplesIn > CAP_SIZE) {
    // Grab the sound
    alcCaptureSamples(inputDevice, buffer, CAP_SIZE);
    addAudioToBuffer(CAP_SIZE);
  }
}

void Audio::getVoipAudio()
{
  recaptureAudioBuffers();

  size_t decodedSize = ovr_Voip_GetPCM(currentState.getRemoteUserID(), buffer, FREQ * 2);
  if (decodedSize > 0) {
    addAudioToBuffer(decodedSize);
  }
}

void Audio::recaptureAudioBuffers()
{
  ALint availBuffers = 0;
  ALuint bufferHolder[NUM_BUFFERS];

  alGetSourcei(audioSource, AL_BUFFERS_PROCESSED, &availBuffers);
  if (availBuffers > 0) {
    alSourceUnqueueBuffers(audioSource, availBuffers, bufferHolder);
    for (int i = 0; i < availBuffers; i++) {
      // Push the recovered buffers back on the queue
      bufferQueue.push_back(bufferHolder[i]);
    }
  }
}

void Audio::addAudioToBuffer(size_t dataSize)
{
  ALuint myBuff;

  // Put the captured data in a buffer
  if (!bufferQueue.empty()) {
    myBuff = bufferQueue.front(); bufferQueue.pop_front();
    alBufferData(myBuff, AL_FORMAT_MONO16, buffer, dataSize * sizeof(short), FREQ);

    // Queue the buffer
    alSourceQueueBuffers(audioSource, 1, &myBuff);
  }
}

void Audio::outputAudio()
{
  // Start the source playing if it's not already playing
  // Playing may stop if we don't get recent data
  ALint state = 0;
  alGetSourcei(audioSource, AL_SOURCE_STATE, &state);
  if (state != AL_PLAYING) {
    alSourcePlay(audioSource);
  }
}

void Audio::processAudio()
{
  if (netConnected) {
    getVoipAudio();
  }
  else {
    if (usingOVRMic) {
      sampleOVRMic();
    }
    else {
      sampleOpenALMic();
    }
  }

  outputAudio();
}

void Audio::processGetEntitlement(ovrMessage *message)
{
  if (!ovr_Message_IsError(message)) {
    printf("User has an entitlement\n");
    currentState.outputMachineState();
    printf("Press h for list of commands.\n\nCommand > ");
  }
  else {
    printf("Could NOT get an entitlement\n");
    exitGame = true;
  }
}

void Audio::outputCommands()
{
  currentState.outputMachineState();
  printf("\nList of Commands\n----------------\nh - list commands\nf - find chat room from matchmaking\nl - leave current room\nq - quit\n\n");
}

void Audio::processVoipPeerConnect(ovrMessage *message)
{
  if (!ovr_Message_IsError(message)) {

    ovrNetworkingPeerHandle netPeer = ovr_Message_GetNetworkingPeer(message);

    ovr_Voip_Accept(ovr_NetworkingPeer_GetID(netPeer));
    printf("Received Voip connect request\n");

  }
  else {
    const ovrErrorHandle error = ovr_Message_GetError(message);
    printf("Received Voip connect failure: %s\n", ovr_Error_GetMessage(error));
  }
}

void Audio::processVoipStateChange(ovrMessage *message)
{
  if (!ovr_Message_IsError(message)) {
    ovrNetworkingPeer *netPeer = ovr_Message_GetNetworkingPeer(message);

    printf("Received voip state change from: %llu\n", ovr_NetworkingPeer_GetID(netPeer));

    ovrPeerConnectionState netState = ovr_NetworkingPeer_GetState(netPeer);
    switch (netState)
    {
    case ovrPeerState_Connected:
      printf("New State: Connected\n");
      netConnected = true;
      break;

    case ovrPeerState_Timeout:
      printf("New State: Timeout\n");
      netConnected = false;
      break;

    case ovrPeerState_Closed:
      printf("New State: Closed\n");
      netConnected = false;
      break;

    case ovrPeerState_Unknown:
    default:
      printf("New State: Unknown\n");
      netConnected = false;
      break;
    }
  }
  else {
    const ovrErrorHandle error = ovr_Message_GetError(message);
    printf("Received voip state change failure: %s\n", ovr_Error_GetMessage(error));
  }
}
