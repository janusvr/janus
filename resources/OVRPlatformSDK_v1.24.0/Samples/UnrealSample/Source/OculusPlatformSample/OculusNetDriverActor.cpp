// Copyright 2016 Oculus VR, LLC All Rights reserved.

#include "OculusPlatformSample.h"
#include "OculusNetDriverActor.h"

void AOculusNetDriverActor::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (!bWasSuccessful)
	{
		UE_LOG_ONLINE(Warning, TEXT("Unable to login with oculus! %s"), *Error);
		return;
	}
	UE_LOG_ONLINE(Display, TEXT("Logged in successfully to oculus!"));

	// Get the App scoped ID
	PlayerID = UserId.ToString();
	OnPlayerIDUpdate(*PlayerID);
}

// Called every frame
void AOculusNetDriverActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	auto World = GetWorld();

	auto NetMode = World->GetNetMode();

	if (NetMode == NM_Standalone)
	{
		NetModeStatus = FString::Printf(TEXT("Standalone"));
		NetDriverStatus = FString::Printf(TEXT("N/A"));
		return;
	}

	auto NetDriver = World->GetNetDriver();

	if (NetMode == NM_ListenServer)
	{	
		NetModeStatus = FString::Printf(TEXT("Listen Server: %s"), *PlayerID);
		auto PendingClients = 0;
		auto ConnectedClients = 0;
		for (auto Connection : NetDriver->ClientConnections)
		{
			if (Connection->State == EConnectionState::USOCK_Open)
			{
				ConnectedClients++;
			}
			else
			{
				PendingClients++;
			}
		}
		auto ClientCount = NetDriver->ClientConnections.Num();
		NetDriverStatus = FString::Printf(TEXT("Pending: %d Connected Clients: %d"), PendingClients, ConnectedClients);
	}
	else if (NetMode == NM_Client)
	{
		NetModeStatus = FString::Printf(TEXT("Client: %s"), *PlayerID);
		if (NetDriver && NetDriver->IsAvailable())
		{
			NetDriverStatus = FString::Printf(TEXT("NetDriver is connected to %s"), *NetDriver->ServerConnection->RemoteAddressToString());
		}
		else
		{
			NetDriverStatus = FString::Printf(TEXT("No NetDriver for world"));
		}
	}
}
