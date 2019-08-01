// Copyright 2016 Oculus VR, LLC All Rights reserved.

#include "OculusPlatformSample.h"
#include "OSSMainMenuWidget.h"



void UOSSMainMenuWidget::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
	UE_LOG_ONLINE(Display, TEXT("Hit OnJoinSessionComplete in main menu"));

	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(SessionName);

	FString TravelURL;
	APlayerController *PlayerController = NULL;
	UWorld * const TheWorld = GetWorld();
	if (!TheWorld)
	{
		UE_LOG_ONLINE(Display, TEXT("The World Does Not Exist.  Existential Crisis"));
		return;
	}
	else
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
		auto gamemode = (AGameModeBase*)GetWorld()->GetAuthGameMode();
		gamemode->bUseSeamlessTravel = false;
		UE_LOG_ONLINE(Display, TEXT("Seamless Travel Set to : %s"), gamemode->bUseSeamlessTravel ? TEXT("True") : TEXT("False"));
	}

	if (Session)
	{
		UE_LOG_ONLINE(Display, TEXT("Got back %s's session: %s"), *Session->OwningUserName, *SessionName.ToString());
		auto OwningUserNameBP = *Session->OwningUserName; //Can be used by Blueprint to update UI

		if (*Session->OwningUserId == *Online::GetIdentityInterface()->GetUniquePlayerId(0)) // I am the owner
		{
			UE_LOG_ONLINE(Display, TEXT("I am the Owner, calling Server Travel to map"));
			GetWorld()->ServerTravel(TEXT("/Game/Maps/OnlineSessionOculusExample?listen"));
		}
		else
		{
			UE_LOG_ONLINE(Display, TEXT("Not the session owner"));
			if (PlayerController && OculusSessionInterface->GetResolvedConnectString(SessionName, TravelURL))
			{
				UE_LOG_ONLINE(Display, TEXT("Calling ClientTravel to: %s"), *TravelURL);
				// Finally call the ClienTravel
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
			}
		}
		auto gamemode = (AGameModeBase*)GetWorld()->GetAuthGameMode();
		gamemode->bUseSeamlessTravel = true;
		UE_LOG_ONLINE(Display, TEXT("Seamless Travel Set to : %s"), gamemode->bUseSeamlessTravel ? TEXT("True") : TEXT("False"));
	}
	

}

