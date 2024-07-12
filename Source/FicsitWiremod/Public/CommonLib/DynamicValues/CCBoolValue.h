﻿// 

#pragma once

#include "CoreMinimal.h"
#include "CCArrayValueBase.h"
#include "CCDynamicValueBase.h"
#include "Behaviour/Gates/Arrays/Filter/Filters/CircuitryBoolArrayFilter.h"
#include "Behaviour/Gates/Arrays/Filter/Rules/CircuitryBoolFilterRule.h"
#include "CommonLib/ReflectionUtilities.h"
#include "CCBoolValue.generated.h"

/**
 * 
 */
UCLASS(Blueprintable,BlueprintType)
class FICSITWIREMOD_API UCCBoolValue : public UCCDynamicValueBase
{
	GENERATED_BODY()

public:

	UCCBoolValue() : Super(Boolean){}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(UCCBoolValue, Value)
	}

	virtual void FromConnectionValue(UObject* Object, FName SourceName, bool FromProperty) override
	{
		if(!Object) return;
		if(Object->GetClass()->ImplementsInterface(IDynamicValuePasser::UClassType::StaticClass()))
			if(auto SameType = Cast<UCCBoolValue>(IDynamicValuePasser::Execute_GetValue(Object, SourceName.ToString())))
			{
				Value = SameType->Value;
				return;
			}
		
		Value = UReflectionUtilities::GetBool(REFLECTION_ARGS, Value);
	}

	virtual bool Equals(UCCDynamicValueBase* Other) override
	{
		if(this == Other) return true;

		if(auto OtherSource = Cast<UCCBoolValue>(Other))
			return OtherSource->Value == Value;

		return false;
	}

	virtual FString ToString() override { return Value ? "true" : "false"; }

	virtual bool FromWrapperValue(const FDynamicValueStringWrapper& Wrapper) override
	{
		Value = Wrapper.Value.Equals("true", ESearchCase::IgnoreCase);
		return true;
	}

	virtual FDynamicValueStringWrapper ToWrapperValue() override
	{
		return FDynamicValueStringWrapper(ConnectionType, ToString());
	}
	
	UPROPERTY(Replicated, SaveGame, BlueprintReadWrite)
	bool Value;
};


UCLASS(Blueprintable,BlueprintType)
class FICSITWIREMOD_API UCCBoolArrayValue : public UCCArrayValueBase
{
	GENERATED_BODY()

public:

	UCCBoolArrayValue() : Super(ArrayOfBoolean){}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(UCCBoolArrayValue, Value)
	}

	virtual void FromConnectionValue(UObject* Object, FName SourceName, bool FromProperty) override
	{
		if(!Object) return;
		if(Object->GetClass()->ImplementsInterface(IDynamicValuePasser::UClassType::StaticClass()))
			if(auto SameType = Cast<UCCBoolArrayValue>(IDynamicValuePasser::Execute_GetValue(Object, SourceName.ToString())))
			{
				Value = SameType->Value;
				return;
			}
		
		Value = UReflectionUtilities::GetBoolArray(REFLECTION_ARGS);
	}
	virtual FString ToString() override { return FString::Join(ToStringArray(), *FString(", ")); }

	virtual bool FromWrapperValue(const FDynamicValueStringWrapper& Wrapper) override
	{
		TArray<FString> StringValues;
		Wrapper.Value.ParseIntoArray(StringValues, *FString(ARRAY_SEPARATOR), false);
		Value.Empty();
		for(auto& StringValue : StringValues) Value.Add(StringValue == "true");

		return true;
	}

	virtual FDynamicValueStringWrapper ToWrapperValue() override
	{
		TArray<FString> Out;
		for(const auto Val : Value) Out.Add(Val ? "true" : "false");
		
		const auto Output = FString::Join(Out, *FString(ARRAY_SEPARATOR));
		return FDynamicValueStringWrapper(ConnectionType, Output);
	}

	

	virtual void AddElement(const FConnectionData& Element) override{ Value.Add(Element.GetBool()); }
	virtual UCCDynamicValueBase* GetElement(int Index) override
	{
		if(!Value.IsValidIndex(Index)) return nullptr;
		
		auto ValueOut = NewObject<UCCBoolValue>(this->GetWorld()->PersistentLevel);
		ValueOut->Value = Value[Index];
		return ValueOut;
	}
	virtual void InsertElement(const FConnectionData& Element, int Index) override
	{
		if(!Value.IsValidIndex(Index)) return;
		
		Value.Insert(Element.GetBool(), Index);
	}
	virtual void Clear() override{ Value.Empty(); }
	virtual int Length() override { return Value.Num(); }
	virtual void RemoveElement(int Index) override { if(Value.IsValidIndex(Index)) Value.RemoveAt(Index); }
	virtual void Resize(int NewSize) override { Value.SetNum(NewSize); }
	virtual void SetElement(const FConnectionData& Element, int Index) override
	{
		if(!Value.IsValidIndex(Index)) return;
		Value[Index] = Element.GetBool();
	}
	virtual bool Contains(const FConnectionData& Element) override { return Value.Contains(Element.GetBool()); }
	
	virtual TArray<FString> ToStringArray() override
	{
		TArray<FString> Out;

		for(const auto& Val : Value)
			Out.Add(Val ? "true" : "false");

		return Out;
	}

	virtual bool Equals(UCCDynamicValueBase* Other) override
	{
		if(this == Other) return true;

		if(auto OtherSource = Cast<UCCBoolArrayValue>(Other))
			return OtherSource->Value == Value;

		return false;
	}

	virtual int FindFirst(const FConnectionData& Element) override
	{
		auto SearchValue = Element.GetBool();
		for(int i = 0; i < Value.Num(); i++)
		{
			if(Value[i] == SearchValue) return i; 
		}

		return -1;
	}

	virtual TArray<double> FindAll(const FConnectionData& Element) override
	{
		auto Out = TArray<double>();
		auto SearchValue = Element.GetBool();
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
		if(!Filter) Filter = NewObject<UCircuitryBoolArrayFilter>(this);
		return Filter->FromJson(FilterData);
	}

	virtual void ApplyFilter() override
	{
		if(Filter) Value = Filter->FilterValues(Value);
	}
	
	UPROPERTY(Replicated, SaveGame, BlueprintReadWrite)
	TArray<bool> Value;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	UCircuitryBoolArrayFilter* Filter = nullptr;
};
