// Copyright 2016 Oculus VR, LLC All Rights reserved.

#pragma once

#include "Online.h"
#include "Blueprint/UserWidget.h"
#include "OSSMainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class OCULUSPLATFORMSAMPLE_API UOSSMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:

	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

public:
	UOSSMainMenuWidget(const FObjectInitializer& ObjectInitializer) :
		UUserWidget(ObjectInitializer)
	{
		if (IsRunningCommandlet())
		{
			FModuleManager::Get().LoadModule(TEXT("OnlineSubsystem"));
		}

		auto OculusSessionInterface = Online::GetSessionInterface();

		OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UOSSMainMenuWidget::OnJoinSessionComplete);
		OculusSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
	}

	// Non-blueprint version because the blueprint cant handle regular enums
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);
};
