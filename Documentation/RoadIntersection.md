# RoadIntersection

## Overview

`ARoadIntersection` is an Actor that manages road intersections, handling multiple road connections and generating smooth transition curves for vehicles to follow.

**File Location:** `Source/ai27Simulator/Public/RoadSystem/RoadIntersection.h`
**Implementation:** `Source/ai27Simulator/Private/RoadSystem/RoadIntersection.cpp`

## Class Declaration

```cpp
UCLASS()
class AI27SIMULATOR_API ARoadIntersection : public AActor
```

## Features

- **Multi-Road Connections**: Handle 2+ roads meeting at a point
- **Connection Types**: Support for incoming, outgoing, and bidirectional traffic
- **Automatic Curve Generation**: Create smooth transition splines between roads
- **Smart Route Selection**: Choose next road based on various modes
- **Debug Visualization**: Visual feedback for connections in editor
- **Traffic Light Support**: Architecture ready for future signal implementation

## Enumerations

### EConnectionType

Defines the type of traffic flow at a connection point.

```cpp
UENUM(BlueprintType)
enum EConnectionType
{
    Incoming,       // Traffic flows INTO intersection
    Outgoing,       // Traffic flows OUT OF intersection
    Bidirectional   // Traffic can flow both ways (most common)
};
```

### EIntersectionType

Categorizes the intersection layout.

```cpp
UENUM(BlueprintType)
enum EIntersectionType
{
    TwoWay,      // Two roads meeting (L shape)
    ThreeWay,    // T or Y intersection
    FourWay,     // Cross or X intersection
    Roundabout,  // Circular with multiple exits
    Custom       // Custom configuration
};
```

## Structs

### FRoadConnectionPoint

Stores information about a road connection at the intersection.

```cpp
USTRUCT(BlueprintType)
struct FRoadConnectionPoint
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ARoadSplineActor* Road;              // The connected road

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bConnectedAtStart;              // Connected at road's start?

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TEnumAsByte<EConnectionType> ConnectionType;  // Traffic flow type

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float ConnectionAngle;               // Angle from center (0-360)

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector ConnectionPoint;             // World location of connection
};
```

## Components

### SceneRoot

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
USceneComponent* SceneRoot;
```

Root component for the actor.

### IntersectionIcon

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
UBillboardComponent* IntersectionIcon;
```

Billboard icon for visibility in the editor viewport.

## Properties

### Intersection Configuration

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `IntersectionName` | `FString` | "Intersection" | Display name for identification |
| `Connections` | `TArray<FRoadConnectionPoint>` | Empty | All connected roads |
| `IntersectionRadius` | `float` | 500.0f | Radius in cm (affects curve tightness) |
| `IntersectionType` | `EIntersectionType` | FourWay | Type of intersection |

### Debug Settings

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `bShowDebugConnections` | `bool` | true | Show debug lines for connections |
| `bShowConnectionAngles` | `bool` | false | Show angle labels |

## Navigation Functions

### GetOutgoingRoads

Get all valid roads a vehicle can transition to from a given incoming road.

```cpp
UFUNCTION(BlueprintCallable, Category = "Intersection")
TArray<ARoadSplineActor*> GetOutgoingRoads(ARoadSplineActor* IncomingRoad) const;
```

**Parameters:**
- `IncomingRoad`: The road the vehicle is coming from

**Returns:** Array of roads the vehicle can transition to

**Logic:**
- Excludes the incoming road
- Only includes roads with `Outgoing` or `Bidirectional` connection types

### ChooseNextRoad

Select the next road based on a transition mode.

```cpp
UFUNCTION(BlueprintCallable, Category = "Intersection")
ARoadSplineActor* ChooseNextRoad(
    ARoadSplineActor* IncomingRoad,
    TEnumAsByte<ETransitionMode> TransitionMode) const;
```

**Parameters:**
- `IncomingRoad`: Road vehicle is coming from
- `TransitionMode`: Selection mode (Random, First, Last)

**Returns:** Selected outgoing road, or `nullptr` if none available

### GenerateTransitionCurve

Create a smooth spline curve between two roads.

```cpp
UFUNCTION(BlueprintCallable, Category = "Intersection")
USplineComponent* GenerateTransitionCurve(
    ARoadSplineActor* FromRoad,
    ARoadSplineActor* ToRoad);
```

**Parameters:**
- `FromRoad`: Road the vehicle is coming from
- `ToRoad`: Road the vehicle is going to

**Returns:** Generated spline component (temporary, cleaned up after use)

**Curve Generation Process:**

1. Get connection points for both roads
2. Calculate start point and tangent from FromRoad's end
3. Calculate end point and tangent from ToRoad's start
4. Normalize tangents and scale by `IntersectionRadius`
5. Create spline with two points and tangents
6. Store for cleanup tracking

