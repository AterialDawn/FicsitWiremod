﻿// 

#pragma once

#include "CoreMinimal.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "VectorAdd.generated.h"

UCLASS()
class FICSITWIREMOD_API AVectorAdd : public AFGWiremodBuildable
{
	GENERATED_BODY()

public:
	virtual void ServerProcess_Implementation(double DeltaTime) override
	{
		Out = GetConnection(0).GetVector() + GetConnection(1).GetVector();
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(AVectorAdd, Out)
	}

	UPROPERTY(Replicated, SaveGame)
	FVector Out;
};
