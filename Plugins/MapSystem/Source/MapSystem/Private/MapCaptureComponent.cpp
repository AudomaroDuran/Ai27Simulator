// Copyright Ai27. All Rights Reserved.

#include "MapCaptureComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"

UMapCaptureComponent::UMapCaptureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMapCaptureComponent::BeginPlay()
{
	Super::BeginPlay();
	CachedOwner = GetOwner();
}

void UMapCaptureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SceneCaptureComponent)
	{
		SceneCaptureComponent->DestroyComponent();
		SceneCaptureComponent = nullptr;
	}

	if (MapRenderTarget)
	{
		MapRenderTarget->ConditionalBeginDestroy();
		MapRenderTarget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UMapCaptureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMapCaptureComponent::InitializeMapCapture()
{
	if (bIsInitialized)
	{
		return;
	}

	CreateRenderTarget();
	SetupSceneCapture();

	// Set initial position to owner's location
	if (CachedOwner)
	{
		FVector OwnerLoc = CachedOwner->GetActorLocation();
		MapCenterWorld = FVector2D(OwnerLoc.X, OwnerLoc.Y);
	}

	UpdateCaptureTransform();
	bIsInitialized = true;

	SetComponentTickEnabled(true);
}

void UMapCaptureComponent::CreateRenderTarget()
{
	MapRenderTarget = NewObject<UTextureRenderTarget2D>(this);
	MapRenderTarget->RenderTargetFormat = RTF_RGBA8;
	MapRenderTarget->InitAutoFormat(MapResolution, MapResolution);
	MapRenderTarget->UpdateResourceImmediate(true);
}

void UMapCaptureComponent::SetupSceneCapture()
{
	if (!CachedOwner)
	{
		return;
	}

	// Create the scene capture component
	SceneCaptureComponent = NewObject<USceneCaptureComponent2D>(CachedOwner);
	SceneCaptureComponent->SetupAttachment(this);
	SceneCaptureComponent->RegisterComponent();

	// Configure for top-down orthographic view
	SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	SceneCaptureComponent->OrthoWidth = BaseOrthoWidth;
	SceneCaptureComponent->TextureTarget = MapRenderTarget;
	SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCaptureComponent->bCaptureEveryFrame = true;
	SceneCaptureComponent->bCaptureOnMovement = true;
	SceneCaptureComponent->bAlwaysPersistRenderingState = true;

	// Set rotation to look straight down
	SceneCaptureComponent->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));

	// Configure capture settings for better quality
	SceneCaptureComponent->ShowFlags.SetAntiAliasing(true);
	SceneCaptureComponent->ShowFlags.SetAtmosphere(false);
	SceneCaptureComponent->ShowFlags.SetFog(false);
	SceneCaptureComponent->ShowFlags.SetVolumetricFog(false);
}

void UMapCaptureComponent::UpdateCaptureTransform()
{
	if (!SceneCaptureComponent)
	{
		return;
	}

	// Calculate position
	FVector CapturePosition(MapCenterWorld.X, MapCenterWorld.Y, InitialCaptureHeight);
	SceneCaptureComponent->SetWorldLocation(CapturePosition);

	// Update ortho width based on zoom
	float NewOrthoWidth = GetCurrentOrthoWidth();
	SceneCaptureComponent->OrthoWidth = NewOrthoWidth;

	// Broadcast the change
	OnMapBoundsChanged.Broadcast(MapCenterWorld, CurrentZoom);
}

void UMapCaptureComponent::SetMapCenter(FVector2D NewCenter)
{
	MapCenterWorld = NewCenter;
	UpdateCaptureTransform();
}

void UMapCaptureComponent::PanMap(FVector2D ScreenDelta)
{
	// Convert screen delta to world delta based on current zoom
	float CurrentOrthoWidth = GetCurrentOrthoWidth();

	// Screen delta is in normalized coordinates (-1 to 1 range typically)
	// Convert to world units
	FVector2D WorldDelta;
	WorldDelta.X = -ScreenDelta.X * CurrentOrthoWidth * PanSpeed;
	WorldDelta.Y = -ScreenDelta.Y * CurrentOrthoWidth * PanSpeed;

	MapCenterWorld += WorldDelta;
	UpdateCaptureTransform();
}

