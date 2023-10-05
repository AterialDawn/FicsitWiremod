﻿// 

#pragma once

#include "CoreMinimal.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "Components/SignComponentBase.h"
#include "Components/SignComponentDescriptor.h"
#include "ManagedSign.generated.h"

USTRUCT(BlueprintType)
struct FManagedSignData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FVector2D Size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FSignComponentData> Components;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FManagedSignConnectionSettings> Connections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FSignComponentVariableData> Variables;

	bool operator==(const FManagedSignData& Other) const
	{
		return Size == Other.Size
		&& Components == Other.Components
		&& Connections == Other.Connections
		&& Variables == Other.Variables;
	}
};

UCLASS()
class FICSITWIREMOD_API AManagedSign : public AFGWiremodBuildable
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override
	{
		Super::BeginPlay();
		OnGenerateSign(Data);
	}
	
	void ApplySignLayout(const FManagedSignData& NewData, UObject* Setter)
	{
		if(!GetCanConfigure(Setter)) return;
		Data = NewData;

		ConnectionsInfo.Inputs.Empty();
		for (auto Connection : Data.Connections)
		{
			ConnectionsInfo.Inputs.Add(FBuildingConnection(Connection.Name, "", Connection.Type.GetValue()));
		}
		
		OnSignDataChanged(NewData);
	}

	virtual void ClientProcess_Implementation(double DeltaTime) override
	{
		for (auto Component : ActiveComponents)
		{
			if(Component) Component->OnUpdate(this);
		}
	}

	UFUNCTION(NetMulticast, Reliable)
	void OnSignDataChanged(const FManagedSignData& NewData);
	void OnSignDataChanged_Implementation(const FManagedSignData& NewData){ OnGenerateSign(NewData); }
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnGenerateSign(const FManagedSignData& NewData);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Replicated)
	FManagedSignData Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USignComponentBase*> ActiveComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USignComponentBase*> StaticComponents;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(AManagedSign, Data)
	}
};


UCLASS()
class UManagedSignUtilityFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, meta=(ScriptOperator="=="), DisplayName="Sign layout equals")
	static bool SignLayoutEquals(const FManagedSignData& Data1, const FManagedSignData& Data2) { return Data1 == Data2; }	
};
