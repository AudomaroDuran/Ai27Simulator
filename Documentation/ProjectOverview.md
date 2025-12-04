# AI27 Simulator - Project Overview

## Introduction

AI27 Simulator is an Unreal Engine 5.5 C++ project designed for motor vehicle simulation. The project provides a flexible, physics-free road network system with smooth vehicle movement along spline-based roads.

**Designer:** Aldo Maradon Duran Bautista
**Copyright:** 2025 AI27. All Rights Reserved.

## Key Features

- **Spline-Based Road Network**: Define roads using editable splines
- **Physics-Free Movement**: Efficient position interpolation for mobile optimization
- **Smart Intersections**: Automatic smooth transition curves between roads
- **Modular Architecture**: Component-based design for flexibility
- **Blueprint Ready**: Full Blueprint support for all classes
- **Mobile Optimized**: Lightweight implementation suitable for mobile platforms

## Project Structure

```
ai27Simulator/
├── Source/
│   ├── ai27Simulator.Target.cs        # Game build target
│   ├── ai27SimulatorEditor.Target.cs  # Editor build target
│   └── ai27Simulator/
│       ├── Public/
│       │   ├── Components/
│       │   │   └── SplineMovementComponent.h
│       │   ├── RoadSystem/
│       │   │   ├── RoadSplineActor.h
│       │   │   └── RoadIntersection.h
│       │   └── Vehicles/
│       │       └── TestVehicle.h
│       ├── Private/
│       │   ├── Components/
│       │   │   └── SplineMovementComponent.cpp
│       │   ├── RoadSystem/
│       │   │   ├── RoadSplineActor.cpp
│       │   │   └── RoadIntersection.cpp
│       │   └── Vehicles/
│       │       └── TestVehicle.cpp
│       ├── ai27Simulator.h
│       ├── ai27Simulator.cpp
│       └── ai27Simulator.Build.cs
└── Documentation/
    ├── ProjectOverview.md             # This file
    ├── SplineMovementComponent.md
    ├── RoadSplineActor.md
    ├── RoadIntersection.md
    ├── TestVehicle.md
    └── BuildConfiguration.md
```

## Architecture Overview

### System Components

```
┌─────────────────────────────────────────────────────────────┐
│                        TestVehicle                          │
│  (Pawn with visual mesh and configuration)                  │
└─────────────────────┬───────────────────────────────────────┘
                      │ uses
                      ▼
┌─────────────────────────────────────────────────────────────┐
│                 SplineMovementComponent                      │
│  (Handles movement along splines)                           │
└─────────────────────┬───────────────────────────────────────┘
                      │ follows
                      ▼
┌─────────────────────────────────────────────────────────────┐
│                    RoadSplineActor                          │
│  (Defines road segments with spline curves)                 │
└─────────────────────┬───────────────────────────────────────┘
                      │ connects to
                      ▼
┌─────────────────────────────────────────────────────────────┐
│                   RoadIntersection                          │
│  (Manages road connections and generates turn curves)       │
└─────────────────────────────────────────────────────────────┘
```

### Data Flow

1. **Road Definition**: Designer places `RoadSplineActor` instances and edits splines
2. **Intersection Setup**: `RoadIntersection` actors connect multiple roads
3. **Vehicle Placement**: `TestVehicle` is assigned to a starting road
4. **Movement Execution**: `SplineMovementComponent` moves vehicle along splines
5. **Road Transitions**: At road end, vehicle uses intersection curves or direct connection

## Core Classes

### SplineMovementComponent

**Role:** Movement logic for actors along splines

**Key Responsibilities:**
- Position and rotation interpolation along splines
- Speed control with acceleration/deceleration
- Smooth transitions between roads
- Event broadcasting (end of road, speed changes)

[Full Documentation](SplineMovementComponent.md)

### RoadSplineActor

**Role:** Road segment definition

**Key Responsibilities:**
- Spline-based road path definition
- Road properties (width, lanes, speed limit)
- Optional mesh generation
- Connection tracking with other roads

[Full Documentation](RoadSplineActor.md)

### RoadIntersection

**Role:** Multi-road junction management

**Key Responsibilities:**
- Handle 2+ road connections
- Generate smooth transition curves
- Route selection logic
- Debug visualization

[Full Documentation](RoadIntersection.md)

### TestVehicle

**Role:** Example vehicle implementation

**Key Responsibilities:**
- Demonstrate SplineMovementComponent usage
- Automatic road following
- Intersection navigation
- Multiple transition modes

