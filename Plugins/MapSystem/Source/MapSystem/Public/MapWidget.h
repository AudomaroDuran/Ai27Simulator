// Copyright Ai27. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapTypes.h"
#include "MapCaptureComponent.h"
#include "MapWidget.generated.h"

class UImage;
class UCanvasPanel;
class UOverlay;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMarkerMoved, FName, MarkerId, FVector, NewWorldPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMarkerClicked, FName, MarkerId, FVector, WorldPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMarkerStateChanged, FName, MarkerId, EMapMarkerState, OldState, EMapMarkerState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMapClicked, FVector, WorldPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoomChanged, float, OldZoom, float, NewZoom);

/**
 * Main map widget that displays the scene capture and handles user interaction.
 * Provides Google Maps-like functionality with pan, zoom, and markers.
 */
UCLASS()
class MAPSYSTEM_API UMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMapWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

public:
	// ==================== Configuration ====================

	/** Map configuration settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	FMapConfiguration MapConfig;

	/** Hit radius for marker selection in pixels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	float MarkerHitRadius = 20.0f;

	// ==================== Events ====================

	/** Called when a marker is moved to a new position */
	UPROPERTY(BlueprintAssignable, Category = "Map|Events")
	FOnMarkerMoved OnMarkerMoved;

	/** Called when a marker is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Map|Events")
	FOnMarkerClicked OnMarkerClicked;

	/** Called when a marker's state changes */
	UPROPERTY(BlueprintAssignable, Category = "Map|Events")
	FOnMarkerStateChanged OnMarkerStateChanged;

	/** Called when the map is clicked (not on a marker) */
	UPROPERTY(BlueprintAssignable, Category = "Map|Events")
	FOnMapClicked OnMapClicked;

	/** Called when zoom level changes */
	UPROPERTY(BlueprintAssignable, Category = "Map|Events")
	FOnZoomChanged OnZoomChanged;

	// ==================== Setup Functions ====================

	/** Initialize the widget with a map capture component */
	UFUNCTION(BlueprintCallable, Category = "Map")
	void InitializeMap(UMapCaptureComponent* InMapCapture);

	/** Set the map image widget (should be bound to the render target) */
	UFUNCTION(BlueprintCallable, Category = "Map")
	void SetMapImage(UImage* InMapImage);

	/** Set the canvas panel for marker widgets */
	UFUNCTION(BlueprintCallable, Category = "Map")
	void SetMarkerCanvas(UCanvasPanel* InMarkerCanvas);

	// ==================== Marker Management ====================

	/** Add a new marker to the map */
	UFUNCTION(BlueprintCallable, Category = "Map|Markers")
	bool AddMarker(const FMapMarkerData& MarkerData);

	/** Remove a marker from the map */
	UFUNCTION(BlueprintCallable, Category = "Map|Markers")
	bool RemoveMarker(FName MarkerId);

	/** Update an existing marker's data */
	UFUNCTION(BlueprintCallable, Category = "Map|Markers")
	bool UpdateMarker(const FMapMarkerData& MarkerData);

	/** Get marker data by ID */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map|Markers")
	bool GetMarker(FName MarkerId, FMapMarkerData& OutMarkerData) const;

	/** Get all markers */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map|Markers")
	TArray<FMapMarkerData> GetAllMarkers() const;

	/** Set marker world position */
	UFUNCTION(BlueprintCallable, Category = "Map|Markers")
	bool SetMarkerWorldPosition(FName MarkerId, FVector NewWorldPosition, bool bValidatePosition = true);

	/** Clear all markers */
	UFUNCTION(BlueprintCallable, Category = "Map|Markers")
	void ClearAllMarkers();

	/** Create origin marker with default settings */
	UFUNCTION(BlueprintCallable, Category = "Map|Markers")
	FName CreateOriginMarker(FVector WorldPosition);

	/** Create destination marker with default settings */
	UFUNCTION(BlueprintCallable, Category = "Map|Markers")
	FName CreateDestinationMarker(FVector WorldPosition);

	// ==================== Map Control ====================

	/** Center map on a world position */
	UFUNCTION(BlueprintCallable, Category = "Map|Control")
	void CenterOnWorldPosition(FVector WorldPosition);

	/** Center map on a marker */
	UFUNCTION(BlueprintCallable, Category = "Map|Control")
	void CenterOnMarker(FName MarkerId);

	/** Fit all markers in view */
	UFUNCTION(BlueprintCallable, Category = "Map|Control")
	void FitMarkersInView(float ViewPadding = 0.2f);

	/** Set zoom level */
	UFUNCTION(BlueprintCallable, Category = "Map|Control")
	void SetZoom(float NewZoom);

	/** Get current zoom level */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map|Control")
	float GetZoom() const;

	// ==================== Coordinate Conversion ====================

	/** Convert local widget position to world position */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map|Conversion")
	FVector LocalToWorld(FVector2D LocalPosition) const;

	/** Convert world position to local widget position */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map|Conversion")
	FVector2D WorldToLocal(FVector WorldPosition) const;

	/** Convert local widget position to map UV */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map|Conversion")
	FVector2D LocalToMapUV(FVector2D LocalPosition) const;

	// ==================== Query Functions ====================

	/** Get marker at screen position, if any */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map|Query")
	bool GetMarkerAtPosition(FVector2D LocalPosition, FMapMarkerData& OutMarkerData) const;

	/** Check if a position on the map is valid for marker placement */
	UFUNCTION(BlueprintCallable, Category = "Map|Query")
	bool IsValidMarkerPosition(FVector2D LocalPosition, FVector& OutWorldPosition) const;

protected:
	/** Reference to the map capture component */
	UPROPERTY()
	TObjectPtr<UMapCaptureComponent> MapCaptureComponent;

	/** The image widget displaying the map */
	UPROPERTY()
	TObjectPtr<UImage> MapImage;

	/** Canvas panel for marker widgets */
	UPROPERTY()
	TObjectPtr<UCanvasPanel> MarkerCanvas;

	/** All markers on the map */
	UPROPERTY()
	TMap<FName, FMapMarkerData> Markers;

	/** Current input mode */
	EMapInputMode CurrentInputMode = EMapInputMode::None;

	/** ID of marker currently being dragged */
	FName DraggingMarkerId;

	/** Last mouse position for delta calculations */
	FVector2D LastMousePosition;

	/** Is mouse currently over the widget */
	bool bIsMouseOver = false;

	/** Cached geometry for calculations */
	FGeometry CachedGeometry;

private:
	void UpdateMarkerPositions();
	void SetMarkerState(FName MarkerId, EMapMarkerState NewState);
	FName FindMarkerAtPosition(FVector2D LocalPosition) const;
	void HandlePanning(FVector2D MouseDelta);
	void HandleMarkerDrag(FVector2D LocalPosition);
	void HandleZoom(float ZoomDelta, FVector2D LocalPosition);

	/** Counter for generating unique marker IDs */
	int32 MarkerIdCounter = 0;
};
