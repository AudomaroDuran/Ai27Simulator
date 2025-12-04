# Map System Plugin

Plugin de sistema de mapa interactivo estilo Google Maps para Unreal Engine 5.5.

## Caracteristicas

- **Vista aerea del nivel**: Usa SceneCapture2D para renderizar una vista top-down
- **Zoom**: Zoom in/out con la rueda del mouse, centrado en la posicion del cursor
- **Paneo**: Arrastrar con el boton derecho para mover el mapa
- **Marcadores**: Sistema de marcadores para origen/destino con drag & drop
- **Validacion de posiciones**: Los marcadores hacen snap a posiciones validas usando traces
- **Conversion de coordenadas**: Funciones para convertir entre posiciones del mundo y UV del mapa

## Configuracion Rapida

### Paso 1: Crear el Widget Blueprint

1. Crear un nuevo Widget Blueprint que herede de `UMapWidget`
2. En el diseñador del widget, agregar:
   - Un `Image` widget para mostrar el mapa
   - Un `CanvasPanel` para los marcadores (overlay sobre la imagen)
   - Widgets adicionales para mostrar los marcadores visualmente

### Paso 2: Configurar el Widget

En el Event Graph del widget:

```
Event Construct:
    -> Set Map Image (tu Image widget)
    -> Set Marker Canvas (tu Canvas Panel)
```

### Paso 3: Colocar el MapSystemActor

1. Buscar `MapSystemActor` en el panel de actores
2. Arrastrarlo al nivel
3. En los detalles del actor:
   - Asignar tu Widget Blueprint a `Map Widget Class`
   - Activar `Auto Create Widget` si quieres que se cree automaticamente
   - Configurar la resolucion del mapa, altura de captura, etc.

### Paso 4: Usar desde Blueprint

#### Agregar marcadores:
```
Get MapSystemActor Reference
    -> Add Origin Marker (World Position)
    -> Add Destination Marker (World Position)
```

#### Escuchar eventos:
```
Bind Event to On Marker Moved
    -> [Tu logica cuando mueven un marcador]

Bind Event to On Map Clicked
    -> [Tu logica cuando hacen click en el mapa]
```

## Estructura de Clases

### UMapCaptureComponent
Componente que maneja el SceneCapture2D. Configuraciones principales:
- `MapResolution`: Resolucion del render target (default: 1024)
- `InitialCaptureHeight`: Altura de la camara sobre el nivel
- `BaseOrthoWidth`: Ancho ortografico base (area visible a zoom 1.0)
- `MinZoom` / `MaxZoom`: Limites de zoom
- `TraceChannel`: Canal de colision para validar posiciones

### UMapWidget
Widget base que maneja la interaccion del usuario:
- Zoom con rueda del mouse
- Paneo con click derecho + arrastrar
- Marcadores con click izquierdo + arrastrar
- Eventos para notificar cambios

### AMapSystemActor
Actor principal que une todo el sistema:
- Contiene el `MapCaptureComponent`
- Puede crear el widget automaticamente
- Proporciona funciones convenientes para Blueprint

### FMapMarkerData
Estructura de datos para los marcadores:
- `MarkerId`: Identificador unico
- `MarkerType`: Origin, Destination, o Custom
- `WorldPosition`: Posicion en el mundo
- `bIsValidPosition`: Si la posicion es valida (paso el trace)
- `bIsDraggable`: Si el usuario puede moverlo
- `Color`, `Label`, `IconSize`: Propiedades visuales

## Funciones de Blueprint Library

```cpp
// Crear marcadores
UMapBlueprintLibrary::MakeOriginMarker(MarkerId, WorldPosition)
UMapBlueprintLibrary::MakeDestinationMarker(MarkerId, WorldPosition)
UMapBlueprintLibrary::MakeCustomMarker(MarkerId, WorldPosition, Color, Label)

// Utilidades
UMapBlueprintLibrary::GetDistance2D(PositionA, PositionB)
UMapBlueprintLibrary::GetDirection2D(Origin, Destination)
UMapBlueprintLibrary::TraceForValidPosition(WorldPosition, ...)
UMapBlueprintLibrary::FindNearestValidPosition(WorldPosition, SearchRadius, ...)
```

## Ejemplo de Widget Blueprint

### Jerarquia recomendada:
```
[Canvas Panel] - Root
    [Image] - MapImage (Fill, mostrar el render target)
    [Canvas Panel] - MarkerCanvas (para posicionar marcadores)
        [Image/Widget] - Origin Marker (actualizar posicion cada tick)
        [Image/Widget] - Destination Marker
    [Overlay]
        [Text Block] - Zoom level indicator
        [Buttons] - Zoom in/out buttons
```

### Actualizar posiciones de marcadores visuales:
```
Event Tick:
    For Each (Get All Markers):
        World To Local (Marker.WorldPosition) -> Set Position of visual widget
```

## Notas de Rendimiento

- El SceneCapture2D tiene un costo de rendimiento. Considera:
  - Reducir `MapResolution` si no necesitas alta calidad
  - Desactivar `bCaptureEveryFrame` y usar `UpdateCapture()` manualmente
  - Filtrar actores que no necesitan aparecer en el mapa con `HiddenActors`

## Validacion de Posiciones

El sistema usa Line Traces para validar que los marcadores esten en posiciones validas:
- El trace va desde `InitialCaptureHeight` hacia abajo
- Usa `TraceChannel` configurable (default: Visibility)
- `bSnapToValidPositions` en la configuracion del widget controla si los marcadores hacen snap automatico
