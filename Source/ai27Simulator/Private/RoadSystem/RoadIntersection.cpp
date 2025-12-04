// Copyright © 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista

#include "RoadSystem/RoadIntersection.h"
#include "RoadSystem/RoadSplineActor.h"
#include "Vehicles/TestVehicle.h"
#include "Components/SplineComponent.h"
#include "Components/BillboardComponent.h"
#include "DrawDebugHelpers.h"

ARoadIntersection::ARoadIntersection()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Create billboard for editor visibility
	IntersectionIcon = CreateDefaultSubobject<UBillboardComponent>(TEXT("IntersectionIcon"));
	IntersectionIcon->SetupAttachment(RootComponent);

	// Default values
	IntersectionName = TEXT("Intersection");
	IntersectionRadius = 500.0f; // 5 meters
	IntersectionType = EIntersectionType::FourWay;

	// Debug
	bShowDebugConnections = true;
	bShowConnectionAngles = false;
}

void ARoadIntersection::BeginPlay()
{
	Super::BeginPlay();

	// Calculate connection points on start
	UpdateConnectionPoints();

	UE_LOG(LogTemp, Log, TEXT("RoadIntersection '%s': %d connections"), *IntersectionName, Connections.Num());
}

void ARoadIntersection::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Update connection points when placing/moving
	UpdateConnectionPoints();
}

#if WITH_EDITOR
void ARoadIntersection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr)
		? PropertyChangedEvent.Property->GetFName()
		: NAME_None;

	// Recalculate connection points if connections changed
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ARoadIntersection, Connections) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(ARoadIntersection, IntersectionRadius))
	{
		UpdateConnectionPoints();
	}
}
#endif

void ARoadIntersection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Debug visualization
	if (bShowDebugConnections)
	{
		FVector Center = GetActorLocation();

		// Draw intersection circle
		DrawDebugCircle(GetWorld(), Center, IntersectionRadius, 32, FColor::Yellow, false, -1.0f, 0, 10.0f, FVector(0, 1, 0), FVector(1, 0, 0), false);

		// Draw connections
		for (const FRoadConnectionPoint& Connection : Connections)
		{
			if (Connection.Road)
			{
				// Color based on connection type
				FColor LineColor = FColor::Green;
				if (Connection.ConnectionType == EConnectionType::Incoming)
				{
					LineColor = FColor::Red;
				}
				else if (Connection.ConnectionType == EConnectionType::Outgoing)
				{
					LineColor = FColor::Blue;
				}

				// Draw line from center to connection point
				DrawDebugLine(GetWorld(), Center, Connection.ConnectionPoint, LineColor, false, -1.0f, 0, 15.0f);

				// Draw sphere at connection point
				DrawDebugSphere(GetWorld(), Connection.ConnectionPoint, 50.0f, 8, LineColor, false, -1.0f, 0, 5.0f);

				// Show angle if enabled
				if (bShowConnectionAngles)
				{
					FString AngleText = FString::Printf(TEXT("%.0f°"), Connection.ConnectionAngle);
					DrawDebugString(GetWorld(), Connection.ConnectionPoint + FVector(0, 0, 100), AngleText, nullptr, FColor::White, -1.0f, true);
				}
			}
		}
	}
}

void ARoadIntersection::UpdateConnectionPoints()
{
	FVector IntersectionCenter = GetActorLocation();

	for (FRoadConnectionPoint& Connection : Connections)
	{
		if (!Connection.Road || !Connection.Road->RoadSpline)
		{
			continue;
		}

		// Get connection point on road
		if (Connection.bConnectedAtStart)
		{
			Connection.ConnectionPoint = Connection.Road->RoadSpline->GetLocationAtDistanceAlongSpline(
				0.0f,
				ESplineCoordinateSpace::World
			);
		}
		else
		{
			Connection.ConnectionPoint = Connection.Road->RoadSpline->GetLocationAtDistanceAlongSpline(
				Connection.Road->RoadSpline->GetSplineLength(),
				ESplineCoordinateSpace::World
			);
		}

		// Calculate angle from center to connection point (in XY plane)
		FVector DirectionToConnection = Connection.ConnectionPoint - IntersectionCenter;
		DirectionToConnection.Z = 0.0f; // Project to XY plane
		DirectionToConnection.Normalize();

		// Calculate angle in degrees (0 = North, 90 = East, etc.)
		float AngleRadians = FMath::Atan2(DirectionToConnection.Y, DirectionToConnection.X);
		Connection.ConnectionAngle = FMath::RadiansToDegrees(AngleRadians);

		// Normalize to 0-360
		if (Connection.ConnectionAngle < 0.0f)
		{
			Connection.ConnectionAngle += 360.0f;
		}
	}

	// Sort connections by angle for easier debugging
	Connections.Sort([](const FRoadConnectionPoint& A, const FRoadConnectionPoint& B)
	{
		return A.ConnectionAngle < B.ConnectionAngle;
	});
}

