// Copyright 2016 Oculus VR, LLC All Rights reserved.

#include "OculusPlatformSample.h"
#include "OnlineSessionSettings.h"
#include "OSSSessionWidget.h"


void UOSSSessionWidget::ReadOnlineFriends()
{

	auto OculusFriendsInterface = Online::GetFriendsInterface();

	UE_LOG_ONLINE(Display, TEXT("Trying to get friends list from server"));

	OculusFriendsInterface->ReadFriendsList(
		0,
		TEXT("onlinePlayers"),
		FOnReadFriendsListComplete::CreateUObject(this, &UOSSSessionWidget::OnReadFriendsListComplete)
		);
}

void UOSSSessionWidget::OnReadFriendsListComplete(int32 LocalUserNum, const bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	UE_LOG_ONLINE(Display, TEXT("Got results back from reading friends list"));
	if (bWasSuccessful)
	{
		auto OculusFriendsInterface = Online::GetFriendsInterface();
		TArray<TSharedRef<FOnlineFriend>> Friends;
		OculusFriendsInterface->GetFriendsList(0, ListName, Friends);
		UE_LOG_ONLINE(Display, TEXT("Online Friends list loaded.  Count of friends: %d"), Friends.Num());

		auto OculusSessionInterface = Online::GetSessionInterface();

		if (!OnFindFriendSessionCompleteDelegate.IsBound())
		{
			OnFindFriendSessionCompleteDelegate = FOnFindFriendSessionCompleteDelegate::CreateUObject(this, &UOSSSessionWidget::OnFindFriendSessionComplete);
			OculusSessionInterface->AddOnFindFriendSessionCompleteDelegate_Handle(0, OnFindFriendSessionCompleteDelegate);
		}

		for (auto Friend : Friends)
		{
			OculusSessionInterface->FindFriendSession(0, Friend->GetUserId().Get());
			FriendsToInvite.Add(Friend->GetDisplayName(), Friend->GetUserId());
			AddFriendToInviteList(Friend->GetDisplayName());
		}
	}
	else
	{
		UE_LOG_ONLINE(Error, TEXT("%s"), *ErrorStr);
	}
}

void UOSSSessionWidget::OnFindFriendSessionComplete(int32 LocalPlayerNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& FriendSearchResults)
{
	UE_LOG_ONLINE(Display, TEXT("Got result if friend is in a session"));
	if (bWasSuccessful)
	{
		if (FriendSearchResults.Num() > 0)
		{
			for (auto FriendSearchResult : FriendSearchResults)
			{
				UE_LOG_ONLINE(Display, TEXT("Search result is valid.  Adding to the list"));
				AddFriendToList(FriendSearchResult.Session.OwningUserName);
				FriendsSessions.Add(FriendSearchResult.Session.OwningUserName, FriendSearchResult);
			}
			return;
		}
	}
}

void UOSSSessionWidget::InviteFriendToSession(const FString& FriendName)
{
	UE_LOG_ONLINE(Display, TEXT("Trying to invite %s"), *FriendName);
	if (FriendsToInvite.Contains(FriendName))
	{
		auto OculusSessionInterface = Online::GetSessionInterface();
		bool bSuccess = OculusSessionInterface->SendSessionInviteToFriend(0, TEXT("Game"), FriendsToInvite[FriendName].Get());
		if (!bSuccess)
			UE_LOG_ONLINE(Warning, TEXT("Failed to invite %s"), *FriendName);
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("Could not find %s"), *FriendName);
	}
}

void UOSSSessionWidget::JoinFriendSession(const FString& FriendName)
{
	UE_LOG_ONLINE(Display, TEXT("Trying to join %s's session"), *FriendName);
	if (FriendsSessions.Contains(FriendName))
	{
		auto OculusSessionInterface = Online::GetSessionInterface();
		OculusSessionInterface->JoinSession(0, TEXT("Game"), FriendsSessions[FriendName]);
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("Could not find %s"), *FriendName);
	}
}

