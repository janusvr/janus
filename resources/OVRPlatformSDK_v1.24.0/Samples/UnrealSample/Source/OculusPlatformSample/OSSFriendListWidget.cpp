// Copyright 2016 Oculus VR, LLC All Rights reserved.

#include "OculusPlatformSample.h"
#include "OSSFriendListWidget.h"




void UOSSFriendListWidget::ReadAllFriends()
{
	auto OculusFriendsInterface = Online::GetFriendsInterface();

	UE_LOG_ONLINE(Display, TEXT("Trying to get friends list from server"));

	OculusFriendsInterface->ReadFriendsList(
		0,
		TEXT("default"),
		FOnReadFriendsListComplete::CreateUObject(this, &UOSSFriendListWidget::OnReadFriendsListComplete)
	);
}

void UOSSFriendListWidget::ReadOnlineFriends()
{
	auto OculusFriendsInterface = Online::GetFriendsInterface();

	UE_LOG_ONLINE(Display, TEXT("Trying to get friends list from server"));

	OculusFriendsInterface->ReadFriendsList(
		0,
		TEXT("onlinePlayers"),
		FOnReadFriendsListComplete::CreateUObject(this, &UOSSFriendListWidget::OnReadFriendsListComplete)
	);
}

void UOSSFriendListWidget::ShowFriendInfo(const FString& FriendName)
{
	auto OculusFriendsInterface = Online::GetFriendsInterface();
	UE_LOG_ONLINE(Display, TEXT("Displaying friend info"));

	if (FriendsAll.Contains(FriendName))
	{
		auto Friend = OculusFriendsInterface->GetFriend(0, FriendsAll[FriendName].Get(),TEXT("default"));
		if (Friend.IsValid())
		{
			FriendDisplayName = Friend->GetDisplayName();
			FriendSessionID = Friend->GetPresence().SessionId.IsValid() ? Friend->GetPresence().SessionId->ToString() : TEXT("None");
			FriendOnlineStatus = Friend->GetPresence().bIsOnline == 1 ? TEXT("Yes") : TEXT("No");
			FriendPlayStatus = Friend->GetPresence().bIsPlaying == 1 ? TEXT("Yes") : TEXT("No");
			FriendSameGameStatus = Friend->GetPresence().bIsPlayingThisGame == 1 ? TEXT("Yes") : TEXT("No");
			FriendJoinableStatus = Friend->GetPresence().bIsJoinable == 1 ? TEXT("Yes") : TEXT("No");
			FriendVoiceStatus = Friend->GetPresence().bHasVoiceSupport == 1 ? TEXT("Yes") : TEXT("No");
		}
	}
}

void UOSSFriendListWidget::OnReadFriendsListComplete(int32 LocalUserNum, const bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	UE_LOG_ONLINE(Display, TEXT("Got results back from reading friends list"));
	if (bWasSuccessful)
	{
		auto OculusFriendsInterface = Online::GetFriendsInterface();
		TArray<TSharedRef<FOnlineFriend>> Friends;
		OculusFriendsInterface->GetFriendsList(0, ListName, Friends);
		UE_LOG_ONLINE(Display, TEXT("Online Friends list loaded.  Count of friends: %d"), Friends.Num());

		for (auto Friend : Friends)
		{
			if (ListName.Equals(TEXT("onlinePlayers"), ESearchCase::IgnoreCase))
			{
				AddFriendToInviteList(Friend->GetDisplayName());
				FriendsToInvite.Add(Friend->GetDisplayName(), Friend->GetUserId());
			}
			else
			{
				AddFriendToList(Friend->GetDisplayName());
				FriendsAll.Add(Friend->GetDisplayName(), Friend->GetUserId());
			}
		}
	}
	else
	{
		UE_LOG_ONLINE(Error, TEXT("%s"), *ErrorStr);
	}
}

void UOSSFriendListWidget::OnFindFriendSessionComplete(int32 LocalUserNum, const bool bWasSuccessful, const FOnlineSessionSearchResult& FriendSearchResult)
{
	UE_LOG_ONLINE(Display, TEXT("Got result if friend is in a session"));
	if (bWasSuccessful)
	{
		if (FriendSearchResult.IsValid())
		{
			UE_LOG_ONLINE(Display, TEXT("Search result is valid.  Adding to the list"));
			AddFriendToList(FriendSearchResult.Session.OwningUserName);
			FriendsSessions.Add(FriendSearchResult.Session.OwningUserName, FriendSearchResult);
			return;
		}
	}
}

void UOSSFriendListWidget::InviteFriendToSession(const FString& FriendName)
{
	UE_LOG_ONLINE(Display, TEXT("Trying to invite %s"), *FriendName);

}
