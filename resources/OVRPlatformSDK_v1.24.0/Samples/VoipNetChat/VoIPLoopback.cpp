// OpenAL header files
#include <al.h>
#include <alc.h>

#include <list>
#include <conio.h>

#include "OVR_Platform.h"
#include "State.h"
#include "Audio.h"

Audio oAudio;

int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: VoIPLoopback.exe <appID>");
		exit(1);
	}

	Audio oAudio;

	// Init with false to use OpenAL Microphone instead of OVR
	if (oAudio.initialize(argv[1], true) != 0)
	{
		fprintf(stderr, "Could not initialize the Oculus Platform\n");
		return 1;
	}

	oAudio.mainLoop();

  return 0;
}