void UOSSSessionWidget::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(SessionName);
	FString TravelURL;
	APlayerController *PlayerController = NULL;
	UWorld * const TheWorld = GetWorld();
	if (!TheWorld)
	{
		UE_LOG_ONLINE(Warning, TEXT("The World Does Not Exist."));
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
		OwningUserNameBP = *Session->OwningUserName; //Can be used by Blueprint to update UI
		LastPlayerCount = Session->RegisteredPlayers.Num(); //save off the number of players in the session when I joined to compare later

		if (*Session->OwningUserId == *Online::GetIdentityInterface()->GetUniquePlayerId(0)) // I am the owner
		{
			UE_LOG_ONLINE(Display, TEXT("I am the session owner and will host"));
			//This is where you could call GetWorld()->ServerTravel(TEXT("/Game/Maps/Minimal_Default3?listen"));
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
		gamemode->bUseSeamlessTravel = true; //after first travel, start using seamless travel.
		UE_LOG_ONLINE(Display, TEXT("Seamless Travel Set to : %s"), gamemode->bUseSeamlessTravel ? TEXT("True") : TEXT("False"));
	}
	OnJoinSessionComplete(SessionName, JoinResult == EOnJoinSessionCompleteResult::Success);

}

void UOSSSessionWidget::CreateSession()
{
	auto OculusSessionInterface = Online::GetSessionInterface();

	if (!OculusSessionInterface.IsValid())
	{
		return;
	}

	UE_LOG_ONLINE(Display, TEXT("Trying to create a session"));

	if (!OnCreateSessionCompleteDelegate.IsBound())
	{
		OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UOSSSessionWidget::OnCreateSessionComplete);
		OculusSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
	}

	TSharedPtr<class FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->NumPublicConnections = 2;
	SessionSettings->bShouldAdvertise = true;
	OculusSessionInterface->CreateSession(/* Hosting Player Num*/ 0, TEXT("Game"), *SessionSettings);
}

void UOSSSessionWidget::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	auto OculusSessionInterface = Online::GetSessionInterface();

	if (!OculusSessionInterface.IsValid())
	{
		return;
	}

	UE_LOG_ONLINE(Display, TEXT("CreateSession Call complete"));

	auto Session = OculusSessionInterface->GetNamedSession(SessionName);

	if (Session)
	{
		UE_LOG_ONLINE(Display, TEXT("Session owned by %s"), *Session->OwningUserName);
		UE_LOG_ONLINE(Display, TEXT("Session state: %s"), EOnlineSessionState::ToString(Session->SessionState));
	}
	
	//Tell the BP we have tried to create a session
	OnCreateSessionCompleteBP(SessionName, bWasSuccessful);
}

void UOSSSessionWidget::StartSession(FName SessionName)
{

	UE_LOG_ONLINE(Display, TEXT("Start Session"));

	auto OculusSessionInterface = Online::GetSessionInterface();

	auto Session = OculusSessionInterface->GetNamedSession(SessionName);

	if (Session)
	{
		UE_LOG_ONLINE(Display, TEXT("Session owned by %s"), *Session->OwningUserName);
		UE_LOG_ONLINE(Display, TEXT("Session state: %s"), EOnlineSessionState::ToString(Session->SessionState));
	}

	if (!OnStartSessionCompleteDelegate.IsBound())
	{
		OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UOSSSessionWidget::OnStartSessionComplete);
		OculusSessionInterface->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
	}

	OculusSessionInterface->StartSession(SessionName);
}



void UOSSSessionWidget::EndSession(FName SessionName)
{
	UE_LOG_ONLINE(Display, TEXT("End Session"));

	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(SessionName);

	if (!OnEndSessionCompleteDelegate.IsBound())
	{
		OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(this, &UOSSSessionWidget::OnEndSessionComplete);
		OculusSessionInterface->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
	}

	OculusSessionInterface->EndSession(SessionName);

	if (Session)
	{
		UE_LOG_ONLINE(Display, TEXT("Session owned by %s"), *Session->OwningUserName);
		UE_LOG_ONLINE(Display, TEXT("Session state: %s"), EOnlineSessionState::ToString(Session->SessionState));
	}
}

