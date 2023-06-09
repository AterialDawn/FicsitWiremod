﻿// 

#pragma once

#include "CoreMinimal.h"
#include "IConstantsDistributor.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "CommonLib/DynamicValues/CCDynamicValueUtils.h"
#include "ArrayClear.generated.h"

UCLASS()
class FICSITWIREMOD_API AArrayClear : public AFGWiremodBuildable, public IDynamicValuePasser
{
	GENERATED_BODY()
    
public:
	virtual void Process_Implementation(float DeltaTime) override
	{
		Out = UCCDynamicValueUtils::FromType(GetConnection(0).ConnectionType, Out ? Out->GetWorld() : this->GetWorld());
		if(auto Array = Cast<UCCArrayValueBase>(Out)) Array->Clear();
		
		SetOutputType(0, Out ? Out->ConnectionType : Unknown);
	}
    
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
		DOREPLIFETIME(AArrayClear, Out);
	}

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override
	{
		bool Idk = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
		Channel->ReplicateSubobject(Out, *Bunch, *RepFlags);

		return Idk;
	}

	virtual UCCDynamicValueBase* GetValue_Implementation(const FString& ValueName) override{ return Out; }
	
	UPROPERTY(Replicated, VisibleInstanceOnly)
	UCCDynamicValueBase* Out;
};
