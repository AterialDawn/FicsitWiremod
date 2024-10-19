﻿// 

#pragma once

#include "CoreMinimal.h"
#include "ArrayGet.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "ArrayBreak.generated.h"

UCLASS()
class FICSITWIREMOD_API AArrayBreak : public AFGWiremodBuildable, public IDynamicValuePasser
{
	GENERATED_BODY()
    
public:
	virtual void ServerProcess_Implementation(double DeltaTime) override
	{
		ArrayCache = Cast<UCCArrayValueBase>(UCCDynamicValueUtils::FromValue(GetConnection(0), ArrayCache));
	}

	virtual TArray<FBuildingConnection> GetConnections_Implementation(EConnectionDirection direction) override
	{
		if(direction == Input) return Super::GetConnections_Implementation(direction);
		if(!ArrayCache) return TArray<FBuildingConnection>();

		const EConnectionType ElementType = UConnectionTypeFunctions::ArrayToBase(ArrayCache->ConnectionType);
		const int ElementsCount = ArrayCache->Length();
		TArray<FBuildingConnection> Out;
		for(int i = 0; i < ElementsCount; i++)
		{
			Out.Add(
				FBuildingConnection(
					ItemDisplayNameFormat.ToString() + " " + FString::FromInt(i),
					FString::FromInt(i),
					ElementType
					)
				);
		}
		
		return Out;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(AArrayBreak, ArrayCache)
	}

	virtual void GatherReplicatedObjects_Implementation(TArray<UObject*>& OutObjects) override
	{
		Super::GatherReplicatedObjects_Implementation(OutObjects);
		OutObjects.Add(ArrayCache);
	}

	UPROPERTY(Replicated, SaveGame)
	UCCArrayValueBase* ArrayCache;

	UPROPERTY()
	TArray<UCCDynamicValueBase*> ArrayIndexedCache;
	
	UPROPERTY(EditDefaultsOnly)
	FText ItemDisplayNameFormat = FText::FromString("Item");
	
	virtual UObject* GetValue_Implementation(const FString& ValueName) override
	{
		if(!ArrayCache) return nullptr;

		const int Index = FCString::Atoi(*ValueName);

		//Caching the array elements so that we don't have to create new ones every frame.
		//Saves a lot of time that we spend on uobject garbage collecting.
		UCCDynamicValueBase* IndexedCacheData = ArrayIndexedCache.IsValidIndex(Index) ? ArrayIndexedCache[Index] : nullptr;
		auto Out = ArrayCache->GetElement(Index, IndexedCacheData);
		ArrayIndexedCache.SetNum(ArrayCache->Length());
		if(ArrayIndexedCache.IsValidIndex(Index)) ArrayIndexedCache[Index] = Out;
		
		return Out;
	}
};