void UOSSSessionWidget::DestroySession(FName SessionName)
{
	UE_LOG_ONLINE(Display, TEXT("Destroy Session"));

	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(SessionName);

	if (Session)
	{
		UE_LOG_ONLINE(Display, TEXT("Session owned by %s"), *Session->OwningUserName);
		UE_LOG_ONLINE(Display, TEXT("Session state: %s"), EOnlineSessionState::ToString(Session->SessionState));
	}

	if (!OnDestroySessionCompleteDelegate.IsBound())
	{
		OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UOSSSessionWidget::OnDestroySessionComplete);
		OculusSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
	}

	OculusSessionInterface->DestroySession(SessionName);
}


void UOSSSessionWidget::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult & InviteResult)
{
	if (!bWasSuccessful)
	{
		UE_LOG_ONLINE(Error, TEXT("Did not successfully invited user to the session!"));
		return;
	}

	UE_LOG_ONLINE(Display, TEXT("Accepted invite to session.  Joining session...."));

	auto OculusSessionInterface = Online::GetSessionInterface();
	OculusSessionInterface->JoinSession(ControllerId, TEXT("Game"), InviteResult);
}

bool UOSSSessionWidget::StartMatchmaking(const FString& PoolName)
{
	auto OculusSessionInterface = Online::GetSessionInterface();

	UE_LOG_ONLINE(Display, TEXT("Starting Matchmaking"));
	TArray< TSharedRef<const FUniqueNetId> > LocalPlayers;

	// Create a matchmaking for two people
	auto SessionSettings = new FOnlineSessionSettings();
	SessionSettings->NumPublicConnections = 2;

	SearchSettings = MakeShareable(new FOnlineSessionSearch());

	// Add the delegate
	if (!OnMatchmakingCompleteDelegate.IsBound())
	{
		OnMatchmakingCompleteDelegate = FOnMatchmakingCompleteDelegate::CreateUObject(this, &UOSSSessionWidget::OnMatchmakingComplete);
		OculusSessionInterface->AddOnMatchmakingCompleteDelegate_Handle(OnMatchmakingCompleteDelegate);
	}

	// Search with this poolname
	SearchSettings->QuerySettings.Set(FName(TEXT("OCULUSPOOL")), PoolName, EOnlineComparisonOp::Equals);

	TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SearchSettings.ToSharedRef();

	// Do the search 
	return OculusSessionInterface->StartMatchmaking(LocalPlayers, TEXT("Game"), *SessionSettings, SearchSettingsRef);
}


bool UOSSSessionWidget::CancelMatchmaking(FName SessionName)
{
	auto OculusSessionInterface = Online::GetSessionInterface();

	// Add the delegate
	if (!OnCancelMatchmakingCompleteDelegate.IsBound())
	{
		OnCancelMatchmakingCompleteDelegate = FOnCancelMatchmakingCompleteDelegate::CreateUObject(this, &UOSSSessionWidget::OnCancelMatchmakingComplete);
		OculusSessionInterface->AddOnCancelMatchmakingCompleteDelegate_Handle(OnCancelMatchmakingCompleteDelegate);
	}

	UE_LOG_ONLINE(Display, TEXT("Cancelling Matchmaking"));
	return OculusSessionInterface->CancelMatchmaking(0, TEXT("Game"));

}

void UOSSSessionWidget::OnCancelMatchmakingComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG_ONLINE(Display, TEXT("Matchmaking Cancel returned: %s"), bWasSuccessful?TEXT("true"):TEXT("false"));
	OnCancelMatchmakingCompleteBP(SessionName, bWasSuccessful);

}


