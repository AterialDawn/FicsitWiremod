﻿// 

#pragma once

#include "CoreMinimal.h"
#include "CCDynamicValueBase.h"
#include "CCArrayValueBase.h"
#include "FGTrainStationIdentifier.h"
#include "ReflectionExternalFunctions.h"
#include "Behaviour/Gates/Arrays/Filter/Filters/CircuitryStringArrayFilter.h"
#include "Buildables/FGBuildableRailroadStation.h"
#include "CommonLib/ReflectionUtilities.h"
#include "CCStringValue.generated.h"

/**
 * 
 */
UCLASS(Blueprintable,BlueprintType)
class FICSITWIREMOD_API UCCStringValue : public UCCDynamicValueBase
{
	GENERATED_BODY()

public:

	UCCStringValue() : Super(String){}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(UCCStringValue, Value)
	}

	virtual void FromConnectionValue(UObject* Object, FName SourceName, bool FromProperty) override
	{
		if(!Object) return;
		if(Object->GetClass()->ImplementsInterface(IDynamicValuePasser::UClassType::StaticClass()))
			if(auto SameType = Cast<UCCStringValue>(IDynamicValuePasser::Execute_GetValue(Object, SourceName.ToString())))
			{
				Value = SameType->Value;
				return;
			}

		if(auto Sign = Cast<AFGBuildableWidgetSign>(Object))
			Value = UReflectionExternalFunctions::GetSignText(Sign, SourceName.ToString(), Value);
		else if(auto station = Cast<AFGBuildableRailroadStation>(Object))
			Value = station->GetStationIdentifier()->GetStationName().ToString();
		else
			Value = UReflectionUtilities::GetString(REFLECTION_ARGS);
	}

	virtual bool Equals(UCCDynamicValueBase* Other, bool ComparePointers = true) override
	{
		if(auto OtherSource = Cast<ThisClass>(Other))
			return OtherSource->Value == Value;
		
		return Super::Equals(Other, ComparePointers);
	}

	virtual bool Equals(UObject* Object, FName SourceName, bool FromProperty) override
	{
		return UReflectionUtilities::GetString(Object, SourceName, FromProperty) == Value;
	}

	virtual FString ToString() override { return Value; }

	virtual bool FromWrapperValue(const FDynamicValueStringWrapper& Wrapper) override
	{
		Value = Wrapper.Value;
		return true;
	}

	virtual FDynamicValueStringWrapper ToWrapperValue() override { return FDynamicValueStringWrapper(ConnectionType, Value); }
	
	UPROPERTY(Replicated, SaveGame, BlueprintReadWrite)
	FString Value;
};



UCLASS(Blueprintable,BlueprintType)
class FICSITWIREMOD_API UCCStringArrayValue : public UCCArrayValueBase
{
	GENERATED_BODY()

public:
	UCCStringArrayValue() : Super(ArrayOfString){}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(UCCStringArrayValue, Value)
	}

	virtual void FromConnectionValue(UObject* Object, FName SourceName, bool FromProperty) override
	{
		if(!Object) return;
		if(Object->GetClass()->ImplementsInterface(IDynamicValuePasser::UClassType::StaticClass()))
			if(auto SameType = Cast<UCCStringArrayValue>(IDynamicValuePasser::Execute_GetValue(Object, SourceName.ToString())))
			{
				Value = SameType->Value;
				return;
			}
		
		Value = UReflectionUtilities::GetStringArray(REFLECTION_ARGS);
	}

	virtual void AddElement(const FConnectionData& Element) override{ Value.Add(Element.GetString()); }
	virtual UCCDynamicValueBase* GetElement(int Index, UObject* Outer) override
	{
		if(!Value.IsValidIndex(Index)) return nullptr;
		
		UCCStringValue* ValueOut;
		if(auto OuterCast = Cast<UCCStringValue>(Outer)) ValueOut = OuterCast;
		else ValueOut = NewObject<UCCStringValue>(this->GetWorld()->PersistentLevel);
		
		ValueOut->Value = Value[Index];
		return ValueOut;
	}
	
	virtual void InsertElement(const FConnectionData& Element, int Index) override
	{
		if(!Value.IsValidIndex(Index)) return;

		Value.Insert(Element.GetString(), Index);
	}
	virtual void Clear() override{ Value.Empty(); }
	virtual int Length() override { return Value.Num(); }
	virtual void RemoveElement(int Index) override { if(Value.IsValidIndex(Index)) Value.RemoveAt(Index); }
	virtual void Resize(int NewSize) override { Value.SetNum(NewSize); }
	virtual void SetElement(const FConnectionData& Element, int Index) override
	{
		if(!Value.IsValidIndex(Index)) return;
		Value[Index] = Element.GetString();
	}
	virtual bool Contains(const FConnectionData& Element) override { return Value.Contains(Element.GetString()); }
	
	virtual bool Equals(UCCDynamicValueBase* Other, bool ComparePointers = true) override
	{
		if(auto OtherSource = Cast<ThisClass>(Other))
			return OtherSource->Value == Value;
		
		return Super::Equals(Other, ComparePointers);
	}

	virtual bool Equals(UObject* Object, FName SourceName, bool FromProperty) override
	{
		return UReflectionUtilities::GetStringArray(Object, SourceName, FromProperty) == Value;
	}

	virtual FString ToString() override { return FString::Join(ToStringArray(), *FString(", ")); }

	virtual bool FromWrapperValue(const FDynamicValueStringWrapper& Wrapper) override
	{
		Value.Empty();
		Wrapper.Value.ParseIntoArray(Value, *FString(ARRAY_SEPARATOR));

		return true;
	}

	virtual FDynamicValueStringWrapper ToWrapperValue() override
	{
		const auto Output = FString::Join(Value, *FString(ARRAY_SEPARATOR));
		return FDynamicValueStringWrapper(ConnectionType, Output);
	}
	
	virtual TArray<FString> ToStringArray() override { return Value; }

	virtual int FindFirst(const FConnectionData& Element) override
	{
		auto SearchValue = Element.GetString();
		for(int i = 0; i < Value.Num(); i++)
		{
			if(Value[i] == SearchValue) return i; 
		}

		return -1;
	}

	virtual TArray<double> FindAll(const FConnectionData& Element) override
	{
		auto Out = TArray<double>();
		auto SearchValue = Element.GetString();
		for(int i = 0; i < Value.Num(); i++)
		{
			if(Value[i] == SearchValue) Out.Add(i);
		}

		return Out;
	}

	virtual void AppendArray(const UCCArrayValueBase* Array) override
	{
		if(auto ThisArray = Cast<ThisClass>(Array))
		{
			Value.Append(ThisArray->Value);
		}
	}

	virtual bool SetFilter(const FCircuitryArrayFilterData& FilterData) override
	{
		if(!Filter) Filter = NewObject<UCircuitryStringArrayFilter>(this);
		return Filter->FromJson(FilterData);
	}

	virtual void ApplyFilter() override
	{
		if(Filter) Value = Filter->FilterValues(Value);
	}
	
	UPROPERTY(Replicated, SaveGame, BlueprintReadWrite)
	TArray<FString> Value;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	UCircuitryStringArrayFilter* Filter = nullptr;
};
