// Copyright 2016 Oculus VR, LLC All Rights reserved.

#include "OculusPlatformSample.h"
#include "OSSAchievementWidget.h"



void UOSSAchievementWidget::ReadAchievements()
{
	auto OculusIdentityInterface = Online::GetIdentityInterface();
	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);
	UE_LOG_ONLINE(Display, TEXT("Achievements:  Got UserId"));
	AchievementStatusText = TEXT("Getting UserID");

	auto OculusAchievementInterface = Online::GetAchievementsInterface();
	UE_LOG_ONLINE(Display, TEXT("Achievements:  Calling QueryAchievementDescriptions()"));
	AchievementStatusText = TEXT("Requesting Achievement Descriptions");
	OculusAchievementInterface->QueryAchievementDescriptions(
		*UserId.Get(),
		FOnQueryAchievementsCompleteDelegate::CreateUObject(this, &UOSSAchievementWidget::OnQueryAchievementDescriptionsComplete)
	);
}

void UOSSAchievementWidget::OnQueryAchievementDescriptionsComplete(const FUniqueNetId& PlayerId, const bool bWasSuccessful)
{
	UE_LOG_ONLINE(Display, TEXT("Got results back from achievement definitions"));
	AchievementStatusText = TEXT("Got Achievemtne Descriptions");

	if (bWasSuccessful)
	{
		UE_LOG_ONLINE(Display, TEXT("Achievement descriptions loaded for: %s"), *PlayerId.ToString());
		auto OculusAchievementInterface = Online::GetAchievementsInterface();

		UE_LOG_ONLINE(Display, TEXT("Trying to load player achievements from server...."));
		AchievementStatusText = TEXT("Requesting Achievement list");
		OculusAchievementInterface->QueryAchievements(
			PlayerId,
			FOnQueryAchievementsCompleteDelegate::CreateUObject(this, &UOSSAchievementWidget::OnQueryAchievementsComplete)
		);
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("Achievement descriptions could not be loaded!"));
		AchievementStatusText = TEXT("Achievement Descriptions could not be loaded");
	}
}


void UOSSAchievementWidget::OnQueryAchievementsComplete(const FUniqueNetId& PlayerId, const bool bWasSuccessful)
{
	UE_LOG_ONLINE(Display, TEXT("Got results back for player achievements"));
	AchievementStatusText = TEXT("Got a response list back from server");
	TArray<FOnlineAchievement> AchievementArray;

	if (bWasSuccessful)
	{
		auto OculusAchievementInterface = Online::GetAchievementsInterface();
		OculusAchievementInterface->GetCachedAchievements(PlayerId, AchievementArray);

		for (auto Cheevo : AchievementArray)
		{
			AddAchievementToList(Cheevo.Id);
		}

		UE_LOG_ONLINE(Display, TEXT("PlayerAchievements loaded"));
		AchievementStatusText = TEXT("Receieved the list of achievements");

		//fire a BP event
		OnQueryAchievementsCompleteBP();
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("Achievements could not be loaded!"));
	}
}

void UOSSAchievementWidget::OnAchievementsWritten(const FUniqueNetId& PlayerId, const bool bWasSuccessful)
{
	UE_LOG_ONLINE(Display, TEXT("Got results back from achievement write"));

	if (bWasSuccessful)
	{
		UE_LOG_ONLINE(Display, TEXT("Achievements written successfully!"));
		OnAchievementsUpdated(TEXT("Achievements written successfully!"));
		//Fire a BP event 
		OnAchievementsWrittenBP();
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("Achievements could not be written!"));
		OnAchievementsUpdated(TEXT("Achievements could not be written!"));
	}
}

void UOSSAchievementWidget::ShowAchievementInfo(const FString& AchievementName)
{
	auto OculusIdentityInterface = Online::GetIdentityInterface();
	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);

	auto OculusAchievementInterface = Online::GetAchievementsInterface();

	AchievementStatusText = TEXT("Showing info for achivement named: ") + AchievementName;

	FOnlineAchievement Achievement;
	FOnlineAchievementDesc AchievementDesc;
	UE_LOG_ONLINE(Display, TEXT("Trying to get cached achievement: %s"), *AchievementName);
	OculusAchievementInterface->GetCachedAchievement(*UserId.Get(), AchievementName, Achievement);
	OculusAchievementInterface->GetCachedAchievementDescription(AchievementName, AchievementDesc);

	AchievementProgressText = AchievementStatusText.SanitizeFloat(Achievement.Progress);
	AchievementNameText = *Achievement.Id;

	UE_LOG_ONLINE(Display, TEXT("Got  Achievement: %s, Progress: %f"), *Achievement.Id, Achievement.Progress);
}

void UOSSAchievementWidget::UnlockAchievement(const FString& AchievementName)
{
	auto OculusAchievementInterface = Online::GetAchievementsInterface();
	auto OculusIdentityInterface = Online::GetIdentityInterface();
	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);

	FOnlineAchievement Achievement;
	FOnlineAchievementDesc AchievementDesc;
	Achievement.Id = TEXT(""); //empty
	UE_LOG_ONLINE(Display, TEXT("Trying to get cached achievement: %s"), *AchievementName);
	AchievementStatusText = TEXT("Trying to get cached achievement: ") + AchievementName;
	OculusAchievementInterface->GetCachedAchievement(*UserId.Get(), AchievementName, Achievement);
	OculusAchievementInterface->GetCachedAchievementDescription(AchievementName, AchievementDesc);
	if (!Achievement.Id.IsEmpty())
	{
		UE_LOG_ONLINE(Display, TEXT("Trying to update player achievements to server...."));
		FOnlineAchievementsWritePtr WriteObject = MakeShareable(new FOnlineAchievementsWrite());
		(Achievement.Progress > 0) ? WriteObject->SetIntStat(*AchievementName, 0) : WriteObject->SetIntStat(*AchievementName, 1); //toggle the acheievement
		FOnlineAchievementsWriteRef WriteObjectRef = WriteObject.ToSharedRef();
		OculusAchievementInterface->WriteAchievements(
			*UserId.Get(),
			WriteObjectRef,
			FOnAchievementsWrittenDelegate::CreateUObject(this, &UOSSAchievementWidget::OnAchievementsWritten)
		);
	}

}