[Full Documentation](TestVehicle.md)

## Usage Guide

### Creating a Road Network

1. **Place Roads**
   - Drag `RoadSplineActor` into the level
   - Edit spline points in viewport to define road shape
   - Set properties: name, speed limit, lanes

2. **Create Intersections**
   - Place `RoadIntersection` at road meeting points
   - Add connections in the Connections array
   - Set connection types (Incoming/Outgoing/Bidirectional)

3. **Place Vehicles**
   - Add `TestVehicle` to level
   - Assign `StartingRoad`
   - Configure speed and transition mode
   - Press Play

### Quick Start Example

```cpp
// 1. Create a simple road in BeginPlay
ARoadSplineActor* Road = GetWorld()->SpawnActor<ARoadSplineActor>(
    ARoadSplineActor::StaticClass(),
    FVector(0, 0, 0),
    FRotator::ZeroRotator
);
Road->RoadName = TEXT("Main Street");
Road->SpeedLimit = 60.0f;

// 2. Spawn a vehicle
ATestVehicle* Vehicle = GetWorld()->SpawnActor<ATestVehicle>(
    ATestVehicle::StaticClass(),
    FVector(0, 0, 50),
    FRotator::ZeroRotator
);
Vehicle->VehicleName = TEXT("Car 1");
Vehicle->InitialSpeedKmH = 80.0f;

// 3. Start the vehicle
Vehicle->AssignToRoad(Road);
```

## Speed Reference

| km/h | cm/s | Use Case |
|------|------|----------|
| 30 | 833 | City slow zones |
| 50 | 1389 | Urban streets |
| 60 | 1667 | Default city speed |
| 80 | 2222 | Main roads |
| 100 | 2778 | Highways |
| 120 | 3333 | Fast highways |

## Performance Considerations

### Optimizations

1. **No Physics**: Pure interpolation without physics overhead
2. **Minimal Tick**: Only essential calculations per frame
3. **Lazy Mesh Generation**: Road meshes generated on demand
4. **Debug Only in PIE**: Visual debug only in Play-In-Editor

### Recommendations

- Use `bGenerateRoadMesh = false` for better runtime performance
- Keep intersection radius reasonable (300-800 cm)
- Limit debug visualization in production
- Consider LOD for vehicle meshes in large-scale simulations

## Extending the System

### Adding a New Vehicle Type

1. Create a subclass of `ATestVehicle` or `APawn`
2. Add `USplineMovementComponent`
3. Implement custom mesh and behavior
4. Override `OnReachedEndOfRoad()` for custom transitions

### Adding Traffic Signals

1. Extend `ARoadIntersection` to include signal state
2. Add timer logic for signal changes
3. Modify `TestVehicle` to check signal before transitioning
4. Implement stop/go behavior based on signal state

### Multi-Lane Support

1. Create lane offset calculations in `RoadSplineActor`
2. Track current lane in vehicle
3. Implement lane change logic
4. Add lane-aware intersection transitions

## Dependencies

- **Unreal Engine 5.5**
- **Core Modules:**
  - Core
  - CoreUObject
  - Engine
  - InputCore
  - EnhancedInput

## File Statistics

| Category | Files | Lines of Code |
|----------|-------|---------------|
| Headers (.h) | 5 | ~920 |
| Implementation (.cpp) | 5 | ~1,560 |
| Build Config (.cs) | 3 | ~55 |
| **Total** | **13** | **~2,535** |

## Documentation Files

| Document | Description |
|----------|-------------|
| [ProjectOverview.md](ProjectOverview.md) | This document - system overview |
| [SplineMovementComponent.md](SplineMovementComponent.md) | Movement component details |
| [RoadSplineActor.md](RoadSplineActor.md) | Road definition actor |
| [RoadIntersection.md](RoadIntersection.md) | Intersection management |
| [TestVehicle.md](TestVehicle.md) | Example vehicle pawn |
| [BuildConfiguration.md](BuildConfiguration.md) | Build system configuration |

## Future Enhancements

- Traffic light system
- Multi-lane roads with lane changing
- AI navigation and pathfinding
- Collision detection and avoidance
- Traffic density management
- Weather effects on vehicle behavior
- Sound system integration
- HUD/UI for vehicle information

---

*Copyright 2025 AI27. All Rights Reserved.*
*Designer: Aldo Maradon Duran Bautista*
*Project: AI27 Simulator*
