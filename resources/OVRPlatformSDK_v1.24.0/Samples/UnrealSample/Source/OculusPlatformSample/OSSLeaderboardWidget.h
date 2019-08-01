// Copyright 2016 Oculus VR, LLC All Rights reserved.

#pragma once

#include "Online.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "OSSLeaderboardWidget.generated.h"

/**
 * 
 */


class TestLeaderboardRead : public FOnlineLeaderboardRead
{
public:
	TestLeaderboardRead()
	{
		// Default properties
		LeaderboardName = FName(TEXT("TIMES_OPENED_GAME"));
		SortedColumn = "TIMES_VISITED";

		// Define default columns
		new (ColumnMetadata) FColumnMetaData("TIMES_VISITED", EOnlineKeyValuePairDataType::Int32);
	}
};

UCLASS()
class OCULUSPLATFORMSAMPLE_API UOSSLeaderboardWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:
	FOnLeaderboardReadCompleteDelegate OnLeaderboardReadCompleteDelegate;
	FOnlineLeaderboardRead* LeaderboardRead;
	


public:
	UOSSLeaderboardWidget(const FObjectInitializer& ObjectInitializer) :
		UUserWidget(ObjectInitializer)
	{
		if (IsRunningCommandlet())
		{
			FModuleManager::Get().LoadModule(TEXT("OnlineSubsystem"));
		}

	}

	UFUNCTION(BlueprintCallable, Category = OculusLeaderboards)
		void SetupLB_UI();

	UFUNCTION(BlueprintCallable, Category = OculusLeaderboards)
		void ReadLeaderboardData(const FName& LBName);

	UFUNCTION(BlueprintCallable, Category = OculusLeaderboards)
		void WriteLeaderboardData(const FName& LBName, int32 ValueToWrite);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void AddLeaderBoardButton(const FString& LBName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void AddLeaderboardRowToUI(const FString& PlayerID, const FString& NickName, const FString& Rank, const FString& Value);

	void OnReadFriendsListComplete(int32 LocalUserNum, const bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	void OnLeaderboardReadComplete(bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void OnLeaderboardReadCompleteBP(bool bWasSuccessful, const FName& LeaderboardName);
	
};
