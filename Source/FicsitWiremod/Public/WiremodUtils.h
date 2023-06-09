// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once
#include "AbstractInstanceManager.h"
#include "FGCharacterPlayer.h"
#include "FGTimeSubsystem.h"
#include "WiremodReflection.h"
#include "Behaviour\CircuitryInterface.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "Kismet/KismetStringLibrary.h"
#include "Utility/WiremodGameWorldModule.h"

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
		if(auto abstractHit = Cast<AAbstractInstanceManager>(hit.Actor))
		{
			FInstanceHandle handle;
			abstractHit->ResolveHit(hit, handle);
			return handle.GetOwner<AActor>();
		}
		return hit.Actor.Get();
	}


#define WM UWiremodReflection
#define STR UKismetStringLibrary
	UFUNCTION(BlueprintPure)
	static FString GetStringifiedValue(const FNewConnectionData& Value)
	{
		switch (Value.ConnectionType)
		{
		case Unknown: return "?";
		case Boolean: return WM::GetFunctionBoolResult(Value) ? "true" : "false";
		case Number: return FString::SanitizeFloat(WM::GetFunctionNumberResult(Value));
		case String: return WM::GetFunctionStringResult(Value);
		case Vector: return WM::GetFunctionVectorResult(Value).ToCompactString();
		case Inventory:
			{
				auto inv = WM::GetFunctionInventory(Value);
				if(!inv) return "Invalid Inv.";

				TArray<FInventoryStack> Stacks;
				inv->GetInventoryStacks(Stacks);
				
				return STR::Conv_IntToString(Stacks.Num()) + "/" + STR::Conv_IntToString(inv->GetSizeLinear()) + " slots occupied";
			};
		case PowerGrid: return "?";
		case Entity:
			{
				auto entity = WM::GetFunctionEntityResult(Value);
				auto objectName = UKismetSystemLibrary::GetObjectName(entity);
				
				if(auto player = Cast<AFGCharacterPlayer>(entity))
				{
					//Check if the player state is valid. If the player is offline it will be null and crash if not handled properly
					if(auto state = player->GetPlayerState()) return objectName + "(Player " + state->GetPlayerName() + ")";
					return UKismetSystemLibrary::GetObjectName(player) + "(Offline player)";
				}
				return UKismetSystemLibrary::GetObjectName(entity);
			}
		case Recipe:
			{
				auto Recipe = WM::GetFunctionRecipeResult(Value);
				return IsValid(Recipe) ? UFGRecipe::GetRecipeName(Recipe).ToString() : FString();
			}
		case Color: return WM::GetFunctionColorResult(Value).ToString();
		case ArrayOfBoolean: return "[" + STR::Conv_IntToString(WM::GetBoolArray(Value).Num()) + " elements]";
		case ArrayOfNumber: return "[" + STR::Conv_IntToString(WM::GetNumberArray(Value).Num()) + " elements]";
		case ArrayOfString: return "[" + STR::Conv_IntToString(WM::GetStringArray(Value).Num()) + " elements]";
		case ArrayOfVector: return "[" + STR::Conv_IntToString(WM::GetVectorArray(Value).Num()) + " elements]";
		case ArrayOfEntity: return "[" + STR::Conv_IntToString(WM::GetEntityArray(Value).Num()) + " elements]";
		case ArrayOfColor: return "[" + STR::Conv_IntToString(WM::GetColorArray(Value).Num()) + " elements]";
		case ArrayOfInventory: return "[" + STR::Conv_IntToString(WM::GetInventoryArray(Value).Num()) + " elements]";
		case ArrayOfPowerGrid: return "[" + STR::Conv_IntToString(WM::GetPowerGridArray(Value).Num()) + " elements]";
		case ArrayOfStack: return "[" + STR::Conv_IntToString(WM::GetItemStackArray(Value).Num()) + " elements]";
		case ArrayOfRecipe: return "[" + STR::Conv_IntToString(WM::GetRecipeArray(Value).Num()) + " elements]";
		case Stack:
			{
				auto stack = WM::GetFunctionStackResult(Value);
				return STR::Conv_IntToString(stack.NumItems) + " " + UFGItemDescriptor::GetItemName(stack.Item.GetItemClass()).ToString();
			}
		case Integer: return STR::Conv_IntToString(WM::GetFunctionNumberResult(Value));
		case Any: return "?";
		case AnyArray: return "?";
		case AnyNonArray: return "?";
		case ItemAmount:
			{
				auto item = WM::GetItemAmount(Value);
				return STR::Conv_IntToString(item.Amount) + " of " + UFGItemDescriptor::GetItemName(item.ItemClass).ToString();
			}
		case ArrayOfItemAmount: return "[" + STR::Conv_IntToString(WM::GetItemAmountArray(Value).Num()) + " elements]";
		case CustomStruct:
			{
				auto Val = WM::GetCustomStruct(Value);
				return  Val.Name + " [" + FString::FromInt(Val.Values.Num()) + " values]";
			}
		default:
			UE_LOG(LogTemp, Error, TEXT("Failed to find switch case for EConnectionType::%d in function GET_STRINGIFIED_VALUE. Returning default value instead..."), (int)Value.ConnectionType);
			return "?";
		}
	}
	
	
	static FString GetModReference(UObject* object)
	{
		if(!IsValid(object)) return "";
		TArray<FString> pathParse;
		UKismetSystemLibrary::GetPathName(object).ParseIntoArray(pathParse, *FString("/"), true);
		if(pathParse.Num() == 0) return "";
		
		return pathParse[0];
	}
    
    	
	UFUNCTION(BlueprintPure)
	static FName GetClassName(UClass* inClass)
	{
		auto unparsed = UKismetSystemLibrary::GetClassDisplayName(inClass);
		TArray<FString> Remove = {"Build_", "BP_", "SF+_", "RP_", "SF_", "MP_", "FF_"};
		FString ReplaceWith = "";
		for (FString Element : Remove)
		{
			unparsed = unparsed.Replace(*Element, *ReplaceWith);
		}
    
		auto parsed = UKismetStringLibrary::GetSubstring(unparsed, 0, unparsed.Len() - 2);
    		
		return FName(parsed);
	}


	UFUNCTION(BlueprintPure)
	static bool IsValidConnectionPair(EConnectionType Input, EConnectionType Output)
	{
		if(Input == Unknown || Output == Unknown) return false;
		if(Input == Any) return Output != CustomStruct;
		if(Input == Output) return true;
		if(Input == AnyArray) return IsArrayType(Output);
		if(Input == AnyNonArray) return !IsArrayType(Output);
		if(Input == Number || Input == Integer) return Output == Number || Output == Integer;
		if(Input == NonReferenceable) return Output != Inventory && Output != PowerGrid;
		
		return false;
	}

	UFUNCTION(BlueprintPure)
	static EConnectionType BaseToArray(EConnectionType in)
	{
		switch (in)
		{
		case Unknown: return Unknown;
		case Boolean: return ArrayOfBoolean;
		case String: return ArrayOfString;
		case Integer:
		case Number: return ArrayOfNumber;
		case Color: return ArrayOfColor;
		case Vector: return ArrayOfVector;
		case Entity: return ArrayOfEntity;
		case PowerGrid: return ArrayOfPowerGrid;
		case Inventory: return ArrayOfInventory;
		case Stack: return ArrayOfStack;
		case Recipe: return ArrayOfRecipe;
		case Any: return Any;
		case AnyNonArray: return AnyArray;
		case ItemAmount: return ArrayOfItemAmount;
		default:
			UE_LOG(LogTemp, Error,TEXT("[WIREMOD] Failed to find a switch case for EConnectionType::%d in function BASE_TO_ARRAY"), in);
			return Unknown;
		}
	}

	UFUNCTION(BlueprintPure)
	static EConnectionType ArrayToBase(EConnectionType in)
	{
		switch (in)
		{
		case Unknown: return Unknown;
		case ArrayOfBoolean: return Boolean;
		case ArrayOfNumber: return Number;
		case ArrayOfString: return String;
		case ArrayOfVector: return Vector;
		case ArrayOfColor: return Color;
		case ArrayOfEntity: return Entity;
		case ArrayOfInventory: return Inventory;
		case ArrayOfStack: return Stack;
		case ArrayOfPowerGrid: return PowerGrid;
		case ArrayOfRecipe: return Recipe;
		case Any: return Any;
		case AnyArray: return AnyNonArray;
		case ArrayOfItemAmount: return ItemAmount;
		default:
			UE_LOG(LogTemp, Error,TEXT("[WIREMOD] Failed to find a switch case for EConnectionType::%d in function ARRAY_TO_BASE"), in);
			return Unknown;
		}
	}

	UFUNCTION(BlueprintPure)
	static bool IsArrayType(EConnectionType type)
	{
		switch (type)
		{
		case ArrayOfBoolean:
		case ArrayOfVector:
		case ArrayOfColor:
		case ArrayOfEntity:
		case ArrayOfInventory:
		case ArrayOfNumber:
		case ArrayOfStack:
		case ArrayOfString:
		case ArrayOfPowerGrid:
		case ArrayOfRecipe:
		case ArrayOfItemAmount:
			return true;

		default: return false;
		}
	}

	

	UFUNCTION(BlueprintPure)
	static bool HasNoteData(const FBuildableNote& note) { return !note.Text.IsEmpty(); }

	UFUNCTION(BlueprintCallable)
	static void CopyTextToClipboard(FString text) { FPlatformMisc::ClipboardCopy(*text); }

	UFUNCTION(BlueprintPure)
	static UTexture2D* GetTexture(AFGBuildable* Buildable)
	{
		if(!Buildable) return nullptr;
		
		if(auto Wiremod = Cast<ICircuitryProcessableInterface>(Buildable))
		{
			return Wiremod->GetTexture();
		}

		return UFGItemDescriptor::GetBigIcon(Buildable->GetBuiltWithDescriptor());
	}


	UFUNCTION(BlueprintPure)
	static FORCEINLINE float TraceDistance()
	{
		return Cast<UConfigPropertyFloat>(Cast<UConfigPropertySection>(UWiremodGameWorldModule::Self->GetConfig())->SectionProperties["WiremodTool_RaycastDistance"])->Value;
	}

	UFUNCTION(BlueprintPure)
	static UConfigPropertyArray* DefaultWireColor(FLinearColor& Color)
	{
		auto Config = Cast<UConfigPropertySection>(UWiremodGameWorldModule::Self->GetConfig());
		auto ColorProperty = Cast<UConfigPropertyArray>(Config->SectionProperties["Wire_Color"]);

		float R = Cast<UConfigPropertyFloat>(ColorProperty->Values[0])->Value;
		float G = Cast<UConfigPropertyFloat>(ColorProperty->Values[1])->Value;
		float B = Cast<UConfigPropertyFloat>(ColorProperty->Values[2])->Value;
		
		Color = FLinearColor(R, G, B);
		return ColorProperty;
	}

	UFUNCTION(BlueprintPure)
	static bool ShouldToolKeepState()
	{
		auto Config = Cast<UConfigPropertySection>(UWiremodGameWorldModule::Self->GetConfig());

		return Cast<UConfigPropertyBool>(Config->SectionProperties["WiremodTool_KeepState"])->Value;
	}

	UFUNCTION(BlueprintPure)
	static UConfigPropertyBool* WireDefaultHidden(bool& Out)
	{
		auto Config = Cast<UConfigPropertySection>(UWiremodGameWorldModule::Self->GetConfig());
		auto BoolProperty = Cast<UConfigPropertyBool>(Config->SectionProperties["Wire_Hidden"]);

		Out = BoolProperty->Value;
		return BoolProperty;
	}

	UFUNCTION(BlueprintPure)
	static UConfigPropertyFloat* WireEmission(float& Out)
	{
		auto Config = Cast<UConfigPropertySection>(UWiremodGameWorldModule::Self->GetConfig());
		auto FloatProperty = Cast<UConfigPropertyFloat>(Config->SectionProperties["Wire_Emission"]);

		Out = FloatProperty->Value;
		return FloatProperty;
	}

};
