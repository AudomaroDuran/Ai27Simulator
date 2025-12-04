# MapSystem Plugin - Como Funciona

## Descripcion General

El plugin **MapSystem** proporciona un sistema de mapa interactivo estilo Google Maps para Unreal Engine 5.5. Permite visualizar una vista aerea del nivel con capacidades de zoom, paneo y marcadores arrastrables.

---

## Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────────┐
│                     AMapSystemActor                          │
│  (Actor principal que coordina todo el sistema)              │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌─────────────────────┐      ┌─────────────────────────┐   │
│  │ UMapCaptureComponent│      │     UMapWidget          │   │
│  │                     │      │                         │   │
│  │ - SceneCapture2D    │◄────►│ - Input handling        │   │
│  │ - RenderTarget      │      │ - Marker management     │   │
│  │ - Coordinate conv.  │      │ - Visual display        │   │
│  │ - Position valid.   │      │                         │   │
│  └─────────────────────┘      └─────────────────────────┘   │
│                                         │                    │
│                                         ▼                    │
│                               ┌─────────────────────────┐   │
│                               │   UMapMarkerWidget      │   │
│                               │   (Por cada marcador)   │   │
│                               └─────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

---

## Componentes Principales

### 1. UMapCaptureComponent

**Ubicacion:** `Source/MapSystem/Public/MapCaptureComponent.h`

Este componente es el corazon del sistema de captura. Maneja:

#### Captura de Escena
```cpp
// Crea un SceneCapture2D mirando hacia abajo (top-down)
SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
SceneCaptureComponent->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
```

#### Propiedades Configurables

| Propiedad | Tipo | Default | Descripcion |
|-----------|------|---------|-------------|
| `MapResolution` | int32 | 1024 | Resolucion del render target en pixeles |
| `InitialCaptureHeight` | float | 5000 | Altura de la camara sobre el nivel |
| `BaseOrthoWidth` | float | 10000 | Ancho ortografico a zoom 1.0 |
| `MinZoom` / `MaxZoom` | float | 0.1 / 10.0 | Limites de zoom |
| `TraceChannel` | ECollisionChannel | Visibility | Canal para validar posiciones |

#### Funciones de Conversion de Coordenadas

```cpp
// Mundo -> UV del mapa (0-1)
FVector2D UV = WorldToMapUV(WorldPosition);

// UV del mapa -> Mundo
FVector WorldPos = MapUVToWorld(UV);

// Obtener limites visibles actuales
FVector2D Min, Max;
GetVisibleWorldBounds(Min, Max);
```

#### Validacion de Posiciones

El componente usa Line Traces para validar que una posicion del mundo sea valida:

```cpp
bool ValidateWorldPosition(FVector WorldPosition, FVector& OutValidPosition) const
{
    // Trace desde InitialCaptureHeight hacia abajo
    // Retorna true si golpea geometria valida
}
```

---

### 2. UMapWidget

**Ubicacion:** `Source/MapSystem/Public/MapWidget.h`

Widget UMG que maneja toda la interaccion del usuario.

#### Manejo de Input

| Accion | Input Default | Funcion |
|--------|---------------|---------|
| Paneo | Click derecho + arrastrar | `HandlePanning()` |
| Zoom | Rueda del mouse | `HandleZoom()` |
| Mover marcador | Click izquierdo + arrastrar | `HandleMarkerDrag()` |
| Click en mapa | Click izquierdo (sin marcador) | Dispara `OnMapClicked` |

#### Sistema de Eventos

```cpp
// Cuando un marcador se mueve
UPROPERTY(BlueprintAssignable)
FOnMarkerMoved OnMarkerMoved;

// Cuando se hace click en el mapa
UPROPERTY(BlueprintAssignable)
FOnMapClicked OnMapClicked;

// Cuando cambia el zoom
UPROPERTY(BlueprintAssignable)
FOnZoomChanged OnZoomChanged;

// Cuando cambia el estado de un marcador
UPROPERTY(BlueprintAssignable)
FOnMarkerStateChanged OnMarkerStateChanged;
```

#### Gestion de Marcadores

Los marcadores se almacenan en un `TMap<FName, FMapMarkerData>`:

```cpp
// Agregar marcadores
FName CreateOriginMarker(FVector WorldPosition);
FName CreateDestinationMarker(FVector WorldPosition);
bool AddMarker(const FMapMarkerData& MarkerData);

// Consultar marcadores
bool GetMarker(FName MarkerId, FMapMarkerData& OutData);
TArray<FMapMarkerData> GetAllMarkers();

// Modificar marcadores
bool SetMarkerWorldPosition(FName MarkerId, FVector NewPosition, bool bValidate);
bool RemoveMarker(FName MarkerId);
```

