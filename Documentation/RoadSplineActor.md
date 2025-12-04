# RoadSplineActor

## Overview

`ARoadSplineActor` is an Actor that represents a road segment based on a spline curve. It can be connected with other RoadSplineActors to form a complete road network.

**File Location:** `Source/ai27Simulator/Public/RoadSystem/RoadSplineActor.h`
**Implementation:** `Source/ai27Simulator/Private/RoadSystem/RoadSplineActor.cpp`

## Class Declaration

```cpp
UCLASS()
class AI27SIMULATOR_API ARoadSplineActor : public AActor
```

## Features

- **Visual Spline Editing**: Edit road path directly in the Unreal Editor viewport
- **Auto Mesh Generation**: Optional automatic road mesh generation along spline
- **Road Properties**: Configurable width, lanes, speed limit
- **Risk Zone Marking**: Mark dangerous road segments
- **Road Connections**: Connect to other roads for network building
- **Navigation Queries**: Get positions, rotations, and distances along road

## Components

### RoadSpline

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road|Components")
USplineComponent* RoadSpline;
```

Main spline component that defines the road path. Edit spline points in the viewport to shape the road.

### SceneRoot

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road|Components")
USceneComponent* SceneRoot;
```

Root component for the actor hierarchy.

## Road Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `RoadWidth` | `float` | 800.0f | Width in cm (800 = 8 meters) |
| `NumLanes` | `int32` | 2 | Number of lanes |
| `SpeedLimit` | `float` | 80.0f | Speed limit in km/h |
| `bIsHighway` | `bool` | false | Highway flag (affects traffic behavior) |
| `bIsRiskZone` | `bool` | false | Risk zone flag (triggers alerts) |
| `RoadName` | `FString` | "Road" | Display name for identification |

## Visual Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `bGenerateRoadMesh` | `bool` | false | Generate visual mesh along spline |
| `RoadMeshSegment` | `UStaticMesh*` | nullptr | Static mesh for road segments |
| `RoadMaterial` | `UMaterialInterface*` | nullptr | Material for road surface |
| `RoadColor` | `FLinearColor` | Gray | Color tint (auto-red for risk zones) |

## Navigation Functions

### GetLocationAtDistance

Get world location at a specific distance along the road.

```cpp
UFUNCTION(BlueprintCallable, Category = "Road|Navigation")
FVector GetLocationAtDistance(float Distance) const;
```

**Parameters:**
- `Distance`: Distance in cm from the start of the spline

**Returns:** World location at that distance

### GetLocationAtTime

Get location at normalized time (0-1) along the spline.

```cpp
UFUNCTION(BlueprintCallable, Category = "Road|Navigation")
FVector GetLocationAtTime(float Time) const;
```

**Parameters:**
- `Time`: 0 = start, 1 = end

**Returns:** World location at that time

### GetRotationAtDistance

Get rotation at a specific distance along the road.

```cpp
UFUNCTION(BlueprintCallable, Category = "Road|Navigation")
FRotator GetRotationAtDistance(float Distance) const;
```

### GetSplineLength

Get total length of the road spline.

```cpp
UFUNCTION(BlueprintPure, Category = "Road|Navigation")
float GetSplineLength() const;
```

**Returns:** Total length in cm

### GetClosestLocationOnSpline

Find the closest point on the road to a given world location.

```cpp
UFUNCTION(BlueprintCallable, Category = "Road|Navigation")
FVector GetClosestLocationOnSpline(const FVector& WorldLocation, float& OutDistance) const;
```

**Parameters:**
- `WorldLocation`: The world position to query
- `OutDistance`: (out) Distance along spline to the closest point

**Returns:** Closest point on the road spline

### IsLocationOnRoad

Check if a world location is within the road bounds.

```cpp
UFUNCTION(BlueprintPure, Category = "Road|Navigation")
bool IsLocationOnRoad(const FVector& WorldLocation, float Tolerance = 500.0f) const;
```

**Parameters:**
- `WorldLocation`: World position to check
- `Tolerance`: Additional tolerance in cm (default: 500)

**Returns:** `true` if within road bounds

## Connection System

### ConnectedRoads

