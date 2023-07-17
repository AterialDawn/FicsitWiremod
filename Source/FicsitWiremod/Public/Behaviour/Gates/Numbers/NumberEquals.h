﻿// 

#pragma once

#include "CoreMinimal.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "NumberEquals.generated.h"

UCLASS()
class FICSITWIREMOD_API ANumberEquals : public AFGWiremodBuildable
{
	GENERATED_BODY()

public:
	virtual void Process_Implementation(double DeltaTime) override
	{
		Out = GetConnection(0).GetFloat() == GetConnection(1).GetFloat();
	}

	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(ANumberEquals, Out);
	}


	UPROPERTY(Replicated, VisibleInstanceOnly)
	bool Out;
};
