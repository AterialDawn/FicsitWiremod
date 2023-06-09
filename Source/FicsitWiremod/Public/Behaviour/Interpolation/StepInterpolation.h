﻿// 

#pragma once

#include "CoreMinimal.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "StepInterpolation.generated.h"

UCLASS()
class FICSITWIREMOD_API AStepInterpolation : public AFGWiremodBuildable
{
	GENERATED_BODY()

public:
	virtual void Process_Implementation(float DeltaTime) override
	{
		float A = GetConnection(0).GetFloat();;
		float B = GetConnection(1).GetFloat();;
		float Alpha = GetConnection(2).GetFloat();;
		int Steps = GetConnection(3).GetFloat();
		
		Out = FMath::InterpStep(A, B, Alpha, Steps);
	}
	
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		DOREPLIFETIME(AStepInterpolation, Out);
	}

	UPROPERTY(Replicated, VisibleInstanceOnly)
	float Out;
};
