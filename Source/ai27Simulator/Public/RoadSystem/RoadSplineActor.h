// Copyright © 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista
// Project: AI27 Simulator

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoadSplineActor.generated.h"

class USplineComponent;
class USplineMeshComponent;

/**
 * Actor que representa una carretera basada en spline
 * Puede ser conectado con otros RoadSplineActors para formar una red de carreteras
 *
 * Features:
 * - Spline editable visualmente en el editor
 * - Mesh de carretera generado automáticamente (opcional)
 * - Propiedades de carretera (ancho, velocidad límite, etc.)
 * - Conexiones con otras carreteras
 * - Zonas de riesgo marcables
 *
 * Uso:
 * 1. Colocar RoadSplineActor en el nivel
 * 2. Seleccionar y editar spline points
 * 3. Configurar propiedades (ancho, lanes, speed limit)
 * 4. Conectar con otras roads si es necesario
 * 5. Vehículos pueden seguir el spline automáticamente
 */
UCLASS()
class AI27SIMULATOR_API ARoadSplineActor : public AActor
{
	GENERATED_BODY()

public:
	ARoadSplineActor();

	// ========================================
	// Components
	// ========================================

	/** Main spline component defining the road path */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road|Components", meta = (Tooltip = "Spline component defining the road path. Edit spline points in viewport."))
	USplineComponent* RoadSpline;

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road|Components", meta = (Tooltip = "Root component for the actor"))
	USceneComponent* SceneRoot;

	// ========================================
	// Road Properties
	// ========================================

	/** Width of the road in cm (default: 800 cm = 8 meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Properties", meta = (Tooltip = "Width of the road in cm (800 = 8m, 1200 = 12m)"))
	float RoadWidth;

	/** Number of lanes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Properties", meta = (Tooltip = "Number of lanes (1, 2, 3, etc.)"))
	int32 NumLanes;

	/** Speed limit in km/h */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Properties", meta = (Tooltip = "Speed limit in km/h (60, 80, 120, etc.)"))
	float SpeedLimit;

	/** Is this a highway? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Properties", meta = (Tooltip = "Is this a highway? (affects traffic behavior)"))
	bool bIsHighway;

	/** Is this a risk zone? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Properties", meta = (Tooltip = "Is this a risk zone? (triggers alerts for tracked vehicles)"))
	bool bIsRiskZone;

	/** Optional name for this road */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Properties", meta = (Tooltip = "Display name for this road (for identification and logs)"))
	FString RoadName;

	// ========================================
	// Visual Properties
	// ========================================

	/** Should generate mesh along spline? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Visual", meta = (Tooltip = "Generate visual road mesh along spline? (optional)"))
	bool bGenerateRoadMesh;

	/** Static mesh to use for road segments (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Visual", meta = (Tooltip = "Static mesh to use for road segments (leave empty for simple spline)"))
	UStaticMesh* RoadMeshSegment;

	/** Material for the road */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Visual", meta = (Tooltip = "Material to apply to road mesh"))
	UMaterialInterface* RoadMaterial;

	/** Color tint for risk zones (red) or normal roads */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Visual", meta = (Tooltip = "Color tint for road (red for risk zones, white for normal)"))
	FLinearColor RoadColor;

	// ========================================
	// Navigation Functions
	// ========================================

	/**
	 * Get location at specific distance along spline
	 * @param Distance Distance in cm from start of spline
	 * @return World location at that distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Navigation", meta = (Tooltip = "Get world location at specific distance along road"))
	FVector GetLocationAtDistance(float Distance) const;

	/**
	 * Get location at normalized time (0-1) along spline
	 * @param Time 0 = start, 1 = end
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Navigation", meta = (Tooltip = "Get location at normalized time (0=start, 1=end)"))
	FVector GetLocationAtTime(float Time) const;

	/**
	 * Get rotation at specific distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Navigation", meta = (Tooltip = "Get rotation at specific distance along road"))
	FRotator GetRotationAtDistance(float Distance) const;

	/**
	 * Get total length of spline
	 */
	UFUNCTION(BlueprintPure, Category = "Road|Navigation", meta = (Tooltip = "Get total length of road in cm"))
	float GetSplineLength() const;

	/**
	 * Find closest location on spline to a world location
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Navigation", meta = (Tooltip = "Find closest point on road to a world location"))
	FVector GetClosestLocationOnSpline(const FVector& WorldLocation, float& OutDistance) const;

	/**
	 * Check if a location is within road bounds
	 */
	UFUNCTION(BlueprintPure, Category = "Road|Navigation", meta = (Tooltip = "Check if a location is within road bounds"))
	bool IsLocationOnRoad(const FVector& WorldLocation, float Tolerance = 500.0f) const;

	// ========================================
	// Connections
	// ========================================

	/** Other roads connected to this one */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Connections", meta = (Tooltip = "Other roads connected to this one (for intersections and route planning)"))
	TArray<ARoadSplineActor*> ConnectedRoads;

	/**
	 * Connect this road to another road
	 * @param OtherRoad Road to connect to
	 * @param bAtStart If true, connects at start of this road. If false, at end
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Connections", meta = (Tooltip = "Connect this road to another road (for intersections)"))
	void ConnectToRoad(ARoadSplineActor* OtherRoad, bool bAtStart = false);

	/**
	 * Get roads connected at the start of this road
	 */
	UFUNCTION(BlueprintPure, Category = "Road|Connections", meta = (Tooltip = "Get all roads connected at the start of this road"))
	TArray<ARoadSplineActor*> GetRoadsAtStart() const;

	/**
	 * Get roads connected at the end of this road
	 */
	UFUNCTION(BlueprintPure, Category = "Road|Connections", meta = (Tooltip = "Get all roads connected at the end of this road"))
	TArray<ARoadSplineActor*> GetRoadsAtEnd() const;

	// ========================================
	// Debug
	// ========================================

	/** Show debug visualization? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Debug", meta = (Tooltip = "Show debug spline visualization in viewport?"))
	bool bShowDebugSpline;

	/** Show road bounds? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Debug", meta = (Tooltip = "Show road width bounds in viewport?"))
	bool bShowRoadBounds;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	virtual void Tick(float DeltaTime) override;

private:
	// Internal mesh generation
	void GenerateRoadMesh();
	void ClearRoadMesh();

	// Spline mesh components (generated)
	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshComponents;

	// Connection tracking
	struct FRoadConnection
	{
		ARoadSplineActor* ConnectedRoad;
		bool bConnectedAtStart; // Of this road
	};
	TArray<FRoadConnection> Connections;
};