void UOSSSessionWidget::OnMatchmakingComplete(FName SessionName, bool bWasSuccessful)
{
	if (!(bWasSuccessful && SearchSettings->SearchResults.Num() > 0))
	{
		UE_LOG_ONLINE(Error, TEXT("Did not successfully find a matchmaking session!"));
		return;
	}

	UE_LOG_ONLINE(Display, TEXT("Found a matchmaking session.  Joining session...."));

	auto OculusSessionInterface = Online::GetSessionInterface();
	OculusSessionInterface->JoinSession(0, SessionName, SearchSettings->SearchResults[0]);
	OnMatchmakingCompleteBP(SessionName, bWasSuccessful);  //tell the blueprint Matchmaking completed
}


int32 UOSSSessionWidget::RefreshPlayerCountInSession()
{
	auto OculusSessionInterface = Online::GetSessionInterface();

	if (!OculusSessionInterface.IsValid())
	{
		return 0;
	}

	auto Session = OculusSessionInterface->GetNamedSession(TEXT("Game"));

	if (!Session)
	{
		return 0;
	}

	if(Session->RegisteredPlayers.Num() != LastPlayerCount)
	{
		Session->RegisteredPlayers.Num() > LastPlayerCount ? OnSessionPlayerCountChanged(true) : OnSessionPlayerCountChanged(false);
		UE_LOG_ONLINE(Display, TEXT("Change in number of players in session!"));
		LastPlayerCount = Session->RegisteredPlayers.Num();
	}
	return Session->RegisteredPlayers.Num();
}

//Blueprint callable function to check the state of the session and update the BP property that is
//tied to the UI item
void UOSSSessionWidget::UpdateSessionState(FName SessionName)
{
	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(SessionName);

	if (Session)
	{
		SessionState = EOnlineSessionState::ToString(Session->SessionState);
		OwningUserNameBP = *Session->OwningUserName;
	}
	else
	{
		SessionState = TEXT("NoSession");
		OwningUserNameBP = TEXT("None");
	}
}

void UOSSSessionWidget::StartVoip(FName SessionName)
{
	auto OculusSessionInterface = Online::GetSessionInterface();
	auto OculusVoiceInterface = Online::GetVoiceInterface();
	auto OculusIdentityInterface = Online::GetIdentityInterface();

	auto Session = OculusSessionInterface->GetNamedSession(SessionName);
	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);

	if (Session)
	{
		OculusSessionInterface->StartSession(SessionName);//

		auto RegisteredPlayers = Session->RegisteredPlayers; //get list of players in the session

		for (auto RegisteredPlayer : RegisteredPlayers)
		{
			//don't register the local player, only the remote
			if (RegisteredPlayer.Get() != *UserId.Get())
			{
				OculusVoiceInterface->RegisterRemoteTalker(RegisteredPlayer.Get());
				OculusVoiceInterface->StartNetworkedVoice(0);
				UE_LOG_ONLINE(Display, TEXT("Registered a Talker: %s"), *RegisteredPlayer.Get().ToString());
			}
		}
	}
}

void UOSSSessionWidget::StopVoip(FName SessionName)
{
	auto OculusSessionInterface = Online::GetSessionInterface();
	auto OculusVoiceInterface = Online::GetVoiceInterface();
	auto OculusIdentityInterface = Online::GetIdentityInterface();

	auto Session = OculusSessionInterface->GetNamedSession(SessionName);
	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);

	if (Session)
	{
		OculusVoiceInterface->StopNetworkedVoice(0);
		OculusVoiceInterface->RemoveAllRemoteTalkers();
		UE_LOG_ONLINE(Display, TEXT("Stopped Talking"));
	}
}

//function that can check if this user is already in a valid session (like if the scene was loaded 
// after already joining a session or accepting an invite
bool UOSSSessionWidget::InAValidSession()
{
	auto OculusSessionInterface = Online::GetSessionInterface();

	if (!OculusSessionInterface.IsValid())
	{
		return false;
	}

	auto Session = OculusSessionInterface->GetNamedSession(TEXT("Game"));
	if (!Session)
	{
		return false;
	}

	UE_LOG_ONLINE(Display, TEXT("InAValidSession returned: %s"), ((Session->SessionState != EOnlineSessionState::NoSession) ? TEXT("true") : TEXT("false")));
	return (Session->SessionState != EOnlineSessionState::NoSession);
}
