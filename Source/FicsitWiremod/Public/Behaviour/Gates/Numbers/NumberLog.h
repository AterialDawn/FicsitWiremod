﻿// 

#pragma once

#include "CoreMinimal.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "NumberLog.generated.h"

UCLASS()
class FICSITWIREMOD_API ANumberLog : public AFGWiremodBuildable
{
	GENERATED_BODY()

public:
	virtual void Process_Implementation(float DeltaTime) override
	{
		Out = UKismetMathLibrary::Log(GetConnection(0).GetFloat(), GetConnection(1).GetFloat());
	}

	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(ANumberLog, Out);
	}


	UPROPERTY(Replicated, VisibleInstanceOnly)
	float Out;
};
