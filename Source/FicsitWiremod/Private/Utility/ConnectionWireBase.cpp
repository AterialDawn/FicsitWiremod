﻿// 


#include "Utility/ConnectionWireBase.h"

#include "WiremodUtils.h"
#include "Behaviour/CircuitryStatics.h"
#include "Components/SplineComponent.h"
#include "Utility/ConnectionWireSplineMesh.h"


// Sets default values
AConnectionWireBase::AConnectionWireBase() : Super()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
	SetRootComponent(Spline);
	NetDormancy = DORM_Never;
	bReplicates = true;
}


void AConnectionWireBase::Tick(float DeltaTime)
{
	if(!PreviewMode && !AssignedConnection.IsValidForWire()) Invalidate();

	if (!PreviewMode) SetActorHiddenInGame(!CircuitryConfig::ShouldWireBeVisible());
}

void AConnectionWireBase::DrawWireFromData()
{
	if(!AssignedConnection.Receiver.Object || !AssignedConnection.Transmitter.Object) return;
	
	TArray<FVector> Points = AssignedConnection.Transmitter.WirePositions;

	//A small hack to force redraw the wire if it doesn't have a complex shape. Implemented for usage with mods that can move buildables (i.e. MicroManage)
	if(Points.Num() == 0)
	{
		Points.SetNum(2);

		if(auto transmitter = Cast<AActor>(AssignedConnection.Transmitter.Object))
			Points[0] = transmitter->GetActorLocation();
		
		if(auto receiver = Cast<AActor>(AssignedConnection.Receiver.Object))
			Points[1] = receiver->GetActorLocation();

		AssignedConnection.Transmitter.UseLocalWirePosition = false;
	}
	
	const auto CoordinateSpace = AssignedConnection.Transmitter.UseLocalWirePosition ? ESplineCoordinateSpace::Local : ESplineCoordinateSpace::World;
	
	DrawWireFromPoints(Points, CoordinateSpace);
}

void AConnectionWireBase::DrawWireFromPoints(const TArray<FVector>& Points, ESplineCoordinateSpace::Type CoordinateSpace)
{
	if(!Spline)
	{
		Spline = Cast<USplineComponent>(AddComponentByClass(USplineComponent::StaticClass(), false, FTransform::Identity, false));
		Spline->Activate();
		Spline->SetIsReplicated(true);
		RootComponent = Spline;
	}
		
	DestroyAllSplineMeshes();
		
	Spline->SetSplinePoints(Points, CoordinateSpace);
	for(int i = 1; i < Points.Num(); i++)
	{
		auto SplineMesh = Cast<UConnectionWireSplineMesh>(AddComponentByClass(UConnectionWireSplineMesh::StaticClass(), false, FTransform::Identity, false));
		SplineMesh->SetStaticMesh(WireMesh);

		FVector StartPos = Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		FVector StartTan = Spline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
		FVector EndPos = Spline->GetLocationAtSplinePoint(i - 1, ESplineCoordinateSpace::Local);
		FVector EndTan = Spline->GetTangentAtSplinePoint(i - 1, ESplineCoordinateSpace::Local);
		SplineMesh->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan);
	}

	UpdateWireVisuals();
}

void AConnectionWireBase::UpdateWireVisuals()
{
	TInlineComponentArray<UConnectionWireSplineMesh*> SplineMeshes;
	GetComponents<UConnectionWireSplineMesh>(SplineMeshes);
	
	auto const Color = FVector(AssignedConnection.Transmitter.WireColor);
	
	for (auto SplineMesh : SplineMeshes)
	{
		SplineMesh->SetScalarParameterValueOnMaterials(FName("Emission"), CircuitryConfig::GetWireEmission());
		SplineMesh->SetVectorParameterValueOnMaterials(FName("CustomColor"), Color);
		SplineMesh->SetScalarParameterValueOnMaterials(FName("Speed"), -1);
	}
}


