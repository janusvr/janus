// Copyright 2016 Oculus VR, LLC All Rights reserved.

#pragma once

#include "Online.h"
#include "Blueprint/UserWidget.h"
#include "OSSAchievementWidget.generated.h"



UCLASS()
class OCULUSPLATFORMSAMPLE_API UOSSAchievementWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
private:


public:

	UOSSAchievementWidget(const FObjectInitializer& ObjectInitializer) :
		UUserWidget(ObjectInitializer)
	{
		if (IsRunningCommandlet())
		{
			FModuleManager::Get().LoadModule(TEXT("OnlineSubsystem"));
		}

	}

	UFUNCTION(BlueprintCallable, Category = Achievement)
		void ReadAchievements();

	UFUNCTION(BlueprintCallable, Category = Achievement)
		void ShowAchievementInfo(const FString& AchievementName);

	UFUNCTION(BlueprintImplementableEvent, Category = Achievement)
		void OnAchievementsUpdated(const FString& AchStatus);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Achievement)
		FString AchievementStatusText;

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void AddAchievementToList(const FString& AchievementName);

	UFUNCTION(BlueprintCallable, Category = Achievement)
		void UnlockAchievement(const FString& AchievementName);

	void OnQueryAchievementDescriptionsComplete(const FUniqueNetId& PlayerId, const bool bWasSuccessful);

	void OnQueryAchievementsComplete(const FUniqueNetId& PlayerId, const bool bWasSuccessful);

	void OnAchievementsWritten(const FUniqueNetId& PlayerId, const bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void OnAchievementsWrittenBP();

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
		void OnQueryAchievementsCompleteBP();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Achievement)
		FString AchievementProgressText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Achievement)
		FString AchievementNameText;
	
};
