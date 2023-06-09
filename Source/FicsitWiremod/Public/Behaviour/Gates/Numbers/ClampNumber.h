﻿// 

#pragma once

#include "CoreMinimal.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "ClampNumber.generated.h"

UCLASS()
class FICSITWIREMOD_API AClampNumber : public AFGWiremodBuildable
{
	GENERATED_BODY()

public:
	virtual void Process_Implementation(float DeltaTime) override
	{
		Out = FMath::Clamp(GetConnection(0).GetFloat(), GetConnection(1).GetFloat(), GetConnection(2).GetFloat());
	}

	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(AClampNumber, Out);
	}


	UPROPERTY(Replicated, VisibleInstanceOnly)
	float Out;
};
