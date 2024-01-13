

#pragma once

#include "CoreMinimal.h"
#include "CCImageData.h"
#include "Behaviour/FGWiremodBuildable.h"
#include "PixelArtManager.generated.h"

/**
 * 
 */
UCLASS()
class FICSITWIREMOD_API APixelArtManager : public AFGWiremodBuildable
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void SetPixelScreenData(const FPixelScreenData& NewData, UObject* Setter)
	{
		PERMISSION_CHECK(Setter);
		
		OnImageChanged(NewData);
	}

	UFUNCTION()
	UTexture* GetImageTexture()
	{
		if(!TextureCache) TextureCache = Data.MakeTexture();
		return TextureCache;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(APixelArtManager, Data)
	}

	UFUNCTION(NetMulticast, Reliable)
	void OnImageChanged(FPixelScreenData NewData);
	void OnImageChanged_Implementation(FPixelScreenData NewData)
	{
		if(HasAuthority()) Data = NewData;
		TextureCache = Data.MakeTexture();
	}
	
	UPROPERTY(Replicated, SaveGame, BlueprintReadWrite)
	FPixelScreenData Data;

	UPROPERTY()
	UTexture* TextureCache;
};



