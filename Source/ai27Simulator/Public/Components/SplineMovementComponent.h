// Copyright © 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista
// Project: AI27 Simulator

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SplineMovementComponent.generated.h"

class ARoadSplineActor;
class USplineComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSplineEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeedChanged, float, NewSpeedKmH);

/**
 * Component simple para mover actors a lo largo de splines
 * NO usa física, solo interpolación de posición
 * Optimizado para móvil - muy eficiente
 *
 * Uso:
 * 1. Add component a tu Actor
 * 2. Call StartFollowingSpline(RoadSplineActor) o StartFollowingSplineComponent(SplineComponent)
 * 3. El actor se moverá automáticamente
 */
UCLASS(ClassGroup=(AI27), meta=(BlueprintSpawnableComponent))
class AI27SIMULATOR_API USplineMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USplineMovementComponent();

	// ========================================
	// Current Spline Reference
	// ========================================

	/** Road actor being followed (if any) */
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (Tooltip = "The road actor currently being followed"))
	ARoadSplineActor* CurrentRoad;

	/** Spline component being followed */
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (Tooltip = "The spline component currently being followed"))
	USplineComponent* CurrentSpline;

	// ========================================
	// Movement Properties
	// ========================================

	/** Maximum speed in cm/s (default: 8000 = ~80 km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed", meta = (Tooltip = "Maximum speed in cm/s (8000 = ~80 km/h, 16667 = ~167 km/h)"))
	float MaxSpeed;

	/** Acceleration in cm/s² */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed", meta = (Tooltip = "Acceleration rate in cm/s² (higher = faster acceleration)"))
	float Acceleration;

	/** Deceleration (braking) in cm/s² */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed", meta = (Tooltip = "Deceleration (braking) rate in cm/s² (higher = faster braking)"))
	float Deceleration;

	/** Current speed in cm/s */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State", meta = (Tooltip = "Current speed in cm/s (use GetSpeedKmH for km/h)"))
	float CurrentSpeed;

	/** Current distance along spline in cm */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State", meta = (Tooltip = "Current distance traveled along spline in cm"))
	float DistanceAlongSpline;

	// ========================================
	// Control
	// ========================================

	/** If true, movement happens automatically in Tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Control", meta = (Tooltip = "If true, movement happens automatically every frame"))
	bool bAutoMove;

	/** Is currently moving? */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State", meta = (Tooltip = "Is the actor currently moving?"))
	bool bIsMoving;

	/** Should loop back to start when reaching end? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Control", meta = (Tooltip = "If true, loops back to start when reaching the end of spline"))
	bool bLoopAtEnd;

	// ========================================
	// Core Functions
	// ========================================

	/**
	 * Start following a RoadSplineActor
	 * @param Road The road actor to follow
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (Tooltip = "Start following a RoadSplineActor. Resets distance to 0."))
	void StartFollowingSpline(ARoadSplineActor* Road);

	/**
	 * Start following a SplineComponent directly
	 * @param Spline The spline component to follow
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (Tooltip = "Start following a SplineComponent directly. Resets distance to 0."))
	void StartFollowingSplineComponent(USplineComponent* Spline);

	/**
	 * Stop movement (decelerate to zero)
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (Tooltip = "Stop movement. Vehicle will decelerate to zero speed."))
	void StopMovement();

	/**
	 * Resume movement (accelerate to max speed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (Tooltip = "Resume movement. Vehicle will accelerate to max speed."))
	void ResumeMovement();

	/**
	 * Set new max speed
	 * @param NewSpeed New max speed in cm/s
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (Tooltip = "Set max speed in cm/s (use SetSpeedKmH for km/h)"))
	void SetSpeed(float NewSpeed);

	/**
	 * Set speed in Km/h (converts to cm/s internally)
	 * @param SpeedKmH Speed in kilometers per hour
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (Tooltip = "Set max speed in km/h (60, 80, 120, etc.)"))
	void SetSpeedKmH(float SpeedKmH);

	/**
	 * Switch to a new spline (useful for intersections)
	 * @param NewRoad New road to follow
	 * @param bMaintainSpeed If true, keeps current speed. If false, starts from zero
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (Tooltip = "Switch to a new road. Useful for intersections and route changes."))
	void SwitchToNewSpline(ARoadSplineActor* NewRoad, bool bMaintainSpeed = true);

	/**
	 * Switch to a new spline component
	 * @param NewSpline New spline to follow
	 * @param bMaintainSpeed If true, keeps current speed
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (Tooltip = "Switch to a new spline component directly."))
	void SwitchToNewSplineComponent(USplineComponent* NewSpline, bool bMaintainSpeed = true);

	// ========================================
	// Query Functions
	// ========================================

	/**
	 * Get current speed in Km/h
	 */
	UFUNCTION(BlueprintPure, Category = "Movement", meta = (Tooltip = "Get current speed in km/h"))
	float GetSpeedKmH() const;

	/**
	 * Get progress along spline as percentage (0-100)
	 */
	UFUNCTION(BlueprintPure, Category = "Movement", meta = (Tooltip = "Get progress along spline as percentage (0-100%)"))
	float GetProgressPercent() const;

	/**
	 * Get remaining distance to end in cm
	 */
	UFUNCTION(BlueprintPure, Category = "Movement", meta = (Tooltip = "Get remaining distance to end of spline in cm"))
	float GetRemainingDistance() const;

	/**
	 * Is currently following a spline?
	 */
	UFUNCTION(BlueprintPure, Category = "Movement", meta = (Tooltip = "Is currently following a spline?"))
	bool IsFollowingSpline() const;

	// ========================================
	// Events
	// ========================================

	/** Called when reaching the end of spline */
	UPROPERTY(BlueprintAssignable, Category = "Movement|Events", meta = (Tooltip = "Event fired when vehicle reaches the end of the spline"))
	FOnSplineEnd OnReachedEnd;

	/** Called when speed changes significantly */
	UPROPERTY(BlueprintAssignable, Category = "Movement|Events", meta = (Tooltip = "Event fired when speed changes by more than 5 km/h"))
	FOnSpeedChanged OnSpeedChanged;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                          FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Internal update functions
	void UpdateMovement(float DeltaTime);
	void UpdateTransform();

	// Last speed for change detection
	float LastNotifiedSpeed;

	// ========================================
	// Smooth Transition System
	// ========================================

	/** Are we currently transitioning rotation between roads? */
	bool bIsTransitioning;

	/** Time remaining for rotation transition in seconds */
	float TransitionTimeRemaining;

	/** Duration of rotation transition in seconds */
	float TransitionDuration;

	/** Rotation at start of transition */
	FRotator TransitionStartRotation;

	/** Target rotation at end of transition */
	FRotator TransitionTargetRotation;

	/** Are we currently interpolating position to close a gap? */
	bool bIsInterpolatingPosition;

	/** Time remaining for position interpolation in seconds */
	float PositionInterpolationTimeRemaining;

	/** Duration of position interpolation in seconds */
	float PositionInterpolationDuration;

	/** Position at start of interpolation */
	FVector PositionInterpolationStart;

	/** Target position at end of interpolation */
	FVector PositionInterpolationTarget;

	/** Rotation at start of position interpolation */
	FRotator PositionInterpolationStartRotation;

	/** Target rotation at end of position interpolation */
	FRotator PositionInterpolationTargetRotation;

	/**
	 * Detect connection point between two roads
	 * Returns true if roads are connected and sets OutStartDistance
	 * @param FromRoad The road we're coming from
	 * @param ToRoad The road we're going to
	 * @param OutStartDistance Distance along ToRoad where we should start (0.0 or spline length)
	 * @param OutShouldReverse Should we travel ToRoad in reverse?
	 * @return true if roads are connected within tolerance
	 */
	bool DetectRoadConnection(ARoadSplineActor* FromRoad, ARoadSplineActor* ToRoad,
	                          float& OutStartDistance, bool& OutShouldReverse) const;

	/** Update smooth rotation during transitions */
	void UpdateTransitionRotation(float DeltaTime);

	/** Update smooth position interpolation during transitions */
	void UpdatePositionInterpolation(float DeltaTime);
};