TArray<ARoadSplineActor*> ARoadIntersection::GetOutgoingRoads(ARoadSplineActor* IncomingRoad) const
{
	TArray<ARoadSplineActor*> OutgoingRoads;

	if (!IncomingRoad)
	{
		return OutgoingRoads;
	}

	// Find incoming connection
	const FRoadConnectionPoint* IncomingConnection = FindConnection(IncomingRoad);
	if (!IncomingConnection)
	{
		UE_LOG(LogTemp, Warning, TEXT("RoadIntersection '%s': Road '%s' is not connected to this intersection"),
			*IntersectionName, *IncomingRoad->RoadName);
		return OutgoingRoads;
	}

	// Get all outgoing or bidirectional roads (excluding the incoming road)
	for (const FRoadConnectionPoint& Connection : Connections)
	{
		if (Connection.Road != IncomingRoad &&
			(Connection.ConnectionType == EConnectionType::Outgoing ||
			 Connection.ConnectionType == EConnectionType::Bidirectional))
		{
			OutgoingRoads.Add(Connection.Road);
		}
	}

	return OutgoingRoads;
}

ARoadSplineActor* ARoadIntersection::ChooseNextRoad(ARoadSplineActor* IncomingRoad, TEnumAsByte<enum ETransitionMode> TransitionMode) const
{
	TArray<ARoadSplineActor*> OutgoingRoads = GetOutgoingRoads(IncomingRoad);

	if (OutgoingRoads.Num() == 0)
	{
		return nullptr;
	}

	// Choose based on mode
	switch (TransitionMode)
	{
	case ETransitionMode::Random:
		{
			int32 RandomIndex = FMath::RandRange(0, OutgoingRoads.Num() - 1);
			return OutgoingRoads[RandomIndex];
		}

	case ETransitionMode::First:
		return OutgoingRoads[0];

	case ETransitionMode::Last:
		return OutgoingRoads[OutgoingRoads.Num() - 1];

	default:
		return OutgoingRoads[0];
	}
}

USplineComponent* ARoadIntersection::GenerateTransitionCurve(ARoadSplineActor* FromRoad, ARoadSplineActor* ToRoad)
{
	if (!FromRoad || !ToRoad || !FromRoad->RoadSpline || !ToRoad->RoadSpline)
	{
		return nullptr;
	}

	// Create temporary spline component
	USplineComponent* TransitionSpline = NewObject<USplineComponent>(this);
	TransitionSpline->RegisterComponent();
	TransitionSpline->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	// Find connection points
	const FRoadConnectionPoint* FromConnection = FindConnection(FromRoad);
	const FRoadConnectionPoint* ToConnection = FindConnection(ToRoad);

	if (!FromConnection || !ToConnection)
	{
		UE_LOG(LogTemp, Warning, TEXT("RoadIntersection: Could not find connection points"));
		TransitionSpline->DestroyComponent();
		return nullptr;
	}

	// Get end point and tangent of FromRoad
	FVector StartPoint = FromConnection->ConnectionPoint;
	FVector StartTangent;

	if (FromConnection->bConnectedAtStart)
	{
		StartTangent = FromRoad->RoadSpline->GetTangentAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
		StartTangent = -StartTangent; // Reverse if coming from start
	}
	else
	{
		StartTangent = FromRoad->RoadSpline->GetTangentAtDistanceAlongSpline(
			FromRoad->RoadSpline->GetSplineLength(),
			ESplineCoordinateSpace::World
		);
	}

	// Get start point and tangent of ToRoad
	FVector EndPoint = ToConnection->ConnectionPoint;
	FVector EndTangent;

	if (ToConnection->bConnectedAtStart)
	{
		EndTangent = ToRoad->RoadSpline->GetTangentAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
	}
	else
	{
		EndTangent = ToRoad->RoadSpline->GetTangentAtDistanceAlongSpline(
			ToRoad->RoadSpline->GetSplineLength(),
			ESplineCoordinateSpace::World
		);
		EndTangent = -EndTangent; // Reverse if connecting to end
	}

	// Normalize tangents and scale by intersection radius for smooth curve
	StartTangent.Normalize();
	EndTangent.Normalize();
	StartTangent *= IntersectionRadius;
	EndTangent *= IntersectionRadius;

	// Clear existing points
	TransitionSpline->ClearSplinePoints();

	// Add two points (start and end) with tangents
	TransitionSpline->AddSplinePoint(StartPoint, ESplineCoordinateSpace::World, false);
	TransitionSpline->AddSplinePoint(EndPoint, ESplineCoordinateSpace::World, false);

	// Set tangents for smooth curve
	TransitionSpline->SetTangentAtSplinePoint(0, StartTangent, ESplineCoordinateSpace::World, false);
	TransitionSpline->SetTangentAtSplinePoint(1, EndTangent, ESplineCoordinateSpace::World, false);

	// Update spline
	TransitionSpline->UpdateSpline();

	// Store for cleanup
	TransitionSplines.Add(TransitionSpline);

	UE_LOG(LogTemp, Log, TEXT("RoadIntersection '%s': Generated transition curve from '%s' to '%s'"),
		*IntersectionName, *FromRoad->RoadName, *ToRoad->RoadName);

	return TransitionSpline;
}

FRoadConnectionPoint* ARoadIntersection::FindConnection(ARoadSplineActor* Road)
{
	for (FRoadConnectionPoint& Connection : Connections)
	{
		if (Connection.Road == Road)
		{
			return &Connection;
		}
	}
	return nullptr;
}

const FRoadConnectionPoint* ARoadIntersection::FindConnection(ARoadSplineActor* Road) const
{
	for (const FRoadConnectionPoint& Connection : Connections)
	{
		if (Connection.Road == Road)
		{
			return &Connection;
		}
	}
	return nullptr;
}
