﻿// 

#pragma once

#include "CoreMinimal.h"
#include "FGGameState.h"
#include "JsonObjectConverter.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "CommonLib/FileUtilities.h"
#include "CommonLib/PlayerOwnedClipboardData.h"
#include "Components/SignComponentBase.h"
#include "Components/SignComponentDescriptor.h"
#include "ManagedSign.generated.h"

UENUM(Blueprintable, BlueprintType)
enum ESignDataVersion : uint8
{
	Release = 0,
	RendererChange = 1
};

USTRUCT(BlueprintType)
struct FManagedSignData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FVector2D Size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, DisplayName="Screen DPI")
	int DotsPerInch = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	double EmissionLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	double TicksPerSecond = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FSignComponentData> Components;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FManagedSignConnectionSettings> Connections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FSignComponentVariableData> Variables;

	float GetTickInterval() const
	{
		if(TicksPerSecond == 0) return 0;
		else return 1.0 / TicksPerSecond;
	}

	bool operator==(const FManagedSignData& Other) const
	{
		return Size == Other.Size
		&& Components == Other.Components
		&& Connections == Other.Connections
		&& Variables == Other.Variables
		&& DotsPerInch == Other.DotsPerInch
		&& EmissionLevel == Other.EmissionLevel
		&& TicksPerSecond == Other.TicksPerSecond;
	}
};

UCLASS()
class UManagedSignClipboardData : public UPlayerOwnedClipboardData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FManagedSignData Data;
	
};

UCLASS()
class FICSITWIREMOD_API AManagedSign : public AFGWiremodBuildable
{
	GENERATED_BODY()

public:
	virtual void ClientProcess_Implementation(double DeltaTime) override
	{
		for (const auto& Component : ActiveComponents)
		{
			if(Component) Component->OnUpdate(this);
		}
	}
	
	virtual void BeginPlay() override
	{
		Super::BeginPlay();
		PrimaryActorTick.UpdateTickIntervalAndCoolDown(Data.GetTickInterval());
		GenerateSignWidget(Data);
	}
	
	void ApplySignLayout(const FManagedSignData& NewData, UObject* Setter)
	{
		if(!GetCanConfigure(Setter)) return;
		ApplySignLayout_Internal(NewData);
	}

	void ApplySignLayout_Internal(const FManagedSignData& NewData)
	{
		Data = NewData;
		PrimaryActorTick.UpdateTickIntervalAndCoolDown(Data.GetTickInterval());
		OnSignDataChanged(NewData);
	}

	UFUNCTION(NetMulticast, Reliable)
	void OnSignDataChanged(const FManagedSignData& NewData);
	void OnSignDataChanged_Implementation(const FManagedSignData& NewData){ GenerateSignWidget(NewData); }
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void GenerateSignWidget(const FManagedSignData& NewData);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Replicated)
	FManagedSignData Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USignComponentBase*> ActiveComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USignComponentBase*> StaticComponents;

	virtual TArray<FBuildingConnection> GetConnectionsInfo_Implementation(EConnectionDirection direction, FBuildableNote& Note) override
	{
		if(direction == Output) return Super::GetConnectionsInfo_Implementation(direction, Note);
		
		TArray<FBuildingConnection> Out;
		for(auto& Connection : Data.Connections)
		{
			Out.Add(FBuildingConnection(Connection.Name, Connection.Name, Connection.Type));
		}

		return Out;
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(AManagedSign, Data)
	}

	virtual bool CanUseFactoryClipboard_Implementation() override { return true; }
	virtual TSubclassOf<UObject> GetClipboardMappingClass_Implementation() override { return StaticClass();}
	virtual TSubclassOf<UFGFactoryClipboardSettings> GetClipboardSettingsClass_Implementation() override { return UManagedSignClipboardData::StaticClass(); }

	virtual UFGFactoryClipboardSettings* CopySettings_Implementation() override
	{
		auto Clipboard = NewObject<UManagedSignClipboardData>(this);
		Clipboard->Data = Data;
		return Clipboard;
	}

	virtual bool PasteSettings_Implementation(UFGFactoryClipboardSettings* factoryClipboard) override
	{
		if(auto Clipboard = Cast<UManagedSignClipboardData>(factoryClipboard))
		{
			if(Data.Size != Clipboard->Data.Size) return false;
			
			ApplySignLayout_Internal(Clipboard->Data);
			return true;
		}

		return false;
	}
};


UCLASS()
class UManagedSignUtilityFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, meta=(ScriptOperator="=="), DisplayName="Sign layout equals")
	static bool SignLayoutEquals(const FManagedSignData& Data1, const FManagedSignData& Data2) { return Data1 == Data2; }

	UFUNCTION(BlueprintPure)
	static FString GetSignLayoutsFolderPath(UObject* WorldContext)
	{
		auto const GameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(WorldContext));
		if(!GameState) return "ERROR: SIGN BUILDING IS NULL";
		auto const SessionName = GameState->GetSessionName();
		
		return FPaths::ProjectSavedDir() + "Circuitry/" + SessionName + "/SignLayouts/";
	}

	UFUNCTION(BlueprintCallable)
	static void GetSignLayoutFileList(UObject* WorldContext, TArray<FString>& Out) { UFileUtilities::FindAllFilesInDirectory(GetSignLayoutsFolderPath(WorldContext), TArray<FString>{".csl"}, Out); }
	
	UFUNCTION(BlueprintCallable)
	static bool ParseSignDataToJson(const FManagedSignData& Data, FString& Out)
	{
		TSharedPtr<FJsonObject> Object = FJsonObjectConverter::UStructToJsonObject(Data);
		if(!Object) return false;
		
		return FJsonSerializer::Serialize(Object.ToSharedRef(), TJsonWriterFactory<>::Create(&Out, 0));
	}

	UFUNCTION(BlueprintCallable)
	static bool LoadSignFromJson(const FString& Json, FManagedSignData& Out)
	{
		TSharedPtr<FJsonObject> Object;
		if(!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Object)) return false;

		if(!FJsonObjectConverter::JsonObjectToUStruct(Object.ToSharedRef(), &Out)) return false;

		return true;
	}
};
