// Copyright 2016 Oculus VR, LLC All Rights reserved.

#pragma once

#include "Online.h"
#include "GameFramework/Actor.h"
#include "OSSOculusActor.generated.h"

UCLASS()
class OCULUSPLATFORMSAMPLE_API AOSSOculusActor : public AActor
{
	GENERATED_BODY()

protected:

	FDelegateHandle OnLoginCompleteDelegateHandle;
	
public:	
	// Sets default values for this actor's properties
	AOSSOculusActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error) {}
	
};
