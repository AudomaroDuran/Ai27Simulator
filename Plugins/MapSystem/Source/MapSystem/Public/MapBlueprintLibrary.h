// Copyright Ai27. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MapTypes.h"
#include "MapBlueprintLibrary.generated.h"

class UMapCaptureComponent;
class UMapWidget;

/**
 * Blueprint function library for the Map System.
 * Provides utility functions for working with the interactive map.
 */
UCLASS()
class MAPSYSTEM_API UMapBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ==================== Marker Creation Helpers ====================

	/** Create a marker data struct for an origin point */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Markers")
	static FMapMarkerData MakeOriginMarker(FName MarkerId, FVector WorldPosition);

	/** Create a marker data struct for a destination point */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Markers")
	static FMapMarkerData MakeDestinationMarker(FName MarkerId, FVector WorldPosition);

	/** Create a custom marker data struct */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Markers")
	static FMapMarkerData MakeCustomMarker(FName MarkerId, FVector WorldPosition, FLinearColor Color, FText Label);

	// ==================== Coordinate Utilities ====================

	/** Calculate distance between two world positions in 2D (XY plane) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Utility")
	static float GetDistance2D(FVector PositionA, FVector PositionB);

	/** Calculate direction from origin to destination (normalized) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Utility")
	static FVector GetDirection2D(FVector Origin, FVector Destination);

	/** Check if a point is within a rectangle defined by min and max bounds */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Utility")
	static bool IsPointInBounds2D(FVector2D Point, FVector2D MinBounds, FVector2D MaxBounds);

	// ==================== Map Configuration Helpers ====================

	/** Create a default map configuration */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Configuration")
	static FMapConfiguration GetDefaultMapConfiguration();

	/** Create a read-only map configuration (no user interaction) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Configuration")
	static FMapConfiguration GetReadOnlyMapConfiguration();

	// ==================== Validation Helpers ====================

	/** Perform a trace from a point to find valid ground position */
	UFUNCTION(BlueprintCallable, Category = "Map System|Validation", meta = (WorldContext = "WorldContextObject"))
	static bool TraceForValidPosition(
		const UObject* WorldContextObject,
		FVector WorldPosition,
		float TraceHeight,
		float TraceDistance,
		ECollisionChannel TraceChannel,
		FVector& OutHitPosition
	);

	/** Perform multiple traces around a point to find nearby valid position */
	UFUNCTION(BlueprintCallable, Category = "Map System|Validation", meta = (WorldContext = "WorldContextObject"))
	static bool FindNearestValidPosition(
		const UObject* WorldContextObject,
		FVector WorldPosition,
		float SearchRadius,
		int32 NumSamples,
		float TraceHeight,
		ECollisionChannel TraceChannel,
		FVector& OutValidPosition
	);
};
