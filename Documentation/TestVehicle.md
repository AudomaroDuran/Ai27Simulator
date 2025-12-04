# TestVehicle

## Overview

`ATestVehicle` is a Pawn-based actor that demonstrates vehicle movement along roads using the SplineMovementComponent. It provides automatic road following, transitions between roads, and intersection support.

**File Location:** `Source/ai27Simulator/Public/Vehicles/TestVehicle.h`
**Implementation:** `Source/ai27Simulator/Private/Vehicles/TestVehicle.cpp`

## Class Declaration

```cpp
UCLASS()
class AI27SIMULATOR_API ATestVehicle : public APawn
```

## Features

- **Spline-Based Movement**: Uses SplineMovementComponent for smooth path following
- **Auto-Start**: Optional automatic movement on BeginPlay
- **Road Assignment**: Easy assignment to any road in the level
- **Auto-Transition**: Automatic switching between connected roads
- **Intersection Support**: Smart navigation through RoadIntersection actors
- **Multiple Transition Modes**: Random, First, or Last road selection
- **Visual Representation**: Default cube mesh scaled to car-like proportions

## Enumeration

### ETransitionMode

Defines how to select the next road when multiple are connected.

```cpp
UENUM(BlueprintType)
enum ETransitionMode : uint8
{
    Random,  // Pick random connected road
    First,   // Always pick first connected road
    Last     // Always pick last connected road
};
```

## Components

### VehicleMesh

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
UStaticMeshComponent* VehicleMesh;
```

Visual representation of the vehicle. By default, uses a scaled cube mesh (2x1x0.5) to approximate car proportions.

### MovementComponent

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
USplineMovementComponent* MovementComponent;
```

The SplineMovementComponent that handles all movement logic.

## Configuration Properties

### Basic Setup

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `StartingRoad` | `ARoadSplineActor*` | nullptr | Road to start following on BeginPlay |
| `bAutoStart` | `bool` | true | Start moving automatically |
| `InitialSpeedKmH` | `float` | 60.0f | Initial speed in km/h |
| `VehicleName` | `FString` | "Test Vehicle" | Display name for logs |

### Auto-Transition Configuration

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `bAutoTransition` | `bool` | true | Auto-switch to next road at end |
| `TransitionMode` | `ETransitionMode` | Random | How to choose next road |
| `bUseIntersections` | `bool` | true | Use RoadIntersection for smooth curves |
| `IntersectionSearchRadius` | `float` | 1000.0f | Search radius for intersections (cm) |

## Control Functions

### AssignToRoad

Assign the vehicle to a road and start following it.

```cpp
UFUNCTION(BlueprintCallable, Category = "Vehicle")
void AssignToRoad(ARoadSplineActor* Road);
```

**Parameters:**
- `Road`: The road to follow

**Behavior:**
1. Sets speed to `InitialSpeedKmH`
2. Calls `MovementComponent->StartFollowingSpline(Road)`

### StopVehicle

Stop the vehicle (decelerates to zero).

```cpp
UFUNCTION(BlueprintCallable, Category = "Vehicle")
void StopVehicle();
```

### ResumeVehicle

Resume vehicle movement (accelerates to max speed).

```cpp
UFUNCTION(BlueprintCallable, Category = "Vehicle")
void ResumeVehicle();
```

### SetVehicleSpeed

Set the vehicle's maximum speed.

```cpp
UFUNCTION(BlueprintCallable, Category = "Vehicle")
void SetVehicleSpeed(float SpeedKmH);
```

## Query Functions

### IsMoving

```cpp
UFUNCTION(BlueprintPure, Category = "Vehicle")
bool IsMoving() const;
```

Returns `true` if the vehicle is currently moving.

### GetCurrentSpeed

```cpp
UFUNCTION(BlueprintPure, Category = "Vehicle")
float GetCurrentSpeed() const;
```

Returns current speed in km/h.

### GetProgress

```cpp
UFUNCTION(BlueprintPure, Category = "Vehicle")
float GetProgress() const;
```

Returns progress along current road as percentage (0-100%).

## Event Handlers

### OnReachedEndOfRoad

Called when the vehicle reaches the end of the current road.

**Logic Flow:**

1. Check if `bAutoTransition` is enabled
2. If `bUseIntersections`, search for nearby RoadIntersection
3. If intersection found, use `TransitionThroughIntersection()`
4. If no intersection, get connected roads from current road
5. Choose next road based on `TransitionMode`
6. Switch to next road maintaining speed

### OnSpeedChanged

Called when the vehicle's speed changes significantly. Can be overridden for UI updates or other reactions.

## Intersection Navigation

### FindNearbyIntersection

Search for a RoadIntersection near the current road's end.

```cpp
ARoadIntersection* FindNearbyIntersection() const;
```

**Search Process:**
1. Get current road's end position
2. Find all RoadIntersection actors in level
3. Return closest one within `IntersectionSearchRadius`

### TransitionThroughIntersection

Handle vehicle transition using an intersection.

```cpp
bool TransitionThroughIntersection(
    ARoadIntersection* Intersection,
    ARoadSplineActor* FromRoad);
```

