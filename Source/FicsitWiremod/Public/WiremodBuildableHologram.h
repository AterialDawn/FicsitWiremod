// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Configuration/Properties/ConfigPropertyFloat.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "Hologram/FGBuildableHologram.h"
#include "Utility/WiremodGameWorldModule.h"
#include "WiremodBuildableHologram.generated.h"

/**
 * 
 */
UCLASS()
class FICSITWIREMOD_API AWiremodBuildableHologram : public AFGBuildableHologram
{
	GENERATED_BODY()
	AWiremodBuildableHologram()
	{
		mNeedsValidFloor = false;
		mAttachmentPointSnapDistanceThreshold = 3000;
		
	}

	virtual void BeginPlay() override
	{
		Super::BeginPlay();
		mGridSnapSize = Cast<UConfigPropertyFloat>(Cast<UConfigPropertySection>(UWiremodGameWorldModule::Self->GetConfig())->SectionProperties["WiremodHolo_SnapSize"])->Value;
	}
	
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;

	virtual bool IsValidHitResult(const FHitResult& hitResult) const override
	{
		return IsValid(hitResult.GetActor());
	};

	virtual void ScrollRotate(int32 delta, int32 step) override
	{
		int32 NewRot = mScrollRotation + delta * step;
		mScrollRotation = NewRot - NewRot%step;
	}

	virtual int32 GetRotationStep() const override { return 5; }

public:

	UFUNCTION(BlueprintNativeEvent)
	double GetGridSize();
	double GetGridSize_Implementation() { return mGridSnapSize; }
};
