// Copyright 2016 Oculus VR, LLC All Rights reserved.

#pragma once

#include "Online.h"
#include "OVR_Platform.h"
#include "Blueprint/UserWidget.h"
#include "OSSSessionWidget.generated.h"

/**
 *
 */
UCLASS()
class OCULUSPLATFORMSAMPLE_API UOSSSessionWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:

	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FOnFindFriendSessionCompleteDelegate OnFindFriendSessionCompleteDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;

	FOnMatchmakingCompleteDelegate OnMatchmakingCompleteDelegate;
	FOnCancelMatchmakingCompleteDelegate OnCancelMatchmakingCompleteDelegate;

	TMap<FString, FOnlineSessionSearchResult> FriendsSessions;
	TMap<FString, TSharedRef<const FUniqueNetId>> FriendsToInvite;

	TSharedPtr<FOnlineSessionSearch> SearchSettings;
	int32 LastPlayerCount;
	
public:

	UOSSSessionWidget(const FObjectInitializer& ObjectInitializer) :
		UUserWidget(ObjectInitializer)
	{
		if (IsRunningCommandlet())
		{
			FModuleManager::Get().LoadModule(TEXT("OnlineSubsystem"));
		}

		auto OculusSessionInterface = Online::GetSessionInterface();
		OnSessionUserInviteAcceptedDelegate = FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UOSSSessionWidget::OnSessionUserInviteAccepted);
		OculusSessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(OnSessionUserInviteAcceptedDelegate);

		OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UOSSSessionWidget::OnJoinSessionComplete);
		OculusSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
		LastPlayerCount = 0;
	}

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		void ReadOnlineFriends();

	void OnReadFriendsListComplete(int32 LocalUserNum, const bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void AddFriendToList(const FString& FriendName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void AddFriendToInviteList(const FString& FriendName);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		void InviteFriendToSession(const FString& FriendName);

	//4.15 and earlier:   void OnFindFriendSessionComplete(int32 LocalUserNum, const bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResult);
	void OnFindFriendSessionComplete(int32 LocalUserNum, const bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		void JoinFriendSession(const FString& FriendName);

	// Non-blueprint version because the blueprint cant handle regular enums
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void OnJoinSessionComplete(FName SessionName, bool bIsJoinSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		void CreateSession();

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void OnCreateSessionCompleteBP(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		void StartSession(const FName SessionName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		int32 RefreshPlayerCountInSession();

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		void EndSession(const FName SessionName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		void DestroySession(const FName SessionName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		bool StartMatchmaking(const FString& PoolName);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		bool CancelMatchmaking(const FName SessionName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusSession)
		FString OwningUserNameBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusSession)
		FString SessionState;

	void OnMatchmakingComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void OnMatchmakingCompleteBP(FName SessionName, bool bWasSuccessful);

	void OnCancelMatchmakingComplete(FName SessionName, bool bWasSuccessful);
	
	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void OnCancelMatchmakingCompleteBP(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		void UpdateSessionState(FName SessionName);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		void StartVoip(const FName SessionName);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		void StopVoip(const FName SessionName);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
		bool InAValidSession();

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void OnSessionPlayerCountChanged(bool bIncreased);
};