---

### 3. FMapMarkerData (Estructura de Datos)

**Ubicacion:** `Source/MapSystem/Public/MapTypes.h`

```cpp
USTRUCT(BlueprintType)
struct FMapMarkerData
{
    FName MarkerId;              // ID unico
    EMapMarkerType MarkerType;   // Origin, Destination, Custom
    EMapMarkerState MarkerState; // Idle, Hovered, Dragging, Invalid
    FVector WorldPosition;       // Posicion en el mundo
    bool bIsValidPosition;       // Si paso la validacion de trace
    bool bIsDraggable;           // Si el usuario puede moverlo
    bool bIsVisible;             // Si esta visible
    FText Label;                 // Etiqueta
    FLinearColor Color;          // Color del marcador
    float IconSize;              // Tamano en pixeles
};
```

---

### 4. AMapSystemActor

**Ubicacion:** `Source/MapSystem/Public/MapSystemActor.h`

Actor que une todos los componentes y proporciona una interfaz simple para Blueprint.

#### Inicializacion

```cpp
void BeginPlay()
{
    InitializeMapSystem();  // Inicializa el MapCaptureComponent

    if (bAutoCreateWidget)
    {
        CreateMapWidget();  // Crea y muestra el widget automaticamente
    }
}
```

#### Funciones de Conveniencia

```cpp
// Control del mapa
void SetMapCenter(FVector2D WorldCenter);
void SetZoom(float NewZoom);
void PanMap(FVector2D Delta);

// Marcadores
FName AddOriginMarker(FVector WorldPosition);
FName AddDestinationMarker(FVector WorldPosition);
bool SetMarkerPosition(FName MarkerId, FVector NewPosition);

// Conversion
FVector2D WorldToMapUV(FVector WorldPosition);
FVector MapUVToWorld(FVector2D UV);
```

---

## Flujo de Datos

### 1. Inicializacion

```
BeginPlay()
    │
    ▼
InitializeMapSystem()
    │
    ├── CreateRenderTarget()     // Crea UTextureRenderTarget2D
    │
    ├── SetupSceneCapture()      // Configura SceneCaptureComponent2D
    │
    └── UpdateCaptureTransform() // Posiciona la camara
```

### 2. Interaccion de Usuario (Zoom)

```
Usuario gira rueda del mouse
    │
    ▼
NativeOnMouseWheel()
    │
    ▼
HandleZoom(WheelDelta, LocalPosition)
    │
    ├── LocalToMapUV() ──────────┐
    │                            ▼
    │                   MapCaptureComponent->ZoomMap()
    │                            │
    │                   ┌────────┴────────┐
    │                   │ Calcula nueva   │
    │                   │ posicion para   │
    │                   │ mantener punto  │
    │                   │ bajo cursor     │
    │                   └────────┬────────┘
    │                            │
    └── OnZoomChanged.Broadcast()
```

### 3. Arrastrar Marcador

```
Click izquierdo en marcador
    │
    ▼
NativeOnMouseButtonDown()
    │
    ├── FindMarkerAtPosition()  // Encuentra marcador bajo cursor
    │
    └── SetMarkerState(Dragging)
            │
            ▼
    CaptureMouse()
            │
            ▼
NativeOnMouseMove() [repetido]
    │
    ▼
HandleMarkerDrag()
    │
    ├── LocalToMapUV()
    │
    ├── MapUVToWorld()
    │
    └── FindValidSnapPosition() ◄── Line Trace para validar
            │
            ├── Posicion valida: Actualiza WorldPosition
            │
            └── Posicion invalida: Marca bIsValidPosition = false
                    │
                    ▼
NativeOnMouseButtonUp()
    │
    ├── OnMarkerMoved.Broadcast()
    │
    └── SetMarkerState(Idle o Invalid)
```

---

## Sistema de Coordenadas

### Espacios de Coordenadas

1. **World Space**: Coordenadas 3D del nivel de Unreal (X, Y, Z)
2. **Map UV**: Coordenadas normalizadas 0-1 en el render target
3. **Local Widget**: Coordenadas en pixeles dentro del widget

### Conversiones

```
World Position (FVector)
        │
        │ WorldToMapUV()
        ▼
   Map UV (FVector2D 0-1)
        │
        │ UV * LocalSize
        ▼
Local Position (FVector2D pixels)
```

### Formula de Conversion World -> UV

