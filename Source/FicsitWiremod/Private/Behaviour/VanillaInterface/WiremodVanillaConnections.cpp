﻿
#include "Behaviour/VanillaInterface/WiremodVanillaConnections.h"

void ACircuitryBlueprintConnectionProxy::Tick(float DeltaSeconds)
{
	//If our object no longer exists then we should just destroy the proxy.
	if(!SavedData.Buildable)
	{
		Execute_Dismantle(this);
		return;
	}

	//Find data for this buildable
	auto Data = AWiremodVanillaConnections::Self->Game_VanillaBuildableData.Find(SavedData.Buildable);

	//If data was found then we're most likely in a blueprint designer, save the current data to not have any desync issues.
	if(Data) SavedData.Data = *Data;
	//If no data was found and blueprint designer is null, then we were just spawned from a blueprint and need to register the data with the system.
	else if(!mBlueprintDesigner) ApplyConnectionToSystem();
}

void ACircuitryBlueprintConnectionProxy::ApplyConnectionToSystem()
{
	if(SavedData.Buildable)
	{
		AWiremodVanillaConnections::Self->UpdateBuildable(SavedData.Buildable, SavedData.Data.Connections, SavedData.Data.OwnerData);
		AWiremodVanillaConnections::Self->DrawWiresForBuildable(SavedData.Buildable);
		ACircuitryLogger::DispatchEvent("[BP_PROXY] Connection applied to subsystem, destroying proxy...", ELogVerbosity::Display);
	}
	Execute_Dismantle(this);
}

void AWiremodVanillaConnections::DrawWiresForBuildable(FVanillaBuildingDataKeyValuePair KeyValuePair, bool SkipDestruct)
{

	AActor* Buildable = Cast<AActor>(KeyValuePair.Buildable);
	if(!Buildable) return;
	
	//Remove all existing wires
	if(!SkipDestruct)
	{
		TArray<UActorComponent*> ChildComponents;
		Buildable->GetComponents(UChildActorComponent::StaticClass(), ChildComponents);
		for (auto ChildComponent : ChildComponents)
		{
			auto ChildActorComponent = Cast<UChildActorComponent>(ChildComponent);
			if(auto Wire = Cast<AConnectionWireBase>(ChildActorComponent->GetChildActor()))
			{
				Wire->Invalidate();
				ChildActorComponent->DestroyChildActor();
				ChildActorComponent->DestroyComponent();
			}
		}
	}

	//If there are no wires to draw then exit the function
	if(KeyValuePair.Data.Connections.Num() == 0) return;

	//Visible wire class to use as wire
	auto WireClass = AConnectionWireBase::StaticClass();
	
	//Create new wires
	for (auto& Connection : KeyValuePair.Data.Connections)
	{
		if(!Connection.IsValidForWire()) continue;
				
		//Create child actor component to store our wire actor. This should probably be optimized as it's a lot better for performance to just have 1 actor that handles all the wires.
		auto WireActorComponent = Cast<UChildActorComponent>(Buildable->AddComponentByClass(UChildActorComponent::StaticClass(), false, FTransform::Identity, false));
		WireActorComponent->SetIsReplicated(true);
		WireActorComponent->SetChildActorClass(WireClass);
		WireActorComponent->CreateChildActor();
		auto WireActor = Cast<AConnectionWireBase>(WireActorComponent->GetChildActor());
		WireActor->SetActorTickEnabled(true);

		//Assign data to wire
		WireActor->AssignedConnection = Connection;
		WireActor->DrawWireFromData();
				
		WireActorComponent->AttachToComponent(Buildable->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
		WireActorComponent->SetHiddenInGame(false);
	}
}

void AWiremodVanillaConnections::Tick(float DeltaTime)
{
	if(!HasAuthority()) return;

	bool FilterNulls = false;
	TArray<int> ClearIndexes;
	
	for(int i = 0; i < SavedVanillaBuildableData.Num(); i++)
	{
		auto Data = SavedVanillaBuildableData[i];
		
		if(!Data.Buildable)
		{
			FilterNulls = true;
			ClearIndexes.Add(i);
			continue;
		}
			
		if(Data.Data.Connections.Num() == 0) continue;
		HandleDynamicConnections(Data.Data.Connections);
	}

	if(FilterNulls)	FilterNullPointers(ClearIndexes);
}


bool AWiremodVanillaConnections::HandleDynamicConnections(const TArray<FDynamicConnectionData>& Connections)
{
	bool HadValid = false;
	for (auto ConnectionData : Connections)
	{
		if(!ConnectionData.Transmitter.Object || !ConnectionData.Receiver.Object) continue;

		HadValid = true;
			
		switch (ConnectionData.Receiver.ConnectionType)
		{
		case Boolean: ConnectionData.Receiver.SetBool( ConnectionData.Transmitter.GetBool()); break;
		case Number:
		case Integer: ConnectionData.Receiver.SetFloat(ConnectionData.Transmitter.GetFloat()); break;
		case String: ConnectionData.Receiver.SetString(ConnectionData.Transmitter.GetString()); break;
		case Color: ConnectionData.Receiver.SetColor(ConnectionData.Transmitter.GetColor()); break;
		case Recipe: ConnectionData.Receiver.SetRecipe(ConnectionData.Transmitter.GetRecipe()); break;
		case ItemDescriptor: ConnectionData.Receiver.Set(ConnectionData.Transmitter.GetItemDescriptor()); break;
		case ArrayOfSplitterRule: ConnectionData.Receiver.Set(ConnectionData.Transmitter.GetSplitterRuleArray()); break;
		case ArrayOfItemDescriptor: ConnectionData.Receiver.Set(ConnectionData.Transmitter.GetItemDescriptorArray()); break;
		default: break;
		}
	}

	return HadValid;
}

void AWiremodVanillaConnections::FilterNullPointers(const TArray<int>& ClearIndexes)
{
	TArray<FVanillaBuildingDataKeyValuePair> Filtered;

	for(int i = 0; i < SavedVanillaBuildableData.Num(); i++)
	{
		auto Data = SavedVanillaBuildableData[i];
		if(!Data.Buildable || ClearIndexes.Contains(i)) continue;
		Filtered.Add(Data);
	}

	SavedVanillaBuildableData = Filtered;
	UpdateConnectionData();
}