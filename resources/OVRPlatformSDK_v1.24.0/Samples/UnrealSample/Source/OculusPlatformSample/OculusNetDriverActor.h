// Copyright 2016 Oculus VR, LLC All Rights reserved.

#pragma once

#include "OSSOculusActor.h"
#include "OculusNetDriverActor.generated.h"

UCLASS()
class OCULUSPLATFORMSAMPLE_API AOculusNetDriverActor : public AOSSOculusActor
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, Category = Network)
	FString NetModeStatus;

	UPROPERTY(BlueprintReadWrite, Category = Network)
	FString NetDriverStatus;

	UFUNCTION(BlueprintImplementableEvent, Category = Identity)
	void OnPlayerIDUpdate(const FString& PlayerID);

	UFUNCTION(BlueprintImplementableEvent, Category = Identity)
	void SendPlayerChat(const FString& PlayerID);

	FString PlayerID;

	virtual void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error) override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
};
