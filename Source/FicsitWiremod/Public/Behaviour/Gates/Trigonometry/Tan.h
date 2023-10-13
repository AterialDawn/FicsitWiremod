﻿// 

#pragma once

#include "CoreMinimal.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "Behaviour/MultistateWiremodBuildable.h"
#include "Tan.generated.h"

UCLASS()
class FICSITWIREMOD_API ATan : public AMultistateWiremodBuildable
{
	GENERATED_BODY()
        
public:
	virtual void ServerProcess_Implementation(double DeltaTime) override
	{
		// 0 - Degrees
		if(CurrentStateIndex == 0) Out = UKismetMathLibrary::DegTan(GetConnection(0).GetFloat());
		// 1 - Radians
		else Out = UKismetMathLibrary::Tan(GetConnection(0).GetFloat());
	}
        
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
        
		DOREPLIFETIME(ATan, Out);
	}
	
	UPROPERTY(Replicated, VisibleInstanceOnly)
	double Out;
};