## Utility Functions

### UpdateConnectionPoints

Recalculate connection points and angles for all connections.

```cpp
UFUNCTION(CallInEditor, BlueprintCallable, Category = "Intersection")
void UpdateConnectionPoints();
```

Called automatically on construction and property changes. Can also be called manually after modifying the Connections array.

**Calculations:**
- Connection point based on road start/end
- Angle calculated in XY plane from intersection center
- Connections sorted by angle for debugging

### GetConnectionCount

```cpp
UFUNCTION(BlueprintPure, Category = "Intersection")
int32 GetConnectionCount() const { return Connections.Num(); }
```

## Debug Visualization

When `bShowDebugConnections` is enabled, the intersection draws:

1. **Yellow Circle**: Intersection radius boundary
2. **Connection Lines**: From center to each connection point
   - **Red**: Incoming connections
   - **Blue**: Outgoing connections
   - **Green**: Bidirectional connections
3. **Spheres**: At each connection point (color-coded)
4. **Angle Labels**: If `bShowConnectionAngles` is enabled

## Usage Examples

### Setting Up an Intersection in Editor

1. Place `RoadIntersection` at the meeting point of roads
2. In Details panel, expand `Connections`
3. Add array elements for each connected road
4. For each connection:
   - Assign the `Road` reference
   - Set `bConnectedAtStart` (true if road's start point connects)
   - Set `ConnectionType` (usually `Bidirectional`)
5. Click "Update Connection Points" or move the actor to refresh

### Using Intersection for Vehicle Transitions

```cpp
// In vehicle code when reaching end of road
ARoadIntersection* Intersection = FindNearbyIntersection();
if (Intersection)
{
    // Get transition curve
    USplineComponent* TransitionCurve = Intersection->GenerateTransitionCurve(
        CurrentRoad,
        NextRoad
    );

    // Follow the curve
    MovementComponent->SwitchToNewSplineComponent(TransitionCurve, true);
}
```

### Choosing Next Road

```cpp
// Random selection
ARoadSplineActor* NextRoad = Intersection->ChooseNextRoad(
    CurrentRoad,
    ETransitionMode::Random
);

// Always first available
NextRoad = Intersection->ChooseNextRoad(
    CurrentRoad,
    ETransitionMode::First
);
```

## Transition Curve Details

### Curve Geometry

The transition curve is a cubic Bezier spline with:

- **Start Point**: End point of FromRoad
- **Start Tangent**: Direction vehicle was traveling, scaled by radius
- **End Point**: Start point of ToRoad
- **End Tangent**: Direction of ToRoad at connection, scaled by radius

### Tangent Handling

For roads connected at their **start**:
- Start tangent is reversed (vehicle was traveling "into" the start)

For roads connected at their **end**:
- End tangent is reversed (vehicle will travel "out of" the end)

### Curve Smoothness

The `IntersectionRadius` affects turn tightness:
- Larger radius = wider, smoother turns
- Smaller radius = tighter turns

Recommended values:
- City streets: 300-500 cm
- Major roads: 500-800 cm
- Highways: 800-1500 cm

## Connection Types Explained

### Incoming

- Vehicles can enter the intersection from this road
- Cannot exit to this road
- Use for one-way streets ending at intersection

### Outgoing

- Vehicles can exit the intersection to this road
- Cannot enter from this road
- Use for one-way streets starting from intersection

### Bidirectional

- Most common type
- Vehicles can both enter and exit
- Use for normal two-way roads

## Lifecycle Events

### OnConstruction

Updates connection points when placed/moved in editor.

### PostEditChangeProperty

Recalculates connection points when Connections or IntersectionRadius changes.

### BeginPlay

- Updates connection points
- Logs intersection info

### Tick

Draws debug visualization when enabled.

## Internal Functions

### FindConnection

Find connection info for a specific road:

```cpp
FRoadConnectionPoint* FindConnection(ARoadSplineActor* Road);
const FRoadConnectionPoint* FindConnection(ARoadSplineActor* Road) const;
```

## Best Practices

1. **Position Accurately**: Place intersection at the actual meeting point of roads
2. **Match Connection Types**: Ensure traffic flow makes sense
3. **Use Reasonable Radius**: Match to road width and expected vehicle speeds
4. **Test Transitions**: Use debug visualization to verify smooth curves
5. **Update After Changes**: Call UpdateConnectionPoints after modifying connections

## Related Classes

- [`ARoadSplineActor`](RoadSplineActor.md) - Road segments
- [`USplineMovementComponent`](SplineMovementComponent.md) - Vehicle movement
- [`ATestVehicle`](TestVehicle.md) - Example vehicle with intersection support

---

*Copyright 2025 AI27. All Rights Reserved.*
*Designer: Aldo Maradon Duran Bautista*
