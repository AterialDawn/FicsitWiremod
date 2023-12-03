﻿// 

#pragma once

#include "CoreMinimal.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "CommonLib/BackwardsCompatibilityHandler.h"
#include "CommonLib/PlayerOwnedClipboardData.h"
#include "CommonLib/DynamicValues/CCCustomStructValue.h"
#include "Kismet/GameplayStatics.h"
#include "ConfigurableConstant.generated.h"

UCLASS()
class UConstantClipboardData : public UPlayerOwnedClipboardData
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<FNamedDynamicValue> Values;
};



UCLASS()
class FICSITWIREMOD_API AConfigurableConstant : public AFGWiremodBuildable, public IDynamicValuePasser
{
	GENERATED_BODY()

public:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(AConfigurableConstant, SavedValues)
	}

	virtual void GatherReplicatedObjects_Implementation(TArray<UObject*>& OutObjects) override
	{
		Super::GatherReplicatedObjects_Implementation(OutObjects);
		for(auto Val : SavedValues) OutObjects.Add(Val.Value);
	}
	
	UFUNCTION(BlueprintCallable)
	FNamedDynamicValue FindValue(FString Name, bool AllowCached = true)
	{
		if(AllowCached && CachedValues.Contains(Name))
			return FNamedDynamicValue(Name, CachedValues[Name]);

		for (FNamedDynamicValue Value : SavedValues)
		{
			if(Value.Name == Name)
			{
				CachedValues.Add(Name, Value.Value);
				return Value;
			}
		}

		return FNamedDynamicValue();
	}

	virtual bool CanUseFactoryClipboard_Implementation() override{ return true; }
	virtual TSubclassOf<UFGFactoryClipboardSettings> GetClipboardSettingsClass_Implementation() override { return UConstantClipboardData::StaticClass(); }
	virtual TSubclassOf<UObject> GetClipboardMappingClass_Implementation() override { return StaticClass(); }
	virtual UFGFactoryClipboardSettings* CopySettings_Implementation() override
	{
		auto Settings = NewObject<UConstantClipboardData>(this);
		Settings->Player = UGameplayStatics::GetPlayerController(this, 0);
		Settings->Values = SavedValues;

		return Settings;
	}
	virtual bool PasteSettings_Implementation(UFGFactoryClipboardSettings* factoryClipboard) override
	{
		auto Settings = Cast<UConstantClipboardData>(factoryClipboard);
		SavedValues = Settings->Values;
		OnRep_ValuesUpdated();
		return true;
	}

	
	virtual UObject* GetValue_Implementation(const class FString& ValueName) override
	{
		return FindValue(ValueName);
	}

	virtual TArray<FBuildingConnection> GetConnectionsInfo_Implementation(EConnectionDirection direction, int& Count, FBuildableNote& Note) override
	{
		if(direction == Input) return TArray<FBuildingConnection>();
		
		TArray<FBuildingConnection> Out;
		for (FNamedDynamicValue Value : SavedValues)
		{
			auto Connection = FBuildingConnection(Value.Name, Value.Name, Value ? Value.Value->ConnectionType.GetValue() : Unknown);
			Out.Add(Connection);
		}

		Count = Out.Num();
		return Out;
	}



	UFUNCTION()
	void OnRep_ValuesUpdated()
	{
		CachedValues.Empty();
	}

	UFUNCTION(BlueprintCallable)
	void RegisterValues(const TArray<FNamedValue>& NewValues)
	{
		SavedValues.Empty();
		for(auto Val : NewValues)
			SavedValues.Add(FNamedDynamicValue(Val.Name, Val.Value.Convert(this)));
		
		OnRep_ValuesUpdated();
	}

	UFUNCTION()
	void netFunc_setBoolValue(FString Name, bool Value) { CreateNewOrUpdate(FNamedDynamicValue(Name, FDynamicValue(Value).Convert(this))); }

	UFUNCTION()
	void netFunc_setStringValue(FString Name, FString Value) { CreateNewOrUpdate(FNamedDynamicValue(Name, FDynamicValue(Value).Convert(this))); }

	UFUNCTION()
	void netFunc_setFloatValue(FString Name, double Value) { CreateNewOrUpdate(FNamedDynamicValue(Name, FDynamicValue(Value).Convert(this))); }

	UFUNCTION()
	void netFunc_setColorValue(FString Name, FLinearColor Value) { CreateNewOrUpdate(FNamedDynamicValue(Name, FDynamicValue(Value).Convert(this))); }

	UFUNCTION()
	void netFunc_setVectorValue(FString Name, FVector Value) { CreateNewOrUpdate(FNamedDynamicValue(Name, FDynamicValue(Value).Convert(this))); }



	UFUNCTION()
	void CreateNewOrUpdate(FNamedDynamicValue NamedValue)
	{
		//Try to find the existing value and change it
		for (auto& StoredValue : SavedValues)
		{
			if(StoredValue.Name == NamedValue.Name)
			{
				StoredValue.Value = NamedValue.Value;
				OnRep_ValuesUpdated();
				return;
			}
		}

		//If value wasn't found, create a new one.
		SavedValues.Add(NamedValue);
		OnRep_ValuesUpdated();
	}

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, SaveGame, ReplicatedUsing=OnRep_ValuesUpdated)
	TArray<FNamedDynamicValue> SavedValues;
	
	UPROPERTY(EditInstanceOnly)
	TMap<FString, UCCDynamicValueBase*> CachedValues;
};
