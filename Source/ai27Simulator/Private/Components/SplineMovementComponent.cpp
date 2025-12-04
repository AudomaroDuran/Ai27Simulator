// Copyright © 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista

#include "Components/SplineMovementComponent.h"
#include "Components/SplineComponent.h"
#include "RoadSystem/RoadSplineActor.h"
#include "DrawDebugHelpers.h"

USplineMovementComponent::USplineMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	MaxSpeed = 8000.0f;           // ~80 km/h
	Acceleration = 500.0f;        // Acceleration rate
	Deceleration = 1000.0f;       // Braking rate
	CurrentSpeed = 0.0f;
	DistanceAlongSpline = 0.0f;
	bAutoMove = true;
	bIsMoving = false;
	bLoopAtEnd = false;
	LastNotifiedSpeed = 0.0f;

	CurrentRoad = nullptr;
	CurrentSpline = nullptr;

	// Transition system
	bIsTransitioning = false;
	TransitionTimeRemaining = 0.0f;
	TransitionDuration = 0.5f;    // 0.5 seconds for smooth turn
	TransitionStartRotation = FRotator::ZeroRotator;
	TransitionTargetRotation = FRotator::ZeroRotator;

	// Position interpolation system
	bIsInterpolatingPosition = false;
	PositionInterpolationTimeRemaining = 0.0f;
	PositionInterpolationDuration = 0.3f;  // 0.3 seconds for position gap closing
	PositionInterpolationStart = FVector::ZeroVector;
	PositionInterpolationTarget = FVector::ZeroVector;
	PositionInterpolationStartRotation = FRotator::ZeroRotator;
	PositionInterpolationTargetRotation = FRotator::ZeroRotator;
}

void USplineMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USplineMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAutoMove && CurrentSpline)
	{
		UpdateMovement(DeltaTime);
	}
}

void USplineMovementComponent::StartFollowingSpline(ARoadSplineActor* Road)
{
	if (!Road)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineMovementComponent: Road is null"));
		return;
	}

	CurrentRoad = Road;
	CurrentSpline = Road->RoadSpline;

	if (CurrentSpline)
	{
		DistanceAlongSpline = 0.0f;
		bIsMoving = true;
		UpdateTransform();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineMovementComponent: Road has no spline component"));
	}
}

void USplineMovementComponent::StartFollowingSplineComponent(USplineComponent* Spline)
{
	if (!Spline)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineMovementComponent: Spline is null"));
		return;
	}

	CurrentSpline = Spline;
	CurrentRoad = nullptr;
	DistanceAlongSpline = 0.0f;
	bIsMoving = true;

	UpdateTransform();
}

void USplineMovementComponent::UpdateMovement(float DeltaTime)
{
	if (!CurrentSpline)
		return;

	// Accelerate or decelerate
	if (bIsMoving)
	{
		// Accelerate towards max speed
		CurrentSpeed = FMath::FInterpConstantTo(CurrentSpeed, MaxSpeed, DeltaTime, Acceleration);
	}
	else
	{
		// Decelerate to zero
		CurrentSpeed = FMath::FInterpConstantTo(CurrentSpeed, 0.0f, DeltaTime, Deceleration);
	}

	// Move along spline
	DistanceAlongSpline += CurrentSpeed * DeltaTime;

	float SplineLength = CurrentSpline->GetSplineLength();

	// Check if reached end
	if (DistanceAlongSpline >= SplineLength)
	{
		if (bLoopAtEnd)
		{
			// Loop back to start
			DistanceAlongSpline = 0.0f;
		}
		else
		{
			// Stop at end
			DistanceAlongSpline = SplineLength;
			bIsMoving = false;
			CurrentSpeed = 0.0f;

			OnReachedEnd.Broadcast();
			return;
		}
	}

	// Update transform (only if not interpolating position)
	if (!bIsInterpolatingPosition)
	{
		UpdateTransform();
	}

	// Update smooth rotation transition (if active)
	UpdateTransitionRotation(DeltaTime);

	// Update smooth position interpolation (if active)
	UpdatePositionInterpolation(DeltaTime);

	// Notify speed changes
	float SpeedKmH = GetSpeedKmH();
	if (FMath::Abs(SpeedKmH - LastNotifiedSpeed) > 5.0f) // Notify if change > 5 km/h
	{
		LastNotifiedSpeed = SpeedKmH;
		OnSpeedChanged.Broadcast(SpeedKmH);
	}

	// Debug visualization in viewport (only in PIE)
	AActor* Owner = GetOwner();
	if (Owner && Owner->GetWorld() && Owner->GetWorld()->IsPlayInEditor())
	{
		FVector DebugLocation = Owner->GetActorLocation() + FVector(0, 0, 150);
		FString DebugText = FString::Printf(TEXT("Speed: %.0f km/h\nProgress: %.0f%%\nMoving: %s"),
			SpeedKmH,
			GetProgressPercent(),
			bIsMoving ? TEXT("Yes") : TEXT("No"));

		DrawDebugString(Owner->GetWorld(), DebugLocation, DebugText, nullptr, FColor::Green, 0.0f, true);
	}
}

