// Copyright Ai27. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MapTypes.generated.h"

/**
 * Type of map marker
 */
UENUM(BlueprintType)
enum class EMapMarkerType : uint8
{
	Origin		UMETA(DisplayName = "Origin"),
	Destination	UMETA(DisplayName = "Destination"),
	Custom		UMETA(DisplayName = "Custom")
};

/**
 * State of a map marker
 */
UENUM(BlueprintType)
enum class EMapMarkerState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Hovered		UMETA(DisplayName = "Hovered"),
	Dragging	UMETA(DisplayName = "Dragging"),
	Invalid		UMETA(DisplayName = "Invalid Position")
};

/**
 * Data structure for a map marker
 */
USTRUCT(BlueprintType)
struct MAPSYSTEM_API FMapMarkerData
{
	GENERATED_BODY()

	/** Unique identifier for this marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FName MarkerId;

	/** Type of marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	EMapMarkerType MarkerType = EMapMarkerType::Custom;

	/** Current state of the marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	EMapMarkerState MarkerState = EMapMarkerState::Idle;

	/** World position of the marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FVector WorldPosition = FVector::ZeroVector;

	/** Whether the current position is valid (on valid geometry) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	bool bIsValidPosition = false;

	/** Whether this marker can be moved by the user */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	bool bIsDraggable = true;

	/** Whether this marker is currently visible */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	bool bIsVisible = true;

	/** Custom label for the marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FText Label;

	/** Custom color for the marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FLinearColor Color = FLinearColor::White;

	/** Icon size in pixels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	float IconSize = 32.0f;

	FMapMarkerData()
	{
		MarkerId = NAME_None;
	}

	FMapMarkerData(FName InId, EMapMarkerType InType)
		: MarkerId(InId)
		, MarkerType(InType)
	{
		// Set default colors based on type
		switch (InType)
		{
		case EMapMarkerType::Origin:
			Color = FLinearColor::Green;
			Label = FText::FromString(TEXT("Origin"));
			break;
		case EMapMarkerType::Destination:
			Color = FLinearColor::Red;
			Label = FText::FromString(TEXT("Destination"));
			break;
		default:
			Color = FLinearColor::White;
			break;
		}
	}

	bool operator==(const FMapMarkerData& Other) const
	{
		return MarkerId == Other.MarkerId;
	}
};

/**
 * Input mode for the map widget
 */
UENUM(BlueprintType)
enum class EMapInputMode : uint8
{
	None			UMETA(DisplayName = "None"),
	Panning			UMETA(DisplayName = "Panning"),
	DraggingMarker	UMETA(DisplayName = "Dragging Marker"),
	PlacingMarker	UMETA(DisplayName = "Placing Marker")
};

/**
 * Configuration for map behavior
 */
USTRUCT(BlueprintType)
struct MAPSYSTEM_API FMapConfiguration
{
	GENERATED_BODY()

	/** Allow user to pan the map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bAllowPanning = true;

	/** Allow user to zoom the map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bAllowZooming = true;

	/** Allow user to move markers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bAllowMarkerDragging = true;

	/** Show marker labels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bShowMarkerLabels = true;

	/** Snap markers to valid positions only */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bSnapToValidPositions = true;

	/** Mouse button for panning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FKey PanButton = EKeys::RightMouseButton;

	/** Mouse button for interacting with markers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FKey MarkerButton = EKeys::LeftMouseButton;

	/** Invert Y axis for panning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bInvertPanY = false;

	/** Zoom sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float ZoomSensitivity = 1.0f;

	/** Pan sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float PanSensitivity = 1.0f;
};