Array of roads connected to this one:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Connections")
TArray<ARoadSplineActor*> ConnectedRoads;
```

### ConnectToRoad

Programmatically connect this road to another.

```cpp
UFUNCTION(BlueprintCallable, Category = "Road|Connections")
void ConnectToRoad(ARoadSplineActor* OtherRoad, bool bAtStart = false);
```

**Parameters:**
- `OtherRoad`: Road to connect to
- `bAtStart`: If true, connects at start of this road; if false, at end

**Note:** Connections are automatically made bidirectional.

### GetRoadsAtStart

Get all roads connected at the start of this road.

```cpp
UFUNCTION(BlueprintPure, Category = "Road|Connections")
TArray<ARoadSplineActor*> GetRoadsAtStart() const;
```

### GetRoadsAtEnd

Get all roads connected at the end of this road.

```cpp
UFUNCTION(BlueprintPure, Category = "Road|Connections")
TArray<ARoadSplineActor*> GetRoadsAtEnd() const;
```

## Debug Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `bShowDebugSpline` | `bool` | false | Show debug spline visualization |
| `bShowRoadBounds` | `bool` | false | Show road width bounds |

## Internal Structure

### FRoadConnection

Internal struct for tracking connection details:

```cpp
struct FRoadConnection
{
    ARoadSplineActor* ConnectedRoad;
    bool bConnectedAtStart;  // Of this road
};
```

## Mesh Generation

When `bGenerateRoadMesh` is enabled with a valid `RoadMeshSegment`:

1. Clears existing mesh segments
2. Calculates number of segments (1 per 10 meters)
3. Creates `USplineMeshComponent` for each segment
4. Sets start/end positions and tangents from spline
5. Applies material and scaling

```cpp
// Internal mesh generation
void GenerateRoadMesh();
void ClearRoadMesh();
```

**Segment Size:** Every 1000 cm (10 meters)

## Lifecycle Events

### OnConstruction

Called when the actor is placed or modified in editor:

- Regenerates road mesh if enabled
- Updates road color (red for risk zones)

### PostEditChangeProperty

Called when properties change in editor:

- Regenerates mesh when `bGenerateRoadMesh`, `RoadMeshSegment`, or `RoadWidth` changes

### BeginPlay

Logs road information:

```
RoadSplineActor 'Road Name': Length=XXXX cm, Lanes=2, Speed=80 km/h
```

## Usage Examples

### Placing a Road in Editor

1. Drag `RoadSplineActor` into the level
2. Select the actor and find the spline in the viewport
3. Click on the spline to add/edit points
4. Configure properties in the Details panel:
   - Set `RoadName` for identification
   - Set `SpeedLimit` for the road
   - Set `NumLanes` as needed
   - Enable `bIsRiskZone` if dangerous

### Connecting Roads

**In Editor:**
1. Select the road
2. Find `ConnectedRoads` in Details panel
3. Add references to other roads

**In C++:**
```cpp
Road1->ConnectToRoad(Road2, false);  // Connect Road1's end to Road2
```

### Generating Road Mesh

1. Create a static mesh for road segments (plane or custom)
2. Assign to `RoadMeshSegment`
3. Optionally assign `RoadMaterial`
4. Enable `bGenerateRoadMesh`

### Querying Road Information

```cpp
// Get position at halfway point
float HalfwayDistance = Road->GetSplineLength() / 2.0f;
FVector MidPoint = Road->GetLocationAtDistance(HalfwayDistance);

// Check if player is on road
FVector PlayerLocation = Player->GetActorLocation();
if (Road->IsLocationOnRoad(PlayerLocation))
{
    // Player is on this road
}

// Find closest point on road
float DistanceAlongRoad;
FVector ClosestPoint = Road->GetClosestLocationOnSpline(SomeLocation, DistanceAlongRoad);
```

## Blueprint Usage

All properties and functions are exposed to Blueprints:

- Properties use `EditAnywhere` and `BlueprintReadWrite`
- Functions use `BlueprintCallable` or `BlueprintPure`
- Full tooltip support for in-editor documentation

## Performance Notes

- `PrimaryActorTick.bCanEverTick = false` - No tick overhead by default
- Tick only enabled when `bShowDebugSpline` is true
- Mesh generation can be slow with many segments - disable for runtime

## Risk Zone Behavior

When `bIsRiskZone` is enabled:

1. `RoadColor` automatically changes to red
2. Vehicles can use this flag to trigger alerts
3. Visual indication in editor for level design

## Related Classes

- [`USplineMovementComponent`](SplineMovementComponent.md) - Movement along roads
- [`ARoadIntersection`](RoadIntersection.md) - Intersection management
- [`ATestVehicle`](TestVehicle.md) - Example vehicle

---

*Copyright 2025 AI27. All Rights Reserved.*
*Designer: Aldo Maradon Duran Bautista*