void USplineMovementComponent::UpdateTransform()
{
	if (!CurrentSpline)
		return;

	AActor* Owner = GetOwner();
	if (!Owner)
		return;

	// Get location and rotation at current distance
	FVector Location = CurrentSpline->GetLocationAtDistanceAlongSpline(
		DistanceAlongSpline,
		ESplineCoordinateSpace::World
	);

	FRotator Rotation = CurrentSpline->GetRotationAtDistanceAlongSpline(
		DistanceAlongSpline,
		ESplineCoordinateSpace::World
	);

	// Apply to owner
	Owner->SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::None);
}

void USplineMovementComponent::StopMovement()
{
	bIsMoving = false;
}

void USplineMovementComponent::ResumeMovement()
{
	if (CurrentSpline)
	{
		bIsMoving = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineMovementComponent: Cannot resume - no spline"));
	}
}

void USplineMovementComponent::SetSpeed(float NewSpeed)
{
	MaxSpeed = FMath::Clamp(NewSpeed, 0.0f, 20000.0f); // Max ~200 km/h
}

void USplineMovementComponent::SetSpeedKmH(float SpeedKmH)
{
	// Convert km/h to cm/s
	// 1 km/h = 27.778 cm/s
	float SpeedCmS = SpeedKmH * 27.778f;
	SetSpeed(SpeedCmS);
}

float USplineMovementComponent::GetSpeedKmH() const
{
	// Convert cm/s to km/h
	// 1 cm/s = 0.036 km/h
	return CurrentSpeed * 0.036f;
}

float USplineMovementComponent::GetProgressPercent() const
{
	if (!CurrentSpline)
		return 0.0f;

	float SplineLength = CurrentSpline->GetSplineLength();
	if (SplineLength == 0.0f)
		return 0.0f;

	return (DistanceAlongSpline / SplineLength) * 100.0f;
}

float USplineMovementComponent::GetRemainingDistance() const
{
	if (!CurrentSpline)
		return 0.0f;

	float SplineLength = CurrentSpline->GetSplineLength();
	return FMath::Max(0.0f, SplineLength - DistanceAlongSpline);
}

bool USplineMovementComponent::IsFollowingSpline() const
{
	return CurrentSpline != nullptr;
}

