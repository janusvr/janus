// Copyright 2016 Oculus VR, LLC All Rights reserved.

#pragma once

#include "Online.h"
#include "Blueprint/UserWidget.h"
#include "OSSFriendListWidget.generated.h"


/**
 * 
 */
UCLASS()
class OCULUSPLATFORMSAMPLE_API UOSSFriendListWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:
	FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;
	FOnFindFriendSessionCompleteDelegate OnFindFriendSessionCompleteDelegate;
	TMap<FString, FOnlineSessionSearchResult> FriendsSessions;
	TMap<FString, TSharedRef<const FUniqueNetId>> FriendsToInvite;
	TMap<FString, TSharedRef<const FUniqueNetId>> FriendsAll;


	
public:

	UOSSFriendListWidget(const FObjectInitializer& ObjectInitializer) :
		UUserWidget(ObjectInitializer)
	{
		if (IsRunningCommandlet())
		{
			FModuleManager::Get().LoadModule(TEXT("OnlineSubsystem"));
		}

	}

	UFUNCTION(BlueprintCallable, Category = OculusFriends)
		void ReadAllFriends();

	UFUNCTION(BlueprintCallable, Category = OculusFriends)
		void ReadOnlineFriends();

	UFUNCTION(BlueprintCallable, Category = OculusFriends)
		void ShowFriendInfo(const FString& FriendName);

	void OnReadFriendsListComplete(int32 LocalUserNum, const bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusFriends)
		void AddFriendToList(const FString& FriendName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusFriends)
		void AddFriendToInviteList(const FString& FriendName);

	UFUNCTION(BlueprintCallable, Category = OculusFriends)
		void InviteFriendToSession(const FString& FriendName);

	void OnFindFriendSessionComplete(int32 LocalUserNum, const bool bWasSuccessful, const FOnlineSessionSearchResult& FriendSearchResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
		FString FriendDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
		FString FriendSessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
		FString FriendOnlineStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
		FString FriendPlayStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
		FString FriendSameGameStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
		FString FriendJoinableStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
		FString FriendVoiceStatus;

};
