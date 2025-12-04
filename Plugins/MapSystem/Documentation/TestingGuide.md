# MapSystem Plugin - Guia de Pruebas

## Indice

1. [Configuracion Inicial](#configuracion-inicial)
2. [Prueba Basica del Sistema](#prueba-basica-del-sistema)
3. [Pruebas de Funcionalidad](#pruebas-de-funcionalidad)
4. [Pruebas en Blueprint](#pruebas-en-blueprint)
5. [Pruebas de Rendimiento](#pruebas-de-rendimiento)
6. [Casos de Prueba Detallados](#casos-de-prueba-detallados)
7. [Depuracion](#depuracion)

---

## Configuracion Inicial

### Paso 1: Compilar el Plugin

1. Abrir el proyecto en Visual Studio o Rider
2. Compilar en modo `Development Editor`
3. Verificar que no hay errores de compilacion

```
Build > Build Solution (Ctrl+Shift+B)
```

### Paso 2: Verificar Carga del Plugin

1. Abrir Unreal Editor
2. Ir a `Edit > Plugins`
3. Buscar "Map System"
4. Verificar que esta habilitado

### Paso 3: Crear Nivel de Prueba

1. Crear un nuevo nivel: `File > New Level > Default`
2. Agregar geometria basica:
   - Plano grande como suelo (escala 100x100)
   - Algunos cubos y objetos 3D
3. Guardar como `L_MapSystemTest`

---

## Prueba Basica del Sistema

### Test 1: Colocar MapSystemActor

**Objetivo:** Verificar que el actor se puede colocar y funciona basicamente.

**Pasos:**
1. En el Content Browser, buscar `MapSystemActor`
2. Arrastrarlo al nivel
3. Posicionarlo en `(0, 0, 0)`
4. En el panel de detalles verificar:
   - `Map Capture Component` existe
   - `Map Resolution` = 1024
   - `Initial Capture Height` = 5000

**Resultado esperado:** El actor aparece en el nivel sin errores.

---

### Test 2: Verificar Render Target

**Objetivo:** Confirmar que el SceneCapture genera una imagen.

**Pasos:**
1. Seleccionar el `MapSystemActor` en el nivel
2. En Details, expandir `Map Capture Component`
3. Hacer Play in Editor (PIE)
4. En el Output Log, no debe haber errores del plugin
5. Pausar el juego
6. Buscar en Content Browser: `MapRenderTarget` (temporal)

**Resultado esperado:** Se genera un render target con la vista aerea del nivel.

---

### Test 3: Crear Widget de Prueba

**Objetivo:** Crear un widget minimo para visualizar el mapa.

**Pasos:**

1. Crear Widget Blueprint:
   - Click derecho en Content Browser
   - `User Interface > Widget Blueprint`
   - Nombrar: `WBP_TestMap`
   - Parent Class: `MapWidget`

2. Disenar el Widget:
   ```
   [Canvas Panel]
       [Image] - Name: "MapImage"
                 Anchors: Fill
                 Size: Fill parent
       [Canvas Panel] - Name: "MarkerCanvas"
                        Anchors: Fill
   ```

3. En el Event Graph:
   ```
   Event Construct
       │
       ├─► Get "MapImage" reference
       │       │
       │       └─► Set Map Image
       │
       └─► Get "MarkerCanvas" reference
               │
               └─► Set Marker Canvas
   ```

4. Configurar MapSystemActor:
   - `Map Widget Class` = `WBP_TestMap`
   - `Auto Create Widget` = true

5. Play in Editor

**Resultado esperado:** Aparece el mapa en pantalla mostrando la vista aerea.

---

## Pruebas de Funcionalidad

### Test 4: Zoom con Rueda del Mouse

**Objetivo:** Verificar funcionamiento del zoom.

**Pasos:**
1. Play in Editor con el widget visible
2. Posicionar el mouse sobre el mapa
3. Girar la rueda hacia arriba (zoom in)
4. Girar la rueda hacia abajo (zoom out)

**Verificaciones:**
- [ ] El zoom se centra en la posicion del cursor
- [ ] El zoom respeta `MinZoom` (0.1) y `MaxZoom` (10.0)
- [ ] La imagen no se distorsiona

**Resultado esperado:** Zoom suave centrado en el cursor.

---

### Test 5: Paneo del Mapa

**Objetivo:** Verificar funcionamiento del paneo.

**Pasos:**
1. Play in Editor
2. Click derecho sobre el mapa y mantener
3. Arrastrar en diferentes direcciones
4. Soltar el boton

**Verificaciones:**
- [ ] El mapa se mueve en la direccion del arrastre
- [ ] El movimiento es proporcional al arrastre
- [ ] Se puede combinar con zoom

**Resultado esperado:** Paneo fluido sin saltos.

---

### Test 6: Crear Marcadores

**Objetivo:** Verificar creacion de marcadores de origen y destino.

**Pasos:**

1. Crear un Blueprint Actor de prueba:
   ```
   Event BeginPlay
       │
       ├─► Get All Actors of Class (MapSystemActor)
       │       │
       │       └─► Get (index 0) ─► MapSystemActor Reference
       │
       ├─► Add Origin Marker
       │       Location: (500, 500, 0)
       │       Return: OriginMarkerId
       │
       └─► Add Destination Marker
               Location: (2000, 2000, 0)
               Return: DestinationMarkerId
   ```

2. Colocar el actor en el nivel
3. Play in Editor

**Verificaciones:**
- [ ] Ambos marcadores aparecen en el mapa
- [ ] El marcador de origen es verde
- [ ] El marcador de destino es rojo
- [ ] Las posiciones corresponden a las coordenadas del mundo

---

### Test 7: Arrastrar Marcadores

**Objetivo:** Verificar drag & drop de marcadores.

**Pasos:**
1. Con marcadores visibles en el mapa
2. Click izquierdo sobre un marcador
3. Arrastrar a nueva posicion
4. Soltar

**Verificaciones:**
- [ ] El marcador sigue el cursor mientras se arrastra
- [ ] El marcador cambia de apariencia al arrastrar
- [ ] El evento `OnMarkerMoved` se dispara al soltar
- [ ] La nueva posicion se refleja en `WorldPosition`

---

### Test 8: Validacion de Posiciones (Snap)

**Objetivo:** Verificar que los marcadores hacen snap a posiciones validas.

**Pasos:**
1. En el nivel, crear areas sin colision (borrar el piso en algunas zonas)
2. Play in Editor
3. Arrastrar un marcador hacia una zona sin suelo
4. Observar comportamiento

**Verificaciones:**
- [ ] El marcador no se coloca en posiciones sin geometria
- [ ] `bIsValidPosition` se actualiza correctamente
- [ ] El marcador visual indica estado invalido (color diferente)

---

## Pruebas en Blueprint

### Test 9: Eventos del Widget

**Objetivo:** Verificar que los eventos Blueprint funcionan.

Crear el siguiente Blueprint de prueba:

```
// En WBP_TestMap Event Graph

Event On Marker Moved (MarkerId, NewWorldPosition)
    │
    └─► Print String: "Marker {MarkerId} moved to {NewWorldPosition}"

Event On Map Clicked (WorldPosition)
    │
    └─► Print String: "Map clicked at {WorldPosition}"

Event On Zoom Changed (OldZoom, NewZoom)
    │
    └─► Print String: "Zoom changed from {OldZoom} to {NewZoom}"
```

**Verificaciones:**
- [ ] Los mensajes aparecen en pantalla al interactuar
- [ ] Los valores son correctos

---

### Test 10: Funciones de Conversion

**Objetivo:** Verificar conversion de coordenadas.

```
// Blueprint de prueba

Get Player Pawn Location
    │
    └─► World To Map UV (MapCaptureComponent)
            │
            └─► Print String: "Player UV: {UV}"

// Verificar ida y vuelta
World Position: (1000, 1000, 0)
    │
    └─► World To Map UV
            │
            └─► Map UV To World
                    │
                    └─► Print String: "Roundtrip: {Result}"
                         (Debe ser cercano a 1000, 1000, Z)
```

---

## Pruebas de Rendimiento

### Test 11: FPS con SceneCapture

**Objetivo:** Medir impacto en rendimiento.

**Pasos:**
1. Abrir la consola (`~`)
2. Ejecutar: `stat fps`
3. Anotar FPS base sin mapa
4. Mostrar el mapa
5. Anotar FPS con mapa visible
6. Hacer zoom in/out repetidamente
7. Anotar FPS durante interaccion

**Metricas a registrar:**

| Escenario | FPS Esperado |
|-----------|--------------|
| Sin mapa | Base del proyecto |
| Mapa visible (idle) | -5 a -15 FPS |
| Durante zoom/pan | Similar a idle |

---

### Test 12: Diferentes Resoluciones

**Objetivo:** Probar diferentes resoluciones del render target.

| Resolucion | Calidad | Rendimiento |
|------------|---------|-------------|
| 256 | Baja | Optimo |
| 512 | Media | Bueno |
| 1024 | Alta (default) | Aceptable |
| 2048 | Muy Alta | Pesado |

**Pasos:**
1. Cambiar `MapResolution` en el MapCaptureComponent
2. Play in Editor
3. Medir FPS y calidad visual

---

## Casos de Prueba Detallados

### CP-001: Inicializacion del Sistema

| Campo | Valor |
|-------|-------|
| **ID** | CP-001 |
| **Nombre** | Inicializacion del MapSystemActor |
| **Precondiciones** | Plugin compilado, nivel con geometria |
| **Pasos** | 1. Colocar MapSystemActor<br>2. Configurar widget<br>3. Play |
| **Resultado Esperado** | Mapa visible sin errores |
| **Prioridad** | Alta |

---

### CP-002: Zoom Limites

| Campo | Valor |
|-------|-------|
| **ID** | CP-002 |
| **Nombre** | Limites de Zoom |
| **Precondiciones** | Mapa visible |
| **Pasos** | 1. Zoom in hasta el maximo<br>2. Zoom out hasta el minimo |
| **Resultado Esperado** | Zoom se detiene en 0.1 y 10.0 |
| **Prioridad** | Media |

---

### CP-003: Marcador Fuera de Limites

| Campo | Valor |
|-------|-------|
| **ID** | CP-003 |
| **Nombre** | Marcador en posicion invalida |
| **Precondiciones** | Mapa con marcadores, zona sin suelo |
| **Pasos** | 1. Arrastrar marcador fuera del suelo |
| **Resultado Esperado** | Marcador indica estado invalido |
| **Prioridad** | Alta |

---

### CP-004: Multiples Marcadores

| Campo | Valor |
|-------|-------|
| **ID** | CP-004 |
| **Nombre** | Crear y gestionar multiples marcadores |
| **Precondiciones** | Mapa funcional |
| **Pasos** | 1. Crear 5+ marcadores<br>2. Mover varios<br>3. Eliminar algunos |
| **Resultado Esperado** | Todos los marcadores funcionan independientemente |
| **Prioridad** | Media |

---

### CP-005: Persistencia de Posiciones

| Campo | Valor |
|-------|-------|
| **ID** | CP-005 |
| **Nombre** | Posiciones se mantienen tras zoom/pan |
| **Precondiciones** | Marcadores colocados |
| **Pasos** | 1. Colocar marcador<br>2. Zoom in/out<br>3. Paneo<br>4. Verificar posicion |
| **Resultado Esperado** | Marcador mantiene su posicion mundial |
| **Prioridad** | Alta |

---

## Depuracion

### Comandos de Consola Utiles

```cpp
// Ver estadisticas de render
stat SceneRendering

// Ver memoria de texturas
stat Textures

// Debug del scene capture
ShowFlag.SceneCapture 1

// Log del plugin
Log LogMapSystem Verbose
```

### Visualizacion de Traces

Para ver los traces de validacion, agregar en el codigo:

```cpp
// En MapCaptureComponent.cpp, funcion ValidateWorldPosition

#if WITH_EDITOR
DrawDebugLine(
    World,
    TraceStart,
    TraceEnd,
    bHit ? FColor::Green : FColor::Red,
    false,
    2.0f
);
#endif
```

### Logs de Depuracion

Agregar en puntos clave:

```cpp
UE_LOG(LogTemp, Warning, TEXT("MapSystem: Zoom = %f, Center = %s"),
    CurrentZoom, *MapCenterWorld.ToString());

UE_LOG(LogTemp, Warning, TEXT("MapSystem: Marker %s moved to %s"),
    *MarkerId.ToString(), *NewPosition.ToString());
```

### Verificar Estado del Widget

En Blueprint, crear funcion de debug:

```
Custom Event: DebugPrintState
    │
    ├─► Print String: "Zoom: {GetZoom()}"
    │
    ├─► Print String: "Markers: {GetAllMarkers().Num}"
    │
    └─► For Each (GetAllMarkers())
            │
            └─► Print String: "{Marker.MarkerId}: {Marker.WorldPosition}"
```

---

## Checklist de Pruebas Completo

### Funcionalidad Core
- [ ] MapSystemActor se coloca sin errores
- [ ] SceneCapture genera imagen
- [ ] Widget muestra el mapa
- [ ] Zoom funciona (rueda del mouse)
- [ ] Paneo funciona (click derecho + arrastrar)

### Marcadores
- [ ] Crear marcador de origen
- [ ] Crear marcador de destino
- [ ] Crear marcador custom
- [ ] Arrastrar marcadores
- [ ] Validacion de posiciones
- [ ] Eventos se disparan correctamente

### Conversion de Coordenadas
- [ ] WorldToMapUV correcto
- [ ] MapUVToWorld correcto
- [ ] LocalToWorld correcto
- [ ] Roundtrip sin perdida significativa

### Rendimiento
- [ ] FPS aceptable con mapa visible
- [ ] Sin memory leaks despues de uso prolongado
- [ ] Limpieza correcta al destruir actor

### Edge Cases
- [ ] Nivel sin geometria (solo cielo)
- [ ] Nivel muy grande
- [ ] Muchos marcadores (50+)
- [ ] Zoom extremo (min y max)
- [ ] Multiples MapSystemActors

---

## Reporte de Bugs

Al reportar bugs, incluir:

1. **Descripcion:** Que paso vs que deberia pasar
2. **Pasos para reproducir:** Lista numerada
3. **Entorno:** Version UE, specs de PC
4. **Logs:** Copiar logs relevantes
5. **Screenshots/Video:** Si aplica

Ejemplo:
```
BUG: Marcador desaparece al hacer zoom maximo

Pasos:
1. Colocar marcador en (1000, 1000)
2. Hacer zoom in hasta 10.0
3. El marcador ya no es visible

Esperado: Marcador visible (posiblemente fuera del viewport)
Actual: Marcador no se renderiza

UE 5.5, Windows 11, RTX 3080
```