void USplineMovementComponent::SwitchToNewSpline(ARoadSplineActor* NewRoad, bool bMaintainSpeed)
{
	if (!NewRoad)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineMovementComponent: NewRoad is null"));
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Store previous road for connection detection
	ARoadSplineActor* PreviousRoad = CurrentRoad;

	// Update references
	CurrentRoad = NewRoad;
	CurrentSpline = NewRoad->RoadSpline;

	if (!CurrentSpline)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineMovementComponent: NewRoad has no RoadSpline"));
		return;
	}

	// Detect smart connection if we have a previous road
	float StartDistance = 0.0f;
	bool bShouldReverse = false;

	if (PreviousRoad && DetectRoadConnection(PreviousRoad, NewRoad, StartDistance, bShouldReverse))
	{
		// Roads are connected - use detected start distance
		DistanceAlongSpline = StartDistance;

		UE_LOG(LogTemp, Log, TEXT("✅ Smart Connection Detected: '%s' → '%s' | StartDistance: %.0f cm | Reverse: %s"),
			*PreviousRoad->RoadName, *NewRoad->RoadName, StartDistance, bShouldReverse ? TEXT("YES") : TEXT("NO"));

		// TODO: Handle reverse direction in future (requires direction tracking)
		if (bShouldReverse)
		{
			UE_LOG(LogTemp, Warning, TEXT("⚠️ REVERSE CONNECTION - Vehicle will teleport! Use RoadIntersection with bUseIntersections=true for proper curved transitions."));
			// For now, we'll start at the end and move backwards is not implemented
			// This will be handled in RoadIntersection Actor with proper curves
		}
	}
	else
	{
		// No previous road or not connected - default to start
		DistanceAlongSpline = 0.0f;

		if (PreviousRoad)
		{
			UE_LOG(LogTemp, Warning, TEXT("❌ No Connection Detected: '%s' → '%s' | Vehicle will start at beginning of new road"),
				*PreviousRoad->RoadName, *NewRoad->RoadName);
		}
	}

	// Maintain or reset speed
	if (!bMaintainSpeed)
	{
		CurrentSpeed = 0.0f;
	}

	// Calculate position gap between current position and new spline start position
	FVector CurrentPosition = Owner->GetActorLocation();
	FVector TargetPosition = CurrentSpline->GetLocationAtDistanceAlongSpline(
		DistanceAlongSpline,
		ESplineCoordinateSpace::World
	);
	float PositionGap = FVector::Dist(CurrentPosition, TargetPosition);

	// If gap is small (<500cm), interpolate position smoothly
	const float MaxInterpolationGap = 500.0f;  // 5 meters
	if (PositionGap > 1.0f && PositionGap < MaxInterpolationGap)
	{
		// Activate position interpolation (position + rotation synchronized)
		bIsInterpolatingPosition = true;
		PositionInterpolationTimeRemaining = PositionInterpolationDuration;
		PositionInterpolationStart = CurrentPosition;
		PositionInterpolationTarget = TargetPosition;

		// Store rotation for synchronized interpolation
		PositionInterpolationStartRotation = Owner->GetActorRotation();
		PositionInterpolationTargetRotation = CurrentSpline->GetRotationAtDistanceAlongSpline(
			DistanceAlongSpline,
			ESplineCoordinateSpace::World
		);

		// Position interpolation handles BOTH position and rotation - disable separate rotation transition
		bIsTransitioning = false;

		UE_LOG(LogTemp, Log, TEXT("🎯 Position + Rotation Interpolation Active: Gap %.0f cm → %.0f seconds transition"),
			PositionGap, PositionInterpolationDuration);
	}
	else
	{
		// Gap too large or too small - use standard rotation transition only
		bIsInterpolatingPosition = false;

		// Start smooth rotation transition (only when NOT interpolating position)
		bIsTransitioning = true;
		TransitionTimeRemaining = TransitionDuration;
		TransitionStartRotation = Owner->GetActorRotation();

		// Get target rotation from new spline
		TransitionTargetRotation = CurrentSpline->GetRotationAtDistanceAlongSpline(
			DistanceAlongSpline,
			ESplineCoordinateSpace::World
		);

		if (PositionGap >= MaxInterpolationGap)
		{
			UE_LOG(LogTemp, Warning, TEXT("⚠️ Large Gap Detected: %.0f cm (use RoadIntersection for gaps > 5m)"), PositionGap);
		}
	}

	bIsMoving = true;

	// Only update transform immediately if NOT interpolating position
	if (!bIsInterpolatingPosition)
	{
		UpdateTransform();
	}
}

void USplineMovementComponent::SwitchToNewSplineComponent(USplineComponent* NewSpline, bool bMaintainSpeed)
{
	if (!NewSpline)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineMovementComponent: NewSpline is null"));
		return;
	}

	CurrentSpline = NewSpline;
	CurrentRoad = nullptr;
	DistanceAlongSpline = 0.0f;

	if (!bMaintainSpeed)
	{
		CurrentSpeed = 0.0f;
	}

	bIsMoving = true;
	UpdateTransform();
}

