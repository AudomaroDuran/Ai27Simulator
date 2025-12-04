// Copyright © 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista

#include "Vehicles/TestVehicle.h"
#include "Components/SplineMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RoadSystem/RoadSplineActor.h"
#include "RoadSystem/RoadIntersection.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ATestVehicle::ATestVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create mesh component
	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	RootComponent = VehicleMesh;

	// Try to load default cube mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		VehicleMesh->SetStaticMesh(CubeMesh.Object);
		VehicleMesh->SetRelativeScale3D(FVector(2.0f, 1.0f, 0.5f)); // Make it look like a car
	}

	// Create movement component
	MovementComponent = CreateDefaultSubobject<USplineMovementComponent>(TEXT("MovementComponent"));

	// Default values
	bAutoStart = true;
	InitialSpeedKmH = 60.0f; // 60 km/h
	VehicleName = TEXT("Test Vehicle");
	StartingRoad = nullptr;

	// Transition defaults
	bAutoTransition = true; // Auto-transition enabled by default
	TransitionMode = ETransitionMode::Random; // Random by default
	bUseIntersections = true; // Use intersections by default
	IntersectionSearchRadius = 1000.0f; // 10 meters

	// Transition curve state
	CurrentTransitionCurve = nullptr;
	PendingTargetRoad = nullptr;
	bFollowingTransitionCurve = false;
}

void ATestVehicle::BeginPlay()
{
	Super::BeginPlay();

	// Bind events
	MovementComponent->OnReachedEnd.AddDynamic(this, &ATestVehicle::OnReachedEndOfRoad);
	MovementComponent->OnSpeedChanged.AddDynamic(this, &ATestVehicle::OnSpeedChanged);

	// Auto-start if configured
	if (bAutoStart && StartingRoad)
	{
		AssignToRoad(StartingRoad);
	}
}

void ATestVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATestVehicle::AssignToRoad(ARoadSplineActor* Road)
{
	if (!Road)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestVehicle '%s': Cannot assign to null road"), *VehicleName);
		return;
	}

	// Set speed
	MovementComponent->SetSpeedKmH(InitialSpeedKmH);

	// Start following road
	MovementComponent->StartFollowingSpline(Road);
}

void ATestVehicle::StopVehicle()
{
	MovementComponent->StopMovement();
}

void ATestVehicle::ResumeVehicle()
{
	MovementComponent->ResumeMovement();
}

void ATestVehicle::SetVehicleSpeed(float SpeedKmH)
{
	MovementComponent->SetSpeedKmH(SpeedKmH);
}

bool ATestVehicle::IsMoving() const
{
	return MovementComponent && MovementComponent->bIsMoving;
}

float ATestVehicle::GetCurrentSpeed() const
{
	return MovementComponent ? MovementComponent->GetSpeedKmH() : 0.0f;
}

float ATestVehicle::GetProgress() const
{
	return MovementComponent ? MovementComponent->GetProgressPercent() : 0.0f;
}

void ATestVehicle::OnReachedEndOfRoad()
{
	// Auto-transition if enabled
	if (!bAutoTransition)
	{
		// Auto-transition disabled, vehicle stops
		return;
	}

	// Get current road
	ARoadSplineActor* CurrentRoad = MovementComponent->CurrentRoad;
	if (!CurrentRoad)
	{
		// No current road, can't transition
		return;
	}

	// Try to use RoadIntersection if enabled
	if (bUseIntersections)
	{
		ARoadIntersection* Intersection = FindNearbyIntersection();
		if (Intersection)
		{
			// Use intersection for smart transition
			if (TransitionThroughIntersection(Intersection, CurrentRoad))
			{
				// Successfully transitioned through intersection
				return;
			}
			// If failed, fall through to normal transition
		}
	}

	// Fallback: Normal transition (no intersection)
	// Get connected roads at the end
	TArray<ARoadSplineActor*> ConnectedRoads = CurrentRoad->GetRoadsAtEnd();

	if (ConnectedRoads.Num() == 0)
	{
		// No connected roads, vehicle stops at end
		return;
	}

	// Choose next road based on transition mode
	ARoadSplineActor* NextRoad = ChooseNextRoad(ConnectedRoads);

	if (NextRoad)
	{
		// Switch to next road, maintaining speed
		MovementComponent->SwitchToNewSpline(NextRoad, true);
	}
}

