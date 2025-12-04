# SplineMovementComponent

## Overview

`USplineMovementComponent` is an ActorComponent designed to move actors along splines without using physics simulation. It provides smooth, efficient movement optimized for mobile platforms.

**File Location:** `Source/ai27Simulator/Public/Components/SplineMovementComponent.h`
**Implementation:** `Source/ai27Simulator/Private/Components/SplineMovementComponent.cpp`

## Class Declaration

```cpp
UCLASS(ClassGroup=(AI27), meta=(BlueprintSpawnableComponent))
class AI27SIMULATOR_API USplineMovementComponent : public UActorComponent
```

## Features

- **Physics-free Movement**: Pure position interpolation along splines
- **Speed Control**: Supports cm/s and km/h conversion
- **Acceleration/Deceleration**: Smooth speed transitions
- **Smart Road Connections**: Automatic detection of connected roads
- **Smooth Transitions**: Position and rotation interpolation between roads
- **Events**: Delegates for end-of-spline and speed changes
- **Mobile Optimized**: Efficient performance for mobile devices
- **Blueprint Support**: Full Blueprint exposure

## Properties

### Current Spline Reference

| Property | Type | Description |
|----------|------|-------------|
| `CurrentRoad` | `ARoadSplineActor*` | The road actor currently being followed |
| `CurrentSpline` | `USplineComponent*` | The spline component currently being followed |

### Movement Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `MaxSpeed` | `float` | 8000.0f | Maximum speed in cm/s (~80 km/h) |
| `Acceleration` | `float` | 500.0f | Acceleration rate in cm/s^2 |
| `Deceleration` | `float` | 1000.0f | Braking rate in cm/s^2 |
| `CurrentSpeed` | `float` | 0.0f | Current speed in cm/s (read-only) |
| `DistanceAlongSpline` | `float` | 0.0f | Current distance traveled in cm |

### Control Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `bAutoMove` | `bool` | true | If true, movement happens automatically in Tick |
| `bIsMoving` | `bool` | false | Is the actor currently moving? (read-only) |
| `bLoopAtEnd` | `bool` | false | Loop back to start when reaching end? |

## Core Functions

### Starting Movement

```cpp
// Start following a RoadSplineActor
UFUNCTION(BlueprintCallable, Category = "Movement")
void StartFollowingSpline(ARoadSplineActor* Road);

// Start following a SplineComponent directly
UFUNCTION(BlueprintCallable, Category = "Movement")
void StartFollowingSplineComponent(USplineComponent* Spline);
```

### Movement Control

```cpp
// Stop movement (decelerates to zero)
UFUNCTION(BlueprintCallable, Category = "Movement")
void StopMovement();

// Resume movement (accelerates to max speed)
UFUNCTION(BlueprintCallable, Category = "Movement")
void ResumeMovement();

// Set max speed in cm/s
UFUNCTION(BlueprintCallable, Category = "Movement")
void SetSpeed(float NewSpeed);

// Set speed in km/h (converts internally)
UFUNCTION(BlueprintCallable, Category = "Movement")
void SetSpeedKmH(float SpeedKmH);
```

### Switching Splines

```cpp
// Switch to a new road (useful for intersections)
UFUNCTION(BlueprintCallable, Category = "Movement")
void SwitchToNewSpline(ARoadSplineActor* NewRoad, bool bMaintainSpeed = true);

// Switch to a new spline component directly
UFUNCTION(BlueprintCallable, Category = "Movement")
void SwitchToNewSplineComponent(USplineComponent* NewSpline, bool bMaintainSpeed = true);
```

### Query Functions

```cpp
// Get current speed in km/h
UFUNCTION(BlueprintPure, Category = "Movement")
float GetSpeedKmH() const;

// Get progress as percentage (0-100)
UFUNCTION(BlueprintPure, Category = "Movement")
float GetProgressPercent() const;

// Get remaining distance to end in cm
UFUNCTION(BlueprintPure, Category = "Movement")
float GetRemainingDistance() const;

// Is currently following a spline?
UFUNCTION(BlueprintPure, Category = "Movement")
bool IsFollowingSpline() const;
```

## Events

### OnReachedEnd

