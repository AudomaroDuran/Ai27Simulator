// Copyright Ai27. All Rights Reserved.

#include "MapSystemActor.h"
#include "MapWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AMapSystemActor::AMapSystemActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Create the map capture component
	MapCaptureComponent = CreateDefaultSubobject<UMapCaptureComponent>(TEXT("MapCaptureComponent"));
	RootComponent = MapCaptureComponent;
}

void AMapSystemActor::BeginPlay()
{
	Super::BeginPlay();

	InitializeMapSystem();

	if (bAutoCreateWidget)
	{
		CreateMapWidget();
	}
}

void AMapSystemActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MapWidget)
	{
		MapWidget->RemoveFromParent();
		MapWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AMapSystemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMapSystemActor::InitializeMapSystem()
{
	if (bIsInitialized)
	{
		return;
	}

	if (MapCaptureComponent)
	{
		MapCaptureComponent->InitializeMapCapture();
	}

	bIsInitialized = true;
	SetActorTickEnabled(true);
}

UMapWidget* AMapSystemActor::CreateMapWidget()
{
	if (!MapWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MapSystemActor: MapWidgetClass is not set. Cannot create widget."));
		return nullptr;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return nullptr;
	}

	MapWidget = CreateWidget<UMapWidget>(PC, MapWidgetClass);
	if (MapWidget)
	{
		MapWidget->InitializeMap(MapCaptureComponent);
		MapWidget->AddToViewport(WidgetZOrder);
	}

	return MapWidget;
}

void AMapSystemActor::SetMapWidget(UMapWidget* InWidget)
{
	MapWidget = InWidget;

	if (MapWidget && MapCaptureComponent)
	{
		MapWidget->InitializeMap(MapCaptureComponent);
	}
}

UTextureRenderTarget2D* AMapSystemActor::GetMapTexture() const
{
	return MapCaptureComponent ? MapCaptureComponent->GetMapTexture() : nullptr;
}

void AMapSystemActor::SetMapCenter(FVector2D WorldCenter)
{
	if (MapCaptureComponent)
	{
		MapCaptureComponent->SetMapCenter(WorldCenter);
	}
}

void AMapSystemActor::SetZoom(float NewZoom)
{
	if (MapCaptureComponent)
	{
		MapCaptureComponent->SetZoom(NewZoom);
	}
}

float AMapSystemActor::GetZoom() const
{
	return MapCaptureComponent ? MapCaptureComponent->CurrentZoom : 1.0f;
}

void AMapSystemActor::PanMap(FVector2D Delta)
{
	if (MapCaptureComponent)
	{
		MapCaptureComponent->PanMap(Delta);
	}
}

FName AMapSystemActor::AddOriginMarker(FVector WorldPosition)
{
	if (MapWidget)
	{
		return MapWidget->CreateOriginMarker(WorldPosition);
	}
	return NAME_None;
}

FName AMapSystemActor::AddDestinationMarker(FVector WorldPosition)
{
	if (MapWidget)
	{
		return MapWidget->CreateDestinationMarker(WorldPosition);
	}
	return NAME_None;
}

bool AMapSystemActor::AddMarker(const FMapMarkerData& MarkerData)
{
	if (MapWidget)
	{
		return MapWidget->AddMarker(MarkerData);
	}
	return false;
}

bool AMapSystemActor::RemoveMarker(FName MarkerId)
{
	if (MapWidget)
	{
		return MapWidget->RemoveMarker(MarkerId);
	}
	return false;
}

bool AMapSystemActor::GetMarkerData(FName MarkerId, FMapMarkerData& OutData) const
{
	if (MapWidget)
	{
		return MapWidget->GetMarker(MarkerId, OutData);
	}
	return false;
}

bool AMapSystemActor::SetMarkerPosition(FName MarkerId, FVector NewPosition)
{
	if (MapWidget)
	{
		return MapWidget->SetMarkerWorldPosition(MarkerId, NewPosition, true);
	}
	return false;
}

TArray<FMapMarkerData> AMapSystemActor::GetAllMarkers() const
{
	if (MapWidget)
	{
		return MapWidget->GetAllMarkers();
	}
	return TArray<FMapMarkerData>();
}

void AMapSystemActor::ClearAllMarkers()
{
	if (MapWidget)
	{
		MapWidget->ClearAllMarkers();
	}
}

FVector2D AMapSystemActor::WorldToMapUV(FVector WorldPosition) const
{
	if (MapCaptureComponent)
	{
		return MapCaptureComponent->WorldToMapUV(WorldPosition);
	}
	return FVector2D(0.5f, 0.5f);
}

FVector AMapSystemActor::MapUVToWorld(FVector2D UV) const
{
	if (MapCaptureComponent)
	{
		return MapCaptureComponent->MapUVToWorld(UV);
	}
	return FVector::ZeroVector;
}

bool AMapSystemActor::IsValidPosition(FVector WorldPosition, FVector& OutValidPosition) const
{
	if (MapCaptureComponent)
	{
		return MapCaptureComponent->ValidateWorldPosition(WorldPosition, OutValidPosition);
	}
	return false;
}

bool AMapSystemActor::FindSnapPosition(FVector2D MapUV, FVector& OutWorldPosition) const
{
	if (MapCaptureComponent)
	{
		return MapCaptureComponent->FindValidSnapPosition(MapUV, OutWorldPosition);
	}
	return false;
}
