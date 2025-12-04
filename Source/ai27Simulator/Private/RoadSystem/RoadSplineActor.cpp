// Copyright © 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista

#include "RoadSystem/RoadSplineActor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

ARoadSplineActor::ARoadSplineActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Create spline component
	RoadSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RoadSpline"));
	RoadSpline->SetupAttachment(RootComponent);
	RoadSpline->SetClosedLoop(false);

	// Default values
	RoadWidth = 800.0f;           // 8 meters
	NumLanes = 2;
	SpeedLimit = 80.0f;           // 80 km/h
	bIsHighway = false;
	bIsRiskZone = false;
	RoadName = TEXT("Road");

	// Visual
	bGenerateRoadMesh = false;     // Disabled by default (can be slow)
	RoadMeshSegment = nullptr;
	RoadMaterial = nullptr;
	RoadColor = FLinearColor::Gray;

	// Debug
	bShowDebugSpline = false;
	bShowRoadBounds = false;
}

void ARoadSplineActor::BeginPlay()
{
	Super::BeginPlay();

	// Log road info
	UE_LOG(LogTemp, Log, TEXT("RoadSplineActor '%s': Length=%.0f cm, Lanes=%d, Speed=%.0f km/h"),
		*RoadName, GetSplineLength(), NumLanes, SpeedLimit);
}

void ARoadSplineActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Update visual representation if needed
	if (bGenerateRoadMesh && RoadMeshSegment)
	{
		GenerateRoadMesh();
	}
	else
	{
		ClearRoadMesh();
	}

	// Update spline color for risk zones
	if (bIsRiskZone)
	{
		RoadColor = FLinearColor::Red;
	}
	else
	{
		RoadColor = FLinearColor::Gray;
	}
}

#if WITH_EDITOR
void ARoadSplineActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr)
		? PropertyChangedEvent.Property->GetFName()
		: NAME_None;

	// Regenerate mesh if relevant properties changed
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ARoadSplineActor, bGenerateRoadMesh) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(ARoadSplineActor, RoadMeshSegment) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(ARoadSplineActor, RoadWidth))
	{
		if (bGenerateRoadMesh && RoadMeshSegment)
		{
			GenerateRoadMesh();
		}
		else
		{
			ClearRoadMesh();
		}
	}
}
#endif

void ARoadSplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Debug visualization
	if (bShowDebugSpline)
	{
		RoadSpline->bDrawDebug = true;
	}
	else
	{
		RoadSpline->bDrawDebug = false;
	}
}

