// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once
#include "AbstractInstanceManager.h"
#include "FGBuildingDescriptor.h"
#include "Behaviour/CircuitryInterface.h"
#include "Buildables/FGBuildable.h"

#include "WiremodUtils.generated.h"


/**
 * 
 */
UCLASS()
class FICSITWIREMOD_API UWiremodUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static AActor* GetActualHitTarget(const FHitResult& hit, FVector& Location)
	{
		Location = hit.Location;
		if(auto abstractHit = Cast<AAbstractInstanceManager>(hit.GetActor()))
		{
			FInstanceHandle handle;
			abstractHit->ResolveHit(hit, handle);
			return handle.GetOwner<AActor>();
		}
		return hit.GetActor();
	}
	
	UFUNCTION(BlueprintPure)
	static FString GetModReference(UObject* object)
	{
		if(!IsValid(object)) return "";
		TArray<FString> pathParse;
		object->GetPathName().ParseIntoArray(pathParse, *FString("/"), true);
		if(pathParse.Num() == 0) return "";
		
		return pathParse[0];
	}

	UFUNCTION(BlueprintPure)
	static FString GetModReferenceFromObjectPath(const FSoftObjectPath& ObjectPath)
	{
		TArray<FString> pathParse;
		ObjectPath.GetAssetPath().ToString().ParseIntoArray(pathParse, *FString("/"), true);
		if(pathParse.Num() == 0) return "";
		
		return pathParse[0];
	}
    
    	
	UFUNCTION(BlueprintPure)
	static FName GetClassName(UClass* inClass, bool DoFilter = true)
	{
		auto unparsed = UKismetSystemLibrary::GetClassDisplayName(inClass);
		unparsed.MidInline(0, unparsed.Len() - 2);

		if(DoFilter)
		{
			TArray<FString> Remove = {"Build_", "BP_", "SF+_", "RP_", "SF_", "MP_", "FF_"};
			FString ReplaceWith = "";

			for (FString Element : Remove)
			{
				unparsed.ReplaceInline(*Element, *ReplaceWith, ESearchCase::CaseSensitive);
			}
		}
		
		return FName(unparsed);
	}

	UFUNCTION(BlueprintPure)
	static UTexture2D* GetTexture(AFGBuildable* Buildable)
	{
		if(!Buildable) return nullptr;
		
		if(auto Circuitry = Cast<ICircuitryProcessableInterface>(Buildable))
		{
			return Circuitry->GetTexture();
		}

		return UFGItemDescriptor::GetBigIcon(Buildable->GetBuiltWithDescriptor());
	}

	UFUNCTION(BlueprintPure)
	static TSubclassOf<UFGBuildingDescriptor> GetBuiltWithDescriptor(AFGBuildable* Buildable)
	{
		if (!Buildable) return nullptr;

		return TSubclassOf<UFGBuildingDescriptor>(Buildable->GetBuiltWithDescriptor());
	}

};