Fired when the actor reaches the end of the spline.

```cpp
UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
FOnSplineEnd OnReachedEnd;
```

**Delegate Signature:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSplineEnd);
```

### OnSpeedChanged

Fired when speed changes by more than 5 km/h.

```cpp
UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
FOnSpeedChanged OnSpeedChanged;
```

**Delegate Signature:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeedChanged, float, NewSpeedKmH);
```

## Speed Conversion

The component uses cm/s internally but provides km/h functions for convenience:

- **cm/s to km/h**: `speed_kmh = speed_cms * 0.036`
- **km/h to cm/s**: `speed_cms = speed_kmh * 27.778`

| km/h | cm/s |
|------|------|
| 60 | 1667 |
| 80 | 2222 |
| 100 | 2778 |
| 120 | 3333 |

## Transition System

The component includes a sophisticated transition system for smooth road switches:

### Rotation Transition

When switching roads, rotation is interpolated using Quaternion Slerp to avoid gimbal lock:

- **Duration**: 0.5 seconds
- **Interpolation**: Smooth step (ease-in-out)

### Position Interpolation

For small gaps between roads (< 500 cm):

- **Duration**: 0.3 seconds
- **Interpolation**: Linear position with Quaternion Slerp for rotation
- **Max Gap**: 500 cm (5 meters)

### Smart Road Detection

The `DetectRoadConnection()` function automatically determines:

1. Whether two roads are connected
2. The starting distance on the new road (0 or spline length)
3. Whether direction should be reversed

**Connection Tolerance**: 500 cm (5 meters)

## Usage Examples

### Basic Usage (Blueprint)

1. Add `SplineMovementComponent` to an Actor
2. Get reference to a `RoadSplineActor`
3. Call `StartFollowingSpline(Road)`

### C++ Example

```cpp
// In your actor's BeginPlay
USplineMovementComponent* Movement = FindComponentByClass<USplineMovementComponent>();
if (Movement && StartingRoad)
{
    Movement->SetSpeedKmH(80.0f);  // Set to 80 km/h
    Movement->StartFollowingSpline(StartingRoad);
}
```

### Event Binding

```cpp
void AMyVehicle::BeginPlay()
{
    Super::BeginPlay();

    Movement->OnReachedEnd.AddDynamic(this, &AMyVehicle::OnEndOfRoad);
    Movement->OnSpeedChanged.AddDynamic(this, &AMyVehicle::OnSpeedChange);
}

void AMyVehicle::OnEndOfRoad()
{
    // Handle end of road
}

void AMyVehicle::OnSpeedChange(float NewSpeedKmH)
{
    // Handle speed change
}
```

## Debug Visualization

In Play-In-Editor mode, the component displays debug information above the owner actor:

- Current speed in km/h
- Progress percentage
- Movement state (Yes/No)

## Internal Implementation

### UpdateMovement (TickComponent)

Called every frame when `bAutoMove` is true:

1. Accelerate or decelerate based on `bIsMoving`
2. Update `DistanceAlongSpline` based on current speed
3. Check for end-of-spline condition
4. Update actor transform (if not interpolating)
5. Update transition interpolations
6. Fire speed change events if needed
7. Draw debug visualization

### UpdateTransform

Sets the owner actor's position and rotation based on current distance along spline:

```cpp
FVector Location = CurrentSpline->GetLocationAtDistanceAlongSpline(
    DistanceAlongSpline, ESplineCoordinateSpace::World);

FRotator Rotation = CurrentSpline->GetRotationAtDistanceAlongSpline(
    DistanceAlongSpline, ESplineCoordinateSpace::World);

Owner->SetActorLocationAndRotation(Location, Rotation);
```

## Performance Considerations

- No physics simulation overhead
- Single spline query per frame
- Smooth interpolation uses simple math
- Debug visualization only in PIE mode

## Related Classes

- [`ARoadSplineActor`](RoadSplineActor.md) - Road definition
- [`ARoadIntersection`](RoadIntersection.md) - Intersection handling
- [`ATestVehicle`](TestVehicle.md) - Example vehicle using this component

---

*Copyright 2025 AI27. All Rights Reserved.*
*Designer: Aldo Maradon Duran Bautista*