void UMapCaptureComponent::ZoomMap(float ZoomDelta, FVector2D ZoomAroundUV)
{
	// Calculate the world position at the zoom point before zooming
	FVector WorldPosBeforeZoom = MapUVToWorld(ZoomAroundUV);

	// Apply zoom
	float NewZoom = FMath::Clamp(CurrentZoom + ZoomDelta * ZoomSpeed, MinZoom, MaxZoom);

	if (FMath::IsNearlyEqual(NewZoom, CurrentZoom))
	{
		return;
	}

	CurrentZoom = NewZoom;

	// Calculate where the zoom point would be after zooming with the same center
	FVector WorldPosAfterZoom = MapUVToWorld(ZoomAroundUV);

	// Adjust center to keep the zoom point in the same place
	FVector2D Offset(
		WorldPosBeforeZoom.X - WorldPosAfterZoom.X,
		WorldPosBeforeZoom.Y - WorldPosAfterZoom.Y
	);
	MapCenterWorld += Offset;

	UpdateCaptureTransform();
}

void UMapCaptureComponent::SetZoom(float NewZoom)
{
	CurrentZoom = FMath::Clamp(NewZoom, MinZoom, MaxZoom);
	UpdateCaptureTransform();
}

FVector2D UMapCaptureComponent::WorldToMapUV(FVector WorldPosition) const
{
	float CurrentOrthoWidth = GetCurrentOrthoWidth();
	float HalfWidth = CurrentOrthoWidth * 0.5f;

	// Calculate UV based on position relative to map center
	FVector2D UV;
	UV.X = (WorldPosition.X - MapCenterWorld.X) / CurrentOrthoWidth + 0.5f;
	UV.Y = (WorldPosition.Y - MapCenterWorld.Y) / CurrentOrthoWidth + 0.5f;

	return UV;
}

FVector UMapCaptureComponent::MapUVToWorld(FVector2D UV) const
{
	float CurrentOrthoWidth = GetCurrentOrthoWidth();

	FVector WorldPos;
	WorldPos.X = MapCenterWorld.X + (UV.X - 0.5f) * CurrentOrthoWidth;
	WorldPos.Y = MapCenterWorld.Y + (UV.Y - 0.5f) * CurrentOrthoWidth;
	WorldPos.Z = 0.0f; // Will be determined by trace if needed

	return WorldPos;
}

bool UMapCaptureComponent::IsWorldPositionVisible(FVector WorldPosition) const
{
	FVector2D UV = WorldToMapUV(WorldPosition);
	return UV.X >= 0.0f && UV.X <= 1.0f && UV.Y >= 0.0f && UV.Y <= 1.0f;
}

void UMapCaptureComponent::GetVisibleWorldBounds(FVector2D& OutMin, FVector2D& OutMax) const
{
	float CurrentOrthoWidth = GetCurrentOrthoWidth();
	float HalfWidth = CurrentOrthoWidth * 0.5f;

	OutMin.X = MapCenterWorld.X - HalfWidth;
	OutMin.Y = MapCenterWorld.Y - HalfWidth;
	OutMax.X = MapCenterWorld.X + HalfWidth;
	OutMax.Y = MapCenterWorld.Y + HalfWidth;
}

bool UMapCaptureComponent::ValidateWorldPosition(FVector WorldPosition, FVector& OutValidPosition) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Start trace from high above the position
	FVector TraceStart = WorldPosition;
	TraceStart.Z = InitialCaptureHeight;

	FVector TraceEnd = WorldPosition;
	TraceEnd.Z = -MaxTraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedOwner);

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		TraceChannel,
		QueryParams
	);

	if (bHit)
	{
		OutValidPosition = HitResult.Location;
		return true;
	}

	return false;
}

bool UMapCaptureComponent::FindValidSnapPosition(FVector2D MapUV, FVector& OutWorldPosition) const
{
	FVector WorldPos = MapUVToWorld(MapUV);
	return ValidateWorldPosition(WorldPos, OutWorldPosition);
}

float UMapCaptureComponent::GetCurrentOrthoWidth() const
{
	// Higher zoom = smaller ortho width (more zoomed in)
	return BaseOrthoWidth / CurrentZoom;
}

void UMapCaptureComponent::UpdateCapture()
{
	if (SceneCaptureComponent)
	{
		SceneCaptureComponent->CaptureScene();
	}
}
