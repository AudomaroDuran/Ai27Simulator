// Copyright Ai27. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "MapCaptureComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMapBoundsChanged, FVector2D, NewCenter, float, NewZoom);

/**
 * Component that handles the scene capture for the map system.
 * Provides top-down view capture with zoom and pan capabilities.
 */
UCLASS(ClassGroup=(MapSystem), meta=(BlueprintSpawnableComponent))
class MAPSYSTEM_API UMapCaptureComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UMapCaptureComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== Configuration ====================

	/** Resolution of the render target (width and height) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	int32 MapResolution = 1024;

	/** Initial height of the capture camera above the scene */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	float InitialCaptureHeight = 5000.0f;

	/** Minimum zoom level (1.0 = default, lower = more zoomed out) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	float MinZoom = 0.1f;

	/** Maximum zoom level (higher = more zoomed in) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	float MaxZoom = 10.0f;

	/** Current zoom level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	float CurrentZoom = 1.0f;

	/** Zoom speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	float ZoomSpeed = 0.1f;

	/** Pan speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	float PanSpeed = 1.0f;

	/** The orthographic width at zoom level 1.0 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Configuration")
	float BaseOrthoWidth = 10000.0f;

	/** Channel to use for the trace validation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Validation")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** Maximum trace distance for point validation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Validation")
	float MaxTraceDistance = 50000.0f;

	// ==================== Runtime Properties ====================

	/** The render target used for the map */
	UPROPERTY(BlueprintReadOnly, Category = "Map|Runtime")
	TObjectPtr<UTextureRenderTarget2D> MapRenderTarget;

	/** The scene capture component used internally */
	UPROPERTY(BlueprintReadOnly, Category = "Map|Runtime")
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;

	/** Current center position of the map in world coordinates */
	UPROPERTY(BlueprintReadOnly, Category = "Map|Runtime")
	FVector2D MapCenterWorld;

	// ==================== Events ====================

	/** Called when map bounds change (pan or zoom) */
	UPROPERTY(BlueprintAssignable, Category = "Map|Events")
	FOnMapBoundsChanged OnMapBoundsChanged;

	// ==================== Functions ====================

	/** Initialize the map capture system */
	UFUNCTION(BlueprintCallable, Category = "Map")
	void InitializeMapCapture();

	/** Set the center position of the map in world coordinates */
	UFUNCTION(BlueprintCallable, Category = "Map")
	void SetMapCenter(FVector2D NewCenter);

	/** Pan the map by a delta amount in screen space */
	UFUNCTION(BlueprintCallable, Category = "Map")
	void PanMap(FVector2D ScreenDelta);

	/** Zoom the map by a delta amount, optionally around a specific UV point */
	UFUNCTION(BlueprintCallable, Category = "Map")
	void ZoomMap(float ZoomDelta, FVector2D ZoomAroundUV = FVector2D(0.5f, 0.5f));

	/** Set absolute zoom level */
	UFUNCTION(BlueprintCallable, Category = "Map")
	void SetZoom(float NewZoom);

	/** Convert world position to UV coordinates (0-1) on the map */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map|Conversion")
	FVector2D WorldToMapUV(FVector WorldPosition) const;

	/** Convert UV coordinates (0-1) on the map to world position */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map|Conversion")
	FVector MapUVToWorld(FVector2D UV) const;

	/** Check if a world position is currently visible on the map */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map|Conversion")
	bool IsWorldPositionVisible(FVector WorldPosition) const;

	/** Get the current visible world bounds of the map */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map|Conversion")
	void GetVisibleWorldBounds(FVector2D& OutMin, FVector2D& OutMax) const;

	/** Validate if a world position is a valid placement point using traces */
	UFUNCTION(BlueprintCallable, Category = "Map|Validation")
	bool ValidateWorldPosition(FVector WorldPosition, FVector& OutValidPosition) const;

	/** Find valid snap position from UV coordinates on the map */
	UFUNCTION(BlueprintCallable, Category = "Map|Validation")
	bool FindValidSnapPosition(FVector2D MapUV, FVector& OutWorldPosition) const;

	/** Get current ortho width based on zoom */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map")
	float GetCurrentOrthoWidth() const;

	/** Get the render target texture */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Map")
	UTextureRenderTarget2D* GetMapTexture() const { return MapRenderTarget; }

	/** Force update the capture */
	UFUNCTION(BlueprintCallable, Category = "Map")
	void UpdateCapture();

private:
	void UpdateCaptureTransform();
	void CreateRenderTarget();
	void SetupSceneCapture();

	/** Cached owner for quick access */
	UPROPERTY()
	TObjectPtr<AActor> CachedOwner;

	bool bIsInitialized = false;
};
