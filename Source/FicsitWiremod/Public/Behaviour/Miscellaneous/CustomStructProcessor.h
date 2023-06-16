﻿// 

#pragma once

#include "CoreMinimal.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "CommonLib/DynamicValues/CCDynamicValueUtils.h"
#include "CustomStructProcessor.generated.h"


UCLASS()
class UCustomStructClipboard : public UFGFactoryClipboardSettings
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FCustomStruct Value;
};

UCLASS()
class FICSITWIREMOD_API ACustomStructProcessor : public AFGWiremodBuildable
{
	GENERATED_BODY()

public:
	virtual void Process_Implementation(float DeltaTime) override
	{
		for(int i = 0; i < Out.Values.Num(); i++)
		{
			if(!IsConnected(i)) continue;

			auto Value = Out.Values[i].Value;
			//Since all inputs will be disconnected when we change the structure, we don't have to check if the connection types match
			//The only case where that could happen is if someone edited their save data in which case it's their own fault if something crashes
			Out.Values[i].Value = UCCDynamicValueUtils::FromValue(GetConnection(i), Value ? (UObject*) Value : (UObject*) this);
		}
	}

	virtual bool CanUseFactoryClipboard_Implementation() override { return true; }
	virtual TSubclassOf<UObject> GetClipboardMappingClass_Implementation() override { return StaticClass(); }
	virtual TSubclassOf<UFGFactoryClipboardSettings> GetClipboardSettingsClass_Implementation() override { return UCustomStructClipboard::StaticClass(); }
	virtual UFGFactoryClipboardSettings* CopySettings_Implementation() override
	{
		auto Val = NewObject<UCustomStructClipboard>();
		Val->Value = Out;
		return Val;
	}

	virtual bool PasteSettings_Implementation(UFGFactoryClipboardSettings* factoryClipboard) override
	{
		auto Val = Cast<UCustomStructClipboard>(factoryClipboard);
		SetCustomStruct_Internal(Val->Value);
		return true;
	}

	UFUNCTION(BlueprintCallable)
	void SetCustomStruct(const FCustomStruct& NewStruct, UObject* Setter)
	{
		if(GetCanConfigure(Setter))
			SetCustomStruct_Internal(NewStruct);
	}

	void SetCustomStruct_Internal(const FCustomStruct& NewStruct)
	{
		//Disconnect all inputs
		OnInputDisconnected_Internal(-1);
		Out = NewStruct;
		Recompile();
	}

	UFUNCTION(BlueprintPure)
	bool RequiresRecompile(const FCustomStruct& CompareTo){ return Out != CompareTo; }

	void Recompile()
	{
		ConnectionsInfo.Inputs.Empty();

		for(auto Field : Out.Values)
		{
			auto Input = FBuildingConnection(Field.Name, "", Field.Value->ConnectionType);
			ConnectionsInfo.Inputs.Add(Input);
		}
	}


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(ACustomStructProcessor, Out)
	}
	
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override
	{
		bool Idk = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

		for(auto Val : Out.Values)
			Channel->ReplicateSubobject(Val.Value, *Bunch, *RepFlags);

		return Idk;
	}

	UPROPERTY(SaveGame, BlueprintReadWrite, Replicated)
	FCustomStruct Out;
};
