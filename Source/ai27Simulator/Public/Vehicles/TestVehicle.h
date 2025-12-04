// Copyright © 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista
// Project: AI27 Simulator

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SplineComponent.h"
#include "TestVehicle.generated.h"

class USplineMovementComponent;
class UStaticMeshComponent;
class ARoadSplineActor;
class ARoadIntersection;

/**
 * How to choose next road when multiple are connected
 */
UENUM(BlueprintType)
enum ETransitionMode : uint8
{
	/** Pick random connected road */
	Random UMETA(DisplayName = "Random"),

	/** Always pick first connected road */
	First UMETA(DisplayName = "First"),

	/** Always pick last connected road */
	Last UMETA(DisplayName = "Last")
};

/**
 * Simple test vehicle for MVP demonstration
 *
 * Features:
 * - Follows splines using SplineMovementComponent
 * - Can be assigned to roads
 * - Visual representation (cube or simple mesh)
 * - No physics - pure spline following
 *
 * Usage:
 * 1. Place TestVehicle in level
 * 2. Assign StartingRoad (RoadSplineActor)
 * 3. Play - vehicle will automatically follow the road
 *
 * Or via Blueprint:
 * 1. Get TestVehicle reference
 * 2. Call AssignToRoad(RoadSplineActor)
 * 3. Vehicle starts moving
 */
UCLASS()
class AI27SIMULATOR_API ATestVehicle : public APawn
{
	GENERATED_BODY()

public:
	ATestVehicle();

	// ========================================
	// Components
	// ========================================

	/** Visual mesh (cube by default) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "Visual mesh representing the vehicle"))
	UStaticMeshComponent* VehicleMesh;

	/** Movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "Spline movement component controlling vehicle movement"))
	USplineMovementComponent* MovementComponent;

	// ========================================
	// Configuration
	// ========================================

	/** Road to start on (can be set in editor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Setup", meta = (Tooltip = "Road to automatically start following on BeginPlay (if AutoStart is enabled)"))
	ARoadSplineActor* StartingRoad;

	/** Should start moving automatically on BeginPlay? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Setup", meta = (Tooltip = "If true, vehicle starts following StartingRoad automatically"))
	bool bAutoStart;

	/** Initial speed in km/h */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Setup", meta = (Tooltip = "Initial speed in km/h (60, 80, 120, etc.)"))
	float InitialSpeedKmH;

	/** Vehicle display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Info", meta = (Tooltip = "Display name for this vehicle (for logs and UI)"))
	FString VehicleName;

	// ========================================
	// Auto-Transition Configuration
	// ========================================

	/** Should automatically transition to next road when reaching end? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Transition", meta = (Tooltip = "If true, automatically switches to next connected road when reaching end"))
	bool bAutoTransition;

	/** How to choose next road when multiple are connected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Transition", meta = (Tooltip = "Random = pick random road, First = always pick first connected road, Last = always pick last"))
	TEnumAsByte<enum ETransitionMode> TransitionMode;

	/** Use RoadIntersection actors for smart transitions (if available) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Transition", meta = (Tooltip = "If true, uses RoadIntersection actors for smooth curved transitions"))
	bool bUseIntersections;

	/** Search radius for finding intersections (in cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Transition", meta = (Tooltip = "How far to search for RoadIntersection actors (in cm, default 1000 = 10m)"))
	float IntersectionSearchRadius;

	// ========================================
	// Control Functions
	// ========================================

	/**
	 * Assign this vehicle to a road and start following it
	 * @param Road Road to follow
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle", meta = (Tooltip = "Assign vehicle to a road and start following it"))
	void AssignToRoad(ARoadSplineActor* Road);

	/**
	 * Stop the vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle", meta = (Tooltip = "Stop the vehicle (decelerates to zero)"))
	void StopVehicle();

	/**
	 * Resume movement
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle", meta = (Tooltip = "Resume vehicle movement (accelerates to max speed)"))
	void ResumeVehicle();

	/**
	 * Set vehicle speed in km/h
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle", meta = (Tooltip = "Set vehicle speed in km/h"))
	void SetVehicleSpeed(float SpeedKmH);

	// ========================================
	// Query Functions
	// ========================================

	/**
	 * Is vehicle currently moving?
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle", meta = (Tooltip = "Is vehicle currently moving?"))
	bool IsMoving() const;

	/**
	 * Get current speed in km/h
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle", meta = (Tooltip = "Get current speed in km/h"))
	float GetCurrentSpeed() const;

	/**
	 * Get progress along current road (0-100%)
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle", meta = (Tooltip = "Get progress along current road as percentage (0-100%)"))
	float GetProgress() const;

protected:
	virtual void BeginPlay() override;

	// Event handlers
	UFUNCTION()
	void OnReachedEndOfRoad();

	UFUNCTION()
	void OnSpeedChanged(float NewSpeedKmH);

	// Transition logic
	/**
	 * Choose next road from connected roads based on TransitionMode
	 * @param ConnectedRoads Array of available connected roads
	 * @return Selected road, or nullptr if no roads available
	 */
	ARoadSplineActor* ChooseNextRoad(const TArray<ARoadSplineActor*>& ConnectedRoads);

	/**
	 * Find RoadIntersection actor near current road end
	 * @return RoadIntersection if found within search radius, nullptr otherwise
	 */
	ARoadIntersection* FindNearbyIntersection() const;

	/**
	 * Handle transition using RoadIntersection
	 * @param Intersection The intersection to use
	 * @param FromRoad Road we're coming from
	 * @return True if transition was successful
	 */
	bool TransitionThroughIntersection(ARoadIntersection* Intersection, ARoadSplineActor* FromRoad);

	/**
	 * Event called when vehicle finishes following a transition curve
	 */
	UFUNCTION()
	void OnTransitionCurveComplete();

private:
	/** Temporary transition spline (when using intersections) */
	UPROPERTY()
	USplineComponent* CurrentTransitionCurve;

	/** Target road after transition curve */
	UPROPERTY()
	ARoadSplineActor* PendingTargetRoad;

	/** Are we currently following a transition curve? */
	bool bFollowingTransitionCurve;

public:
	virtual void Tick(float DeltaTime) override;
};
