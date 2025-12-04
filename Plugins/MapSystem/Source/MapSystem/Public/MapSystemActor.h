// Copyright Ai27. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapCaptureComponent.h"
#include "MapTypes.h"
#include "MapSystemActor.generated.h"

class UMapWidget;

/**
 * Main actor for the Map System.
 * Place this in your level to enable the interactive map functionality.
 * This actor manages the scene capture and provides an easy interface for Blueprints.
 */
UCLASS(Blueprintable, BlueprintType)
class MAPSYSTEM_API AMapSystemActor : public AActor
{
	GENERATED_BODY()

public:
	AMapSystemActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ==================== Components ====================

	/** The map capture component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map System")
	TObjectPtr<UMapCaptureComponent> MapCaptureComponent;

	// ==================== Configuration ====================

	/** Widget class to spawn for the map UI (optional - can create your own) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map System|UI")
	TSubclassOf<UMapWidget> MapWidgetClass;

	/** Should the map widget be created automatically on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map System|UI")
	bool bAutoCreateWidget = false;

	/** Z-order for the map widget when auto-created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map System|UI", meta = (EditCondition = "bAutoCreateWidget"))
	int32 WidgetZOrder = 0;

	// ==================== Runtime Properties ====================

	/** Reference to the spawned map widget (if auto-created or manually set) */
	UPROPERTY(BlueprintReadOnly, Category = "Map System|Runtime")
	TObjectPtr<UMapWidget> MapWidget;

	// ==================== Initialization Functions ====================

	/** Initialize the map system. Call this if you're setting up manually. */
	UFUNCTION(BlueprintCallable, Category = "Map System")
	void InitializeMapSystem();

	/** Create and display the map widget */
	UFUNCTION(BlueprintCallable, Category = "Map System|UI")
	UMapWidget* CreateMapWidget();

	/** Set an externally created map widget */
	UFUNCTION(BlueprintCallable, Category = "Map System|UI")
	void SetMapWidget(UMapWidget* InWidget);

	/** Get the map capture component */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System")
	UMapCaptureComponent* GetMapCapture() const { return MapCaptureComponent; }

	/** Get the render target texture for use in custom UIs */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System")
	UTextureRenderTarget2D* GetMapTexture() const;

	// ==================== Map Control ====================

	/** Set the center of the map view */
	UFUNCTION(BlueprintCallable, Category = "Map System|Control")
	void SetMapCenter(FVector2D WorldCenter);

	/** Set the zoom level */
	UFUNCTION(BlueprintCallable, Category = "Map System|Control")
	void SetZoom(float NewZoom);

	/** Get current zoom level */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Control")
	float GetZoom() const;

	/** Pan the map by a delta */
	UFUNCTION(BlueprintCallable, Category = "Map System|Control")
	void PanMap(FVector2D Delta);

	// ==================== Marker Management ====================

	/** Add origin marker at world position */
	UFUNCTION(BlueprintCallable, Category = "Map System|Markers")
	FName AddOriginMarker(FVector WorldPosition);

	/** Add destination marker at world position */
	UFUNCTION(BlueprintCallable, Category = "Map System|Markers")
	FName AddDestinationMarker(FVector WorldPosition);

	/** Add custom marker */
	UFUNCTION(BlueprintCallable, Category = "Map System|Markers")
	bool AddMarker(const FMapMarkerData& MarkerData);

	/** Remove a marker by ID */
	UFUNCTION(BlueprintCallable, Category = "Map System|Markers")
	bool RemoveMarker(FName MarkerId);

	/** Get marker data by ID */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Markers")
	bool GetMarkerData(FName MarkerId, FMapMarkerData& OutData) const;

	/** Set marker world position */
	UFUNCTION(BlueprintCallable, Category = "Map System|Markers")
	bool SetMarkerPosition(FName MarkerId, FVector NewPosition);

	/** Get all markers */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Markers")
	TArray<FMapMarkerData> GetAllMarkers() const;

	/** Clear all markers */
	UFUNCTION(BlueprintCallable, Category = "Map System|Markers")
	void ClearAllMarkers();

	// ==================== Coordinate Conversion ====================

	/** Convert world position to UV on the map (0-1) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Conversion")
	FVector2D WorldToMapUV(FVector WorldPosition) const;

	/** Convert map UV to world position */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map System|Conversion")
	FVector MapUVToWorld(FVector2D UV) const;

	// ==================== Validation ====================

	/** Check if a world position is valid for placement */
	UFUNCTION(BlueprintCallable, Category = "Map System|Validation")
	bool IsValidPosition(FVector WorldPosition, FVector& OutValidPosition) const;

	/** Find valid snap position from map UV */
	UFUNCTION(BlueprintCallable, Category = "Map System|Validation")
	bool FindSnapPosition(FVector2D MapUV, FVector& OutWorldPosition) const;

private:
	bool bIsInitialized = false;
};
