﻿// 

#pragma once

#include "CoreMinimal.h"
#include "Behaviour/MultistateWiremodBuildable.h"
#include "NOR.generated.h"

UCLASS()
class FICSITWIREMOD_API ANOR : public AMultistateWiremodBuildable
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(ANOR, Out)
	};

	virtual void Process_Implementation(float DeltaTime) override
	{
		//Default state - N+ inputs, 1 output
		if(CurrentStateIndex == 0)
		{
			TArray<FNewConnectionData> Connected;
			GetAllConnected(Connected);
		
			for (auto Data : Connected)
			{
				if(WM::GetFunctionBoolResult(Data))
				{
					Out = false;
					return;
				}
			}
			Out = true;	
		}
		//Alternate state - 1 bool array input, 1 output
		else if(CurrentStateIndex == 1)
		{
			auto Array = WM::GetBoolArray(GetConnection(0));

			for (auto Element : Array)
			{
				if(Element)
				{
					Out = false;
					return;
				}
			}
			Out = true;
		}
	}
	
	UPROPERTY(Replicated, VisibleInstanceOnly)
	bool Out;
};
