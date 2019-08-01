#include "OVR_Platform.h"
#include "SampleGame.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main(int argc, char** argv) {

	if (argc < 2) {
		fprintf(stderr, "usage: NativePlatformSample.exe <appID>");
		exit(1);
	}

	SampleGame oGame;

	if (oGame.init(argv[1]) != 0)
	{
		fprintf(stderr, "Could not initialize the Oculus Platform\n");
		return 1;
	}
	oGame.gameLoop();
	return 0;
}