ARoadSplineActor* ATestVehicle::ChooseNextRoad(const TArray<ARoadSplineActor*>& ConnectedRoads)
{
	if (ConnectedRoads.Num() == 0)
	{
		return nullptr;
	}

	switch (TransitionMode)
	{
	case ETransitionMode::Random:
		// Pick random road
		{
			int32 RandomIndex = FMath::RandRange(0, ConnectedRoads.Num() - 1);
			return ConnectedRoads[RandomIndex];
		}

	case ETransitionMode::First:
		// Always pick first road
		return ConnectedRoads[0];

	case ETransitionMode::Last:
		// Always pick last road
		return ConnectedRoads[ConnectedRoads.Num() - 1];

	default:
		// Fallback to first
		return ConnectedRoads[0];
	}
}

void ATestVehicle::OnSpeedChanged(float NewSpeedKmH)
{
	// Event handler for speed changes
	// Can be used to update UI or trigger other events
}

ARoadIntersection* ATestVehicle::FindNearbyIntersection() const
{
	if (!MovementComponent || !MovementComponent->CurrentRoad)
	{
		return nullptr;
	}

	// Get current road end position
	ARoadSplineActor* CurrentRoad = MovementComponent->CurrentRoad;
	FVector RoadEndPoint = CurrentRoad->RoadSpline->GetLocationAtDistanceAlongSpline(
		CurrentRoad->RoadSpline->GetSplineLength(),
		ESplineCoordinateSpace::World
	);

	// Find all RoadIntersection actors in level
	TArray<AActor*> FoundIntersections;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoadIntersection::StaticClass(), FoundIntersections);

	// Find closest intersection within search radius
	ARoadIntersection* ClosestIntersection = nullptr;
	float ClosestDistance = IntersectionSearchRadius;

	for (AActor* Actor : FoundIntersections)
	{
		ARoadIntersection* Intersection = Cast<ARoadIntersection>(Actor);
		if (Intersection)
		{
			float Distance = FVector::Dist(RoadEndPoint, Intersection->GetActorLocation());
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestIntersection = Intersection;
			}
		}
	}

	if (ClosestIntersection)
	{
		UE_LOG(LogTemp, Log, TEXT("TestVehicle '%s': Found intersection '%s' at distance %.0f cm"),
			*VehicleName, *ClosestIntersection->IntersectionName, ClosestDistance);
	}

	return ClosestIntersection;
}

bool ATestVehicle::TransitionThroughIntersection(ARoadIntersection* Intersection, ARoadSplineActor* FromRoad)
{
	if (!Intersection || !FromRoad || !MovementComponent)
	{
		return false;
	}

	// Get next road from intersection based on transition mode
	ARoadSplineActor* NextRoad = Intersection->ChooseNextRoad(FromRoad, TransitionMode);

	if (!NextRoad)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestVehicle '%s': No outgoing roads from intersection '%s'"),
			*VehicleName, *Intersection->IntersectionName);
		return false;
	}

	// Generate transition curve
	USplineComponent* TransitionCurve = Intersection->GenerateTransitionCurve(FromRoad, NextRoad);

	if (!TransitionCurve)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestVehicle '%s': Failed to generate transition curve"),
			*VehicleName);
		return false;
	}

	// Store pending target road
	PendingTargetRoad = NextRoad;
	CurrentTransitionCurve = TransitionCurve;
	bFollowingTransitionCurve = true;

	// Start following transition curve
	MovementComponent->SwitchToNewSplineComponent(TransitionCurve, true);

	// Bind to OnReachedEnd to detect when curve is complete
	MovementComponent->OnReachedEnd.AddUniqueDynamic(this, &ATestVehicle::OnTransitionCurveComplete);

	UE_LOG(LogTemp, Log, TEXT("TestVehicle '%s': Following transition curve from '%s' to '%s'"),
		*VehicleName, *FromRoad->RoadName, *NextRoad->RoadName);

	return true;
}

void ATestVehicle::OnTransitionCurveComplete()
{
	if (!bFollowingTransitionCurve || !PendingTargetRoad || !MovementComponent)
	{
		return;
	}

	// Unbind from curve end event
	MovementComponent->OnReachedEnd.RemoveDynamic(this, &ATestVehicle::OnTransitionCurveComplete);

	// Switch to target road
	MovementComponent->SwitchToNewSpline(PendingTargetRoad, true);

	UE_LOG(LogTemp, Log, TEXT("TestVehicle '%s': Transition curve complete, now on road '%s'"),
		*VehicleName, *PendingTargetRoad->RoadName);

	// Cleanup
	if (CurrentTransitionCurve)
	{
		CurrentTransitionCurve->DestroyComponent();
		CurrentTransitionCurve = nullptr;
	}

	PendingTargetRoad = nullptr;
	bFollowingTransitionCurve = false;

	// Re-bind to normal OnReachedEnd handler
	MovementComponent->OnReachedEnd.AddUniqueDynamic(this, &ATestVehicle::OnReachedEndOfRoad);
}