FVector ARoadSplineActor::GetLocationAtDistance(float Distance) const
{
	if (!RoadSpline)
		return FVector::ZeroVector;

	return RoadSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FVector ARoadSplineActor::GetLocationAtTime(float Time) const
{
	if (!RoadSpline)
		return FVector::ZeroVector;

	float Distance = Time * GetSplineLength();
	return GetLocationAtDistance(Distance);
}

FRotator ARoadSplineActor::GetRotationAtDistance(float Distance) const
{
	if (!RoadSpline)
		return FRotator::ZeroRotator;

	return RoadSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

float ARoadSplineActor::GetSplineLength() const
{
	if (!RoadSpline)
		return 0.0f;

	return RoadSpline->GetSplineLength();
}

FVector ARoadSplineActor::GetClosestLocationOnSpline(const FVector& WorldLocation, float& OutDistance) const
{
	if (!RoadSpline)
	{
		OutDistance = 0.0f;
		return FVector::ZeroVector;
	}

	float InputKey = RoadSpline->FindInputKeyClosestToWorldLocation(WorldLocation);
	OutDistance = RoadSpline->GetDistanceAlongSplineAtSplineInputKey(InputKey);

	return RoadSpline->GetLocationAtDistanceAlongSpline(OutDistance, ESplineCoordinateSpace::World);
}

bool ARoadSplineActor::IsLocationOnRoad(const FVector& WorldLocation, float Tolerance) const
{
	if (!RoadSpline)
		return false;

	float Distance;
	FVector ClosestPoint = GetClosestLocationOnSpline(WorldLocation, Distance);

	float DistanceToRoad = FVector::Dist(WorldLocation, ClosestPoint);

	return DistanceToRoad <= (RoadWidth * 0.5f + Tolerance);
}

void ARoadSplineActor::ConnectToRoad(ARoadSplineActor* OtherRoad, bool bAtStart)
{
	if (!OtherRoad)
	{
		UE_LOG(LogTemp, Warning, TEXT("RoadSplineActor: Cannot connect to null road"));
		return;
	}

	// Add to connections array
	if (!ConnectedRoads.Contains(OtherRoad))
	{
		ConnectedRoads.Add(OtherRoad);

		// Track connection details
		FRoadConnection Connection;
		Connection.ConnectedRoad = OtherRoad;
		Connection.bConnectedAtStart = bAtStart;
		Connections.Add(Connection);

		UE_LOG(LogTemp, Log, TEXT("RoadSplineActor '%s' connected to '%s' at %s"),
			*RoadName, *OtherRoad->RoadName, bAtStart ? TEXT("START") : TEXT("END"));
	}

	// Make connection bidirectional
	if (!OtherRoad->ConnectedRoads.Contains(this))
	{
		OtherRoad->ConnectedRoads.Add(this);
	}
}

TArray<ARoadSplineActor*> ARoadSplineActor::GetRoadsAtStart() const
{
	TArray<ARoadSplineActor*> RoadsAtStart;

	// Get roads explicitly connected at the start via ConnectToRoad()
	for (const FRoadConnection& Connection : Connections)
	{
		if (Connection.bConnectedAtStart)
		{
			RoadsAtStart.Add(Connection.ConnectedRoad);
		}
	}

	// Note: ConnectedRoads added manually in editor are assumed to be at the END
	// So we don't add them here to avoid duplication

	return RoadsAtStart;
}

TArray<ARoadSplineActor*> ARoadSplineActor::GetRoadsAtEnd() const
{
	TArray<ARoadSplineActor*> RoadsAtEnd;

	// First, get roads explicitly connected at the end via ConnectToRoad()
	for (const FRoadConnection& Connection : Connections)
	{
		if (!Connection.bConnectedAtStart)
		{
			RoadsAtEnd.Add(Connection.ConnectedRoad);
		}
	}

	// Also include roads from ConnectedRoads array (manually added in editor)
	// Assume they are connected at the end if not explicitly defined
	for (ARoadSplineActor* Road : ConnectedRoads)
	{
		if (Road && !RoadsAtEnd.Contains(Road))
		{
			// Check if this road is already in Connections array
			bool bAlreadyInConnections = false;
			for (const FRoadConnection& Connection : Connections)
			{
				if (Connection.ConnectedRoad == Road)
				{
					bAlreadyInConnections = true;
					break;
				}
			}

			// If not in Connections, assume it's connected at the end
			if (!bAlreadyInConnections)
			{
				RoadsAtEnd.Add(Road);
			}
		}
	}

	return RoadsAtEnd;
}

void ARoadSplineActor::GenerateRoadMesh()
{
	// Clear existing meshes
	ClearRoadMesh();

	if (!RoadMeshSegment)
	{
		UE_LOG(LogTemp, Warning, TEXT("RoadSplineActor: No RoadMeshSegment set"));
		return;
	}

	int32 NumSegments = FMath::Max(1, FMath::FloorToInt(GetSplineLength() / 1000.0f)); // Segment every 10 meters

	for (int32 i = 0; i < NumSegments; ++i)
	{
		// Create spline mesh component
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
		SplineMesh->SetStaticMesh(RoadMeshSegment);

		if (RoadMaterial)
		{
			SplineMesh->SetMaterial(0, RoadMaterial);
		}

		SplineMesh->RegisterComponent();
		SplineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

		// Calculate positions
		float StartDistance = (float)i / NumSegments * GetSplineLength();
		float EndDistance = (float)(i + 1) / NumSegments * GetSplineLength();

		FVector StartPos = RoadSpline->GetLocationAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::Local);
		FVector StartTangent = RoadSpline->GetTangentAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::Local);

		FVector EndPos = RoadSpline->GetLocationAtDistanceAlongSpline(EndDistance, ESplineCoordinateSpace::Local);
		FVector EndTangent = RoadSpline->GetTangentAtDistanceAlongSpline(EndDistance, ESplineCoordinateSpace::Local);

		// Set spline mesh data
		SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);

		// Scale to road width
		FVector2D Scale(RoadWidth / 100.0f, 1.0f); // Adjust based on mesh size
		SplineMesh->SetStartScale(Scale);
		SplineMesh->SetEndScale(Scale);

		// Add to array
		SplineMeshComponents.Add(SplineMesh);
	}

	UE_LOG(LogTemp, Log, TEXT("RoadSplineActor: Generated %d mesh segments"), NumSegments);
}

void ARoadSplineActor::ClearRoadMesh()
{
	for (USplineMeshComponent* Mesh : SplineMeshComponents)
	{
		if (Mesh)
		{
			Mesh->DestroyComponent();
		}
	}

	SplineMeshComponents.Empty();
}