```cpp
FVector2D UMapCaptureComponent::WorldToMapUV(FVector WorldPosition) const
{
    float CurrentOrthoWidth = BaseOrthoWidth / CurrentZoom;

    FVector2D UV;
    UV.X = (WorldPosition.X - MapCenterWorld.X) / CurrentOrthoWidth + 0.5f;
    UV.Y = (WorldPosition.Y - MapCenterWorld.Y) / CurrentOrthoWidth + 0.5f;

    return UV;
}
```

---

## Validacion de Posiciones con Traces

El sistema usa Line Traces para asegurar que los marcadores se coloquen en geometria valida:

```cpp
bool ValidateWorldPosition(FVector WorldPosition, FVector& OutValidPosition) const
{
    FVector TraceStart = WorldPosition;
    TraceStart.Z = InitialCaptureHeight;  // Desde arriba

    FVector TraceEnd = WorldPosition;
    TraceEnd.Z = -MaxTraceDistance;       // Hacia abajo

    FHitResult HitResult;
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        TraceChannel  // Default: ECC_Visibility
    );

    if (bHit)
    {
        OutValidPosition = HitResult.Location;
        return true;
    }
    return false;
}
```

---

## Configuracion del SceneCapture

El SceneCapture2D se configura para una vista top-down optima:

```cpp
void SetupSceneCapture()
{
    SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
    SceneCaptureComponent->OrthoWidth = BaseOrthoWidth;
    SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    SceneCaptureComponent->bCaptureEveryFrame = true;

    // Mirando hacia abajo
    SceneCaptureComponent->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));

    // Optimizaciones visuales
    SceneCaptureComponent->ShowFlags.SetAtmosphere(false);
    SceneCaptureComponent->ShowFlags.SetFog(false);
}
```

---

## Diagrama de Clases UML

```
┌──────────────────────────┐
│     AMapSystemActor      │
├──────────────────────────┤
│ + MapCaptureComponent    │
│ + MapWidget              │
│ + MapWidgetClass         │
│ + bAutoCreateWidget      │
├──────────────────────────┤
│ + InitializeMapSystem()  │
│ + CreateMapWidget()      │
│ + AddOriginMarker()      │
│ + AddDestinationMarker() │
│ + SetZoom()              │
│ + PanMap()               │
└──────────┬───────────────┘
           │ contains
           ▼
┌──────────────────────────┐     uses      ┌────────────────────────┐
│  UMapCaptureComponent    │◄──────────────│      UMapWidget        │
├──────────────────────────┤               ├────────────────────────┤
│ + SceneCaptureComponent  │               │ + MapConfig            │
│ + MapRenderTarget        │               │ + Markers: TMap        │
│ + MapCenterWorld         │               │ + CurrentInputMode     │
│ + CurrentZoom            │               ├────────────────────────┤
├──────────────────────────┤               │ + Initialize()         │
│ + InitializeMapCapture() │               │ + AddMarker()          │
│ + SetMapCenter()         │               │ + CreateOriginMarker() │
│ + ZoomMap()              │               │ + LocalToWorld()       │
│ + PanMap()               │               │ + WorldToLocal()       │
│ + WorldToMapUV()         │               │ + OnMarkerMoved        │
│ + MapUVToWorld()         │               │ + OnMapClicked         │
│ + ValidateWorldPosition()│               └────────────────────────┘
└──────────────────────────┘                          │
                                                      │ displays
                                                      ▼
                                          ┌────────────────────────┐
                                          │   UMapMarkerWidget     │
                                          ├────────────────────────┤
                                          │ + MarkerData           │
                                          │ + IdleColor            │
                                          │ + HoveredColor         │
                                          ├────────────────────────┤
                                          │ + InitializeMarker()   │
                                          │ + SetMarkerState()     │
                                          │ + OnClicked            │
                                          │ + OnDragStarted        │
                                          └────────────────────────┘
```

---

## Consideraciones de Rendimiento

### SceneCapture2D
- Renderiza la escena completa cada frame si `bCaptureEveryFrame = true`
- Para mejor rendimiento, desactivar y llamar `UpdateCapture()` manualmente

### Optimizaciones Recomendadas
1. Reducir `MapResolution` si la calidad no es critica
2. Desactivar efectos innecesarios en ShowFlags
3. Usar `HiddenActors` para excluir actores del capture
4. Considerar capturar solo cuando el mapa esta visible

```cpp
// Captura manual cuando sea necesario
MapCaptureComponent->bCaptureEveryFrame = false;
// ... cuando el mapa se abre ...
MapCaptureComponent->UpdateCapture();
```
