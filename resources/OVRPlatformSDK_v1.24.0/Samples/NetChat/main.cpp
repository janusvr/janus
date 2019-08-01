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
#include "NetChat.h"

int main(int argc, char** argv) {

	if (argc < 2) {
		fprintf(stderr, "usage: NetChat.exe <appID>");
		exit(1);
	}

	NetChat oApp;

	if (oApp.init(argv[1]) != 0)
	{
		fprintf(stderr, "Could not initialize the Oculus Platform\n");
		return 1;
	}
	oApp.mainLoop();

	return 0;
}
