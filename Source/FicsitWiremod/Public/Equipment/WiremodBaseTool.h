﻿// 

#pragma once

#include "WiremodUtils.h"
#include "CoreMinimal.h"
#include "Equipment/FGEquipment.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/CircuitryInputMappings.h"
#include "EnhancedInputSubsystems.h"
#include "FGPlayerController.h"
#include "WiremodBaseTool.generated.h"

UCLASS()
class FICSITWIREMOD_API AWiremodBaseTool : public AFGEquipment
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	AActor* GetTargetLookAt(double TraceDistance, TEnumAsByte<ETraceTypeQuery> Channel, FVector& Location, bool& Success)
	{
		auto camera = UGameplayStatics::GetPlayerCameraManager(this, 0);

		FVector Start = camera->GetCameraLocation();
		FVector End = camera->GetCameraLocation() + camera->GetActorForwardVector() * TraceDistance;
		FHitResult Result;

		Success = UKismetSystemLibrary::LineTraceSingle(this, Start, End, Channel, false, TArray<AActor*>(), EDrawDebugTrace::None, Result, true);
		return UWiremodUtils::GetActualHitTarget(Result, Location);
	}

	
	virtual void WasEquipped_Implementation() override
	{
		Super::WasEquipped_Implementation();
		if(IsLocallyControlled()) InjectMappings();
	}

	virtual void WasUnEquipped_Implementation() override
	{
		Super::WasUnEquipped_Implementation();
		if(IsLocallyControlled()) EjectMappings();
	}

	virtual void OnInteractWidgetAddedOrRemoved(UFGInteractWidget* widget, bool added) override
	{
		Super::OnInteractWidgetAddedOrRemoved(widget, added);
		if(added) EjectMappings();
		else InjectMappings();
	}

#if !WITH_EDITOR
	virtual void Destroyed() override
	{
		if(IsLocallyControlled()) EjectMappings();
		Super::Destroyed();
	}
#endif

	virtual bool ShouldSaveState() const override { return true; }

	template<class T>
	T* GetFirstContextOfType()
	{
		for(auto& InputsContext : InputsContexts)
		{
			if(InputsContext.Value->IsA<T>()) return Cast<T>(InputsContext.Value);
		}

		return nullptr;
	}
		

	void InjectMappings()
	{
		auto Player = Cast<AFGPlayerController>(GetInstigatorController());
		if(!Player)
		{
			ACircuitryLogger::DispatchErrorEvent("[CIRCUITRY TOOL KEYBIND HANDLER] Somehow, the player reference was null when injecting mappings.");
			return;
		}
		
		auto LocalPlayer = Player->GetLocalPlayer();
		if(!LocalPlayer)
		{
			ACircuitryLogger::DispatchErrorEvent("[CIRCUITRY TOOL KEYBIND HANDLER] Somehow, the local player reference was null when injecting mappings.");
			return;
		}
		
		auto EnhancedInputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if(!EnhancedInputSystem)
		{
			ACircuitryLogger::DispatchErrorEvent("[KEYBIND HANDLER] Input System is nullptr.");
			return;
		}
		
		for(auto& InputsContext : InputsContexts) EnhancedInputSystem->AddMappingContext(InputsContext.Value, MappingContextPriority);
		ApplyScrollWheelPatch();
	}

	void EjectMappings()
	{
		auto Player = Cast<AFGPlayerController>(GetInstigatorController());
		if(!Player)
		{
			ACircuitryLogger::DispatchErrorEvent("[CIRCUITRY TOOL KEYBIND HANDLER] Somehow, the player reference was null when ejecting mappings.");
			return;
		}
		
		auto LocalPlayer = Player->GetLocalPlayer();
		if(!LocalPlayer)
		{
			ACircuitryLogger::DispatchErrorEvent("[CIRCUITRY TOOL KEYBIND HANDLER] Somehow, the local player reference was null when ejecting mappings.");
			return;
		}
		
		auto EnhancedInputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if(!EnhancedInputSystem)
		{
			ACircuitryLogger::DispatchErrorEvent("[KEYBIND HANDLER] Input System is nullptr.");
			return;
		}
		
		for(auto& InputsContext : InputsContexts) EnhancedInputSystem->RemoveMappingContext(InputsContext.Value);
		RemoveScrollWheelPatch();
	}

	
	UFUNCTION(BlueprintImplementableEvent) void ApplyScrollWheelPatch();
	UFUNCTION(BlueprintImplementableEvent) void RemoveScrollWheelPatch();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MappingContextPriority = 10000;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FName, UFGInputMappingContext*> InputsContexts;

protected:

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ShowNotification(const FText& Text);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void HideHudHints();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DisplayHudHints();


	FORCEINLINE bool IsLocallyControlled() const { return GetInstigator() ? GetInstigator()->IsLocallyControlled() : false; }
};
