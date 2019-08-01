// Copyright 2016 Oculus VR, LLC All Rights reserved.

#include "OculusPlatformSample.h"
#include "OSSLeaderboardWidget.h"



//Add buttons for each leaderboard.  
void UOSSLeaderboardWidget::SetupLB_UI()
{
	AddLeaderBoardButton(TEXT("lb_test1"));
	AddLeaderBoardButton(TEXT("lb_test2"));
}

void UOSSLeaderboardWidget::ReadLeaderboardData(const FName& LBName)
{
	auto OculusLeaderboardInterface = Online::GetLeaderboardsInterface();
	auto OculusFriendsInterface = Online::GetFriendsInterface();
	auto OculusCloudInterface = Online::GetUserCloudInterface();
	auto OculusIdentityInterface = Online::GetIdentityInterface();

	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);

	UE_LOG_ONLINE(Display, TEXT("Trying to get leaderboard: %s data from server"), *LBName.ToString());
	
	TArray<TSharedRef<const FUniqueNetId>> Players;

	LeaderboardRead = new FOnlineLeaderboardRead();
	auto LeaderboardReadRef = new FOnlineLeaderboardReadRef(LeaderboardRead);

	LeaderboardRead->LeaderboardName = LBName;
	OculusLeaderboardInterface->ReadLeaderboards(Players, *LeaderboardReadRef);

	if (!OnLeaderboardReadCompleteDelegate.IsBound())
	{
		OnLeaderboardReadCompleteDelegate = FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &UOSSLeaderboardWidget::OnLeaderboardReadComplete);
		OculusLeaderboardInterface->AddOnLeaderboardReadCompleteDelegate_Handle(OnLeaderboardReadCompleteDelegate);
	}

	Online::GetLeaderboardsInterface()->FlushLeaderboards(TEXT("Test"));
	
}

void UOSSLeaderboardWidget::OnLeaderboardReadComplete(bool bInIsError)
{

	OnLeaderboardReadCompleteBP(bInIsError, LeaderboardRead->LeaderboardName);//notify the BP that the leaderboard read is complete to clear the UI for new data
	
	FStatsColumnArray ColumnArray;
	UE_LOG_ONLINE(Display, TEXT("Leaderboard Read complete"));
	for (auto row : LeaderboardRead->Rows)
	{
		UE_LOG_ONLINE(Display, TEXT("Found Leaderboard entry: PlayerID: %s, Nickname: %s, Rank: %i, "),*row.PlayerId->ToString(), *row.NickName, row.Rank);
		
		ColumnArray = row.Columns;
		for (auto column : ColumnArray)
		{
			//add data to UI for each entry
			UE_LOG_ONLINE(Display, TEXT("Type: %s, Value: "),column.Value.GetTypeString());
			EOnlineKeyValuePairDataType::Type LBValType = column.Value.GetType();
			UE_LOG_ONLINE(Display, TEXT("Value: %s"), *column.Value.ToString());
			AddLeaderboardRowToUI(*row.PlayerId->ToString(), *row.NickName, FString::FromInt(row.Rank), *column.Value.ToString());
		}
	}
}

void UOSSLeaderboardWidget::WriteLeaderboardData(const FName& LBName, int32 ValueToWrite)
{
	if (LBName.IsNone() && LBName.IsEqual(TEXT("None")))
		return;

	auto OculusLeaderboardInterface = Online::GetLeaderboardsInterface();
	auto OculusFriendsInterface = Online::GetFriendsInterface();
	auto OculusCloudInterface = Online::GetUserCloudInterface();
	auto OculusIdentityInterface = Online::GetIdentityInterface();

	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);

	UE_LOG_ONLINE(Display, TEXT("Trying to write to leaderboard: %s with data: %i"), *LBName.ToString(), ValueToWrite);

	TArray<TSharedRef<const FUniqueNetId>> Players;
	FOnlineLeaderboardWrite LeaderboardWriteObj;
	TArray<FName> LBNames;

	LBNames.Add(LBName);
	LeaderboardWriteObj.LeaderboardNames = LBNames;
	LeaderboardWriteObj.RatedStat = TEXT("SCORE");
	LeaderboardWriteObj.SetIntStat(TEXT("SCORE"), ValueToWrite);
	OculusLeaderboardInterface->WriteLeaderboards(TEXT("test"), *UserId, LeaderboardWriteObj);
}




