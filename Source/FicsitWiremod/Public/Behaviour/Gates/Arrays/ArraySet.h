// 

#pragma once

#include "CoreMinimal.h"
#include "IConstantsDistributor.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "CommonLib/DynamicValues/CCDynamicValueUtils.h"
#include "ArraySet.generated.h"

UCLASS()
class FICSITWIREMOD_API AArraySet : public AFGWiremodBuildable, public IDynamicValuePasser
{
	GENERATED_BODY()
    
public:
	virtual void Process_Implementation(float DeltaTime) override
	{
		const int Index = GetConnection(2).GetFloat();
		
		Out = UCCDynamicValueUtils::FromValue(GetConnection(0), Out ? Out->GetWorld() : this->GetWorld());
		if(auto Array = Cast<UCCArrayValueBase>(Out))
			Array->SetElement(GetConnection(1), Index);
		
		SetOutputType(0, Out ? Out->ConnectionType : Unknown);
	}
    
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
		DOREPLIFETIME(AArraySet, Out);
	}

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override
	{
		bool Idk = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
		Channel->ReplicateSubobject(Out, *Bunch, *RepFlags);

		return Idk;
	}

	virtual UCCDynamicValueBase* GetValue_Implementation(const FString& ValueName) override{ return Out; }

	virtual void OnInputConnected_Implementation(const FConnectionData& Data, int Index, UObject* Setter) override
	{
		//If a new array was connected
		if(Index == 0)
		{
			//Disconnect the previous element input if the array type is not compatible with it.
			if(!UConnectionTypeFunctions::IsValidConnectionPair(GetConnection(0).ConnectionType, Data.ConnectionType))
				OnInputDisconnected_Internal(1);
		}
		Super::OnInputConnected_Implementation(Data, Index, Setter);
		SetInputType(1, UConnectionTypeFunctions::ArrayToBase(Data.ConnectionType));
	}
	
	UPROPERTY(Replicated, VisibleInstanceOnly)
	UCCDynamicValueBase* Out;
};
