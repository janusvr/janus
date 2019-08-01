// Copyright 2016 Oculus VR, LLC All Rights reserved.

#include "OculusPlatformSample.h"
#include "OSSOculusActor.h"

// Sets default values
AOSSOculusActor::AOSSOculusActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AOSSOculusActor::BeginPlay()
{
	Super::BeginPlay();

	auto OculusIdentityInterface = Online::GetIdentityInterface();
	if (!OculusIdentityInterface.IsValid())
	{
		UE_LOG_ONLINE(Error, TEXT("No OculusIdentityInterface found!"));
		return;
	}

	OnLoginCompleteDelegateHandle = OculusIdentityInterface->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateUObject(this, &AOSSOculusActor::OnLoginComplete));
	if (OculusIdentityInterface->AutoLogin(0))
	{
		UE_LOG_ONLINE(Display, TEXT("Waiting for login response from oculus...."));
	}
}

// Called every frame
void AOSSOculusActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