bool USplineMovementComponent::DetectRoadConnection(ARoadSplineActor* FromRoad, ARoadSplineActor* ToRoad,
                                                     float& OutStartDistance, bool& OutShouldReverse) const
{
	if (!FromRoad || !ToRoad || !FromRoad->RoadSpline || !ToRoad->RoadSpline)
	{
		return false;
	}

	// Get end point of FromRoad (where we're coming from)
	FVector FromEndPoint = FromRoad->RoadSpline->GetLocationAtDistanceAlongSpline(
		FromRoad->RoadSpline->GetSplineLength(),
		ESplineCoordinateSpace::World
	);

	// Get start and end points of ToRoad
	FVector ToStartPoint = ToRoad->RoadSpline->GetLocationAtDistanceAlongSpline(
		0.0f,
		ESplineCoordinateSpace::World
	);

	FVector ToEndPoint = ToRoad->RoadSpline->GetLocationAtDistanceAlongSpline(
		ToRoad->RoadSpline->GetSplineLength(),
		ESplineCoordinateSpace::World
	);

	// Calculate distances
	float DistToStart = FVector::Dist(FromEndPoint, ToStartPoint);
	float DistToEnd = FVector::Dist(FromEndPoint, ToEndPoint);

	// Connection tolerance in cm (5 meters = 500 cm)
	const float ConnectionTolerance = 500.0f;

	UE_LOG(LogTemp, Log, TEXT("🔍 DetectRoadConnection: '%s' → '%s' | DistToStart: %.0f cm | DistToEnd: %.0f cm | Tolerance: %.0f cm"),
		*FromRoad->RoadName, *ToRoad->RoadName, DistToStart, DistToEnd, ConnectionTolerance);

	// Check if connected to start of ToRoad
	if (DistToStart < ConnectionTolerance)
	{
		OutStartDistance = 0.0f;
		OutShouldReverse = false;
		UE_LOG(LogTemp, Log, TEXT("  → Connected to START of '%s'"), *ToRoad->RoadName);
		return true;
	}

	// Check if connected to end of ToRoad (travel in reverse)
	if (DistToEnd < ConnectionTolerance)
	{
		OutStartDistance = ToRoad->RoadSpline->GetSplineLength();
		OutShouldReverse = true;
		UE_LOG(LogTemp, Log, TEXT("  → Connected to END of '%s' (REVERSE)"), *ToRoad->RoadName);
		return true;
	}

	// Not connected within tolerance
	UE_LOG(LogTemp, Warning, TEXT("  → ❌ NOT CONNECTED (both distances > tolerance)"));

	return false;
}

void USplineMovementComponent::UpdateTransitionRotation(float DeltaTime)
{
	if (!bIsTransitioning)
	{
		return;
	}

	// Skip if position interpolation is handling rotation
	if (bIsInterpolatingPosition)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !CurrentSpline)
	{
		bIsTransitioning = false;
		return;
	}

	// Update transition timer
	TransitionTimeRemaining -= DeltaTime;

	if (TransitionTimeRemaining <= 0.0f)
	{
		// Transition complete
		bIsTransitioning = false;

		// Snap to target rotation
		Owner->SetActorRotation(TransitionTargetRotation);
	}
	else
	{
		// Calculate interpolation alpha (0 to 1)
		float Alpha = 1.0f - (TransitionTimeRemaining / TransitionDuration);
		Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

		// Smooth interpolation using ease-in-out
		Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

		// Interpolate rotation using Quaternion Slerp (takes shortest path)
		FQuat StartQuat = TransitionStartRotation.Quaternion();
		FQuat TargetQuat = TransitionTargetRotation.Quaternion();
		FQuat SmoothedQuat = FQuat::Slerp(StartQuat, TargetQuat, Alpha);
		FRotator SmoothedRotation = SmoothedQuat.Rotator();

		// Apply smoothed rotation
		Owner->SetActorRotation(SmoothedRotation);
	}
}

void USplineMovementComponent::UpdatePositionInterpolation(float DeltaTime)
{
	if (!bIsInterpolatingPosition)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !CurrentSpline)
	{
		bIsInterpolatingPosition = false;
		return;
	}

	// Update interpolation timer
	PositionInterpolationTimeRemaining -= DeltaTime;

	if (PositionInterpolationTimeRemaining <= 0.0f)
	{
		// Interpolation complete - snap to target and continue normal movement
		bIsInterpolatingPosition = false;

		// Set to target position and rotation (already correct from interpolation)
		Owner->SetActorLocation(PositionInterpolationTarget);
		Owner->SetActorRotation(PositionInterpolationTargetRotation);

		// DON'T call UpdateTransform() here - it would override the interpolated rotation
		// The next frame will call it naturally through UpdateMovement()

		UE_LOG(LogTemp, Log, TEXT("✅ Position + Rotation Interpolation Complete - Now following spline normally"));
	}
	else
	{
		// Calculate interpolation alpha (0 to 1)
		float Alpha = 1.0f - (PositionInterpolationTimeRemaining / PositionInterpolationDuration);
		Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

		// Smooth interpolation using ease-in-out
		Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

		// Interpolate position linearly
		FVector SmoothedPosition = FMath::Lerp(PositionInterpolationStart, PositionInterpolationTarget, Alpha);

		// Interpolate rotation using Quaternion Slerp (takes shortest path, avoids gimbal lock)
		FQuat StartQuat = PositionInterpolationStartRotation.Quaternion();
		FQuat TargetQuat = PositionInterpolationTargetRotation.Quaternion();
		FQuat SmoothedQuat = FQuat::Slerp(StartQuat, TargetQuat, Alpha);
		FRotator SmoothedRotation = SmoothedQuat.Rotator();

		// Apply smoothed position and rotation together
		Owner->SetActorLocation(SmoothedPosition);
		Owner->SetActorRotation(SmoothedRotation);
	}
}
