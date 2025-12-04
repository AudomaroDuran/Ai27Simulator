// Copyright © 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista
// Project: AI27 Simulator

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoadIntersection.generated.h"

class ARoadSplineActor;
class USplineComponent;
class UBillboardComponent;

// Forward declare ETransitionMode from TestVehicle
enum ETransitionMode : uint8;

/**
 * Type of connection at intersection
 */
UENUM(BlueprintType)
enum EConnectionType
{
	/** Traffic flows INTO intersection from this road */
	Incoming UMETA(DisplayName = "Incoming"),

	/** Traffic flows OUT OF intersection to this road */
	Outgoing UMETA(DisplayName = "Outgoing"),

	/** Traffic can flow both ways (most common) */
	Bidirectional UMETA(DisplayName = "Bidirectional")
};

/**
 * Type of intersection
 */
UENUM(BlueprintType)
enum EIntersectionType
{
	/** Two roads meeting (L shape) */
	TwoWay UMETA(DisplayName = "2-Way (L)"),

	/** Three roads meeting (T or Y shape) */
	ThreeWay UMETA(DisplayName = "3-Way (T/Y)"),

	/** Four roads meeting (+ or X shape) */
	FourWay UMETA(DisplayName = "4-Way (Cross)"),

	/** Roundabout with multiple exits */
	Roundabout UMETA(DisplayName = "Roundabout"),

	/** Custom configuration */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Connection point for a road at an intersection
 */
USTRUCT(BlueprintType)
struct FRoadConnectionPoint
{
	GENERATED_BODY()

	/** The road connected at this point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "The road connected at this intersection point"))
	ARoadSplineActor* Road;

	/** Is this road connected at its START or END? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "Is this road connected at its start point? (false = connected at end point)"))
	bool bConnectedAtStart;

	/** Type of connection (incoming traffic, outgoing, or bidirectional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "Incoming = traffic comes IN to intersection, Outgoing = traffic goes OUT, Bidirectional = both"))
	TEnumAsByte<enum EConnectionType> ConnectionType;

	/** Connection angle in degrees (0-360) for visualization */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (Tooltip = "Angle of this connection relative to intersection center (auto-calculated)"))
	float ConnectionAngle;

	/** World location of connection point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (Tooltip = "World location where this road connects (auto-calculated)"))
	FVector ConnectionPoint;

	FRoadConnectionPoint()
		: Road(nullptr)
		, bConnectedAtStart(false)
		, ConnectionType(EConnectionType::Bidirectional)
		, ConnectionAngle(0.0f)
		, ConnectionPoint(FVector::ZeroVector)
	{
	}
};

/**
 * Actor que representa una intersección de carreteras
 * Maneja conexiones múltiples y genera curvas de transición suaves
 *
 * Features:
 * - Maneja 2+ roads conectadas
 * - Genera curvas de transición automáticamente
 * - Lógica de decisión (qué road elegir)
 * - Visualización de conexiones en editor
 * - Soporte para semáforos (futuro)
 *
 * Uso:
 * 1. Colocar RoadIntersection en nivel
 * 2. Agregar roads al array Connections
 * 3. Especificar tipo de conexión (Incoming/Outgoing/Bidirectional)
 * 4. Las curvas de transición se generan automáticamente
 */
UCLASS()
class AI27SIMULATOR_API ARoadIntersection : public AActor
{
	GENERATED_BODY()

public:
	ARoadIntersection();

	// ========================================
	// Components
	// ========================================

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "Root component for the intersection"))
	USceneComponent* SceneRoot;

	/** Billboard for editor visibility */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "Billboard icon for editor visibility"))
	UBillboardComponent* IntersectionIcon;

	// ========================================
	// Intersection Properties
	// ========================================

	/** Name of this intersection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection|Info", meta = (Tooltip = "Display name for this intersection (for identification)"))
	FString IntersectionName;

	/** Roads connected to this intersection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection|Connections", meta = (Tooltip = "All roads connected to this intersection"))
	TArray<FRoadConnectionPoint> Connections;

	/** Radius of intersection area in cm (for curve generation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection|Properties", meta = (Tooltip = "Radius of intersection area in cm (affects turn tightness)"))
	float IntersectionRadius;

	/** Type of intersection (T, Cross, Roundabout, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection|Info", meta = (Tooltip = "Type of intersection (for visualization and behavior)"))
	TEnumAsByte<enum EIntersectionType> IntersectionType;

	// ========================================
	// Navigation Functions
	// ========================================

	/**
	 * Get valid outgoing roads from a given incoming road
	 * @param IncomingRoad The road the vehicle is coming from
	 * @return Array of roads the vehicle can transition to
	 */
	UFUNCTION(BlueprintCallable, Category = "Intersection", meta = (Tooltip = "Get all valid outgoing roads from an incoming road"))
	TArray<ARoadSplineActor*> GetOutgoingRoads(ARoadSplineActor* IncomingRoad) const;

	/**
	 * Choose next road based on transition mode
	 * @param IncomingRoad Road vehicle is coming from
	 * @param TransitionMode How to choose (Random, First, Last)
	 * @return Selected outgoing road, or nullptr if none available
	 */
	UFUNCTION(BlueprintCallable, Category = "Intersection", meta = (Tooltip = "Choose next road from intersection based on mode"))
	ARoadSplineActor* ChooseNextRoad(ARoadSplineActor* IncomingRoad, TEnumAsByte<enum ETransitionMode> TransitionMode) const;

	/**
	 * Generate transition spline between two roads
	 * Creates a smooth curve for vehicle to follow
	 * @param FromRoad Road coming from
	 * @param ToRoad Road going to
	 * @return Generated spline component (temporary, cleaned up after use)
	 */
	UFUNCTION(BlueprintCallable, Category = "Intersection", meta = (Tooltip = "Generate smooth transition curve between two roads"))
	USplineComponent* GenerateTransitionCurve(ARoadSplineActor* FromRoad, ARoadSplineActor* ToRoad);

	// ========================================
	// Utility Functions
	// ========================================

	/**
	 * Calculate connection points and angles
	 * Call this after modifying Connections array
	 */
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Intersection", meta = (Tooltip = "Recalculate connection points and angles (call after editing connections)"))
	void UpdateConnectionPoints();

	/**
	 * Get total number of connections
	 */
	UFUNCTION(BlueprintPure, Category = "Intersection", meta = (Tooltip = "Get total number of connected roads"))
	int32 GetConnectionCount() const { return Connections.Num(); }

	// ========================================
	// Debug Visualization
	// ========================================

	/** Show debug visualization of connections? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection|Debug", meta = (Tooltip = "Show debug lines for connections in viewport?"))
	bool bShowDebugConnections;

	/** Show connection angles? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection|Debug", meta = (Tooltip = "Show connection angle labels in viewport?"))
	bool bShowConnectionAngles;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	virtual void Tick(float DeltaTime) override;

private:
	/** Find connection info for a specific road */
	FRoadConnectionPoint* FindConnection(ARoadSplineActor* Road);
	const FRoadConnectionPoint* FindConnection(ARoadSplineActor* Road) const;

	/** Temporary transition splines (cleaned up after use) */
	UPROPERTY()
	TArray<USplineComponent*> TransitionSplines;
};