**Process:**
1. Get next road from intersection based on TransitionMode
2. Generate transition curve
3. Store pending target road
4. Switch to following the curve
5. Bind to curve completion event

### OnTransitionCurveComplete

Called when the vehicle finishes following a transition curve.

**Cleanup:**
1. Unbind from curve end event
2. Switch to the pending target road
3. Destroy transition curve component
4. Re-bind to normal end-of-road handler

## Internal State

| Property | Type | Description |
|----------|------|-------------|
| `CurrentTransitionCurve` | `USplineComponent*` | Active transition curve |
| `PendingTargetRoad` | `ARoadSplineActor*` | Road to switch to after curve |
| `bFollowingTransitionCurve` | `bool` | Currently on a transition curve |

## Usage Examples

### Basic Setup in Editor

1. Place `TestVehicle` in the level
2. In Details panel:
   - Assign `StartingRoad` to a RoadSplineActor
   - Enable `bAutoStart` (default)
   - Set `InitialSpeedKmH` (e.g., 80)
3. Press Play - vehicle moves automatically

### Spawning at Runtime

```cpp
// Spawn vehicle
ATestVehicle* Vehicle = GetWorld()->SpawnActor<ATestVehicle>(
    ATestVehicle::StaticClass(),
    SpawnLocation,
    SpawnRotation
);

// Configure
Vehicle->VehicleName = TEXT("Spawned Vehicle");
Vehicle->TransitionMode = ETransitionMode::Random;

// Assign to road
Vehicle->AssignToRoad(SomeRoad);
```

### Controlling Vehicle via Blueprint

```
Event BeginPlay:
    -> Get Reference to TestVehicle
    -> Set Vehicle Speed (100.0)  // Set to 100 km/h
    -> Assign To Road (RoadReference)

Event OnKeyPress (Space):
    -> If IsMoving()
        -> Stop Vehicle
    -> Else
        -> Resume Vehicle
```

### Disabling Auto-Transition

```cpp
// Keep vehicle from automatically switching roads
Vehicle->bAutoTransition = false;

// Handle end of road manually
Vehicle->MovementComponent->OnReachedEnd.AddDynamic(
    this,
    &AMyController::HandleEndOfRoad
);
```

### Custom Transition Logic

```cpp
void AMyVehicle::OnReachedEndOfRoad()
{
    // Get connected roads
    TArray<ARoadSplineActor*> NextRoads = CurrentRoad->GetRoadsAtEnd();

    // Custom logic - pick road with highest speed limit
    ARoadSplineActor* BestRoad = nullptr;
    float HighestSpeed = 0;

    for (ARoadSplineActor* Road : NextRoads)
    {
        if (Road->SpeedLimit > HighestSpeed)
        {
            HighestSpeed = Road->SpeedLimit;
            BestRoad = Road;
        }
    }

    if (BestRoad)
    {
        MovementComponent->SwitchToNewSpline(BestRoad, true);
    }
}
```

## Default Mesh

The vehicle uses a cube mesh from `/Engine/BasicShapes/Cube` with scale:
- X: 2.0 (length)
- Y: 1.0 (width)
- Z: 0.5 (height)

This creates a basic car-like shape. Replace with your own mesh by:

1. Creating a Blueprint subclass
2. Swapping the static mesh in the VehicleMesh component
3. Or setting `VehicleMesh->SetStaticMesh()` in code

## Event Binding

The vehicle automatically binds to MovementComponent events in BeginPlay:

```cpp
MovementComponent->OnReachedEnd.AddDynamic(this, &ATestVehicle::OnReachedEndOfRoad);
MovementComponent->OnSpeedChanged.AddDynamic(this, &ATestVehicle::OnSpeedChanged);
```

When using intersection transitions, the vehicle temporarily rebinds the OnReachedEnd event to detect curve completion.

## Transition Mode Selection

### Random

Best for simulating realistic traffic where vehicles take various paths.

```cpp
int32 RandomIndex = FMath::RandRange(0, ConnectedRoads.Num() - 1);
return ConnectedRoads[RandomIndex];
```

### First

Useful for deterministic testing or creating a specific route.

```cpp
return ConnectedRoads[0];
```

### Last

Alternative deterministic option, useful when roads are ordered specifically.

```cpp
return ConnectedRoads[ConnectedRoads.Num() - 1];
```

## Best Practices

1. **Use Intersections**: Enable `bUseIntersections` for smooth turns
2. **Set Reasonable Radius**: Match `IntersectionSearchRadius` to your road layout
3. **Test Transitions**: Verify vehicles navigate correctly at all junctions
4. **Name Vehicles**: Use `VehicleName` for debugging multi-vehicle scenarios
5. **Monitor Speed**: Use the OnSpeedChanged event for speed displays

## Related Classes

- [`USplineMovementComponent`](SplineMovementComponent.md) - Core movement logic
- [`ARoadSplineActor`](RoadSplineActor.md) - Road definition
- [`ARoadIntersection`](RoadIntersection.md) - Intersection handling

---

*Copyright 2025 AI27. All Rights Reserved.*
*Designer: Aldo Maradon Duran Bautista*
