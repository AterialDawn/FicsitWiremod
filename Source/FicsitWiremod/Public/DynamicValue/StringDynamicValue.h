﻿// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DynamicValueBase.h"
#include "UObject/Object.h"
#include "StringDynamicValue.generated.h"

/**
 * 
 */
UCLASS(Blueprintable,NonTransient)
class FICSITWIREMOD_API UStringDynamicValue : public UDynamicValueBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, SaveGame,Replicated)
	FString Value;


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(UStringDynamicValue, Value)
	}

	virtual bool IsSupportedForNetworking() const override{return true;}
};
