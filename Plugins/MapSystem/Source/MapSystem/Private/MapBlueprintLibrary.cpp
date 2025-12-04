// Copyright Ai27. All Rights Reserved.

#include "MapBlueprintLibrary.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

FMapMarkerData UMapBlueprintLibrary::MakeOriginMarker(FName MarkerId, FVector WorldPosition)
{
	FMapMarkerData Marker(MarkerId, EMapMarkerType::Origin);
	Marker.WorldPosition = WorldPosition;
	Marker.Color = FLinearColor::Green;
	Marker.Label = FText::FromString(TEXT("Origin"));
	return Marker;
}

FMapMarkerData UMapBlueprintLibrary::MakeDestinationMarker(FName MarkerId, FVector WorldPosition)
{
	FMapMarkerData Marker(MarkerId, EMapMarkerType::Destination);
	Marker.WorldPosition = WorldPosition;
	Marker.Color = FLinearColor::Red;
	Marker.Label = FText::FromString(TEXT("Destination"));
	return Marker;
}

FMapMarkerData UMapBlueprintLibrary::MakeCustomMarker(FName MarkerId, FVector WorldPosition, FLinearColor Color, FText Label)
{
	FMapMarkerData Marker(MarkerId, EMapMarkerType::Custom);
	Marker.WorldPosition = WorldPosition;
	Marker.Color = Color;
	Marker.Label = Label;
	return Marker;
}

float UMapBlueprintLibrary::GetDistance2D(FVector PositionA, FVector PositionB)
{
	return FVector2D::Distance(
		FVector2D(PositionA.X, PositionA.Y),
		FVector2D(PositionB.X, PositionB.Y)
	);
}

FVector UMapBlueprintLibrary::GetDirection2D(FVector Origin, FVector Destination)
{
	FVector Direction = Destination - Origin;
	Direction.Z = 0.0f;
	return Direction.GetSafeNormal();
}

bool UMapBlueprintLibrary::IsPointInBounds2D(FVector2D Point, FVector2D MinBounds, FVector2D MaxBounds)
{
	return Point.X >= MinBounds.X && Point.X <= MaxBounds.X &&
	       Point.Y >= MinBounds.Y && Point.Y <= MaxBounds.Y;
}

FMapConfiguration UMapBlueprintLibrary::GetDefaultMapConfiguration()
{
	return FMapConfiguration();
}

FMapConfiguration UMapBlueprintLibrary::GetReadOnlyMapConfiguration()
{
	FMapConfiguration Config;
	Config.bAllowPanning = false;
	Config.bAllowZooming = false;
	Config.bAllowMarkerDragging = false;
	return Config;
}

bool UMapBlueprintLibrary::TraceForValidPosition(
	const UObject* WorldContextObject,
	FVector WorldPosition,
	float TraceHeight,
	float TraceDistance,
	ECollisionChannel TraceChannel,
	FVector& OutHitPosition)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	FVector TraceStart = WorldPosition;
	TraceStart.Z = TraceHeight;

	FVector TraceEnd = WorldPosition;
	TraceEnd.Z = -TraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		TraceChannel,
		QueryParams
	);

	if (bHit)
	{
		OutHitPosition = HitResult.Location;
		return true;
	}

	return false;
}

bool UMapBlueprintLibrary::FindNearestValidPosition(
	const UObject* WorldContextObject,
	FVector WorldPosition,
	float SearchRadius,
	int32 NumSamples,
	float TraceHeight,
	ECollisionChannel TraceChannel,
	FVector& OutValidPosition)
{
	// First try the exact position
	if (TraceForValidPosition(WorldContextObject, WorldPosition, TraceHeight, TraceHeight * 2.0f, TraceChannel, OutValidPosition))
	{
		return true;
	}

	// Search in a spiral pattern
	float BestDistance = TNumericLimits<float>::Max();
	FVector BestPosition = FVector::ZeroVector;
	bool bFoundValid = false;

	for (int32 i = 0; i < NumSamples; ++i)
	{
		float Angle = (float(i) / float(NumSamples)) * 2.0f * PI;
		float Radius = SearchRadius * (float(i) / float(NumSamples));

		FVector SamplePos = WorldPosition;
		SamplePos.X += FMath::Cos(Angle) * Radius;
		SamplePos.Y += FMath::Sin(Angle) * Radius;

		FVector HitPos;
		if (TraceForValidPosition(WorldContextObject, SamplePos, TraceHeight, TraceHeight * 2.0f, TraceChannel, HitPos))
		{
			float Distance = FVector::Dist(WorldPosition, HitPos);
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestPosition = HitPos;
				bFoundValid = true;
			}
		}
	}

	if (bFoundValid)
	{
		OutValidPosition = BestPosition;
		return true;
	}

	return false;
}
