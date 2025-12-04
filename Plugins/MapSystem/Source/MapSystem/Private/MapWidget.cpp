// Copyright Ai27. All Rights Reserved.

#include "MapWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

UMapWidget::UMapWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UMapWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMapWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	CachedGeometry = MyGeometry;
	UpdateMarkerPositions();
}

FReply UMapWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FVector2D LocalPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	LastMousePosition = LocalPosition;

	// Check if clicking on a marker
	if (InMouseEvent.GetEffectingButton() == MapConfig.MarkerButton)
	{
		FName HitMarkerId = FindMarkerAtPosition(LocalPosition);

		if (!HitMarkerId.IsNone())
		{
			// Check if marker is draggable
			FMapMarkerData* MarkerData = Markers.Find(HitMarkerId);
			if (MarkerData && MarkerData->bIsDraggable && MapConfig.bAllowMarkerDragging)
			{
				CurrentInputMode = EMapInputMode::DraggingMarker;
				DraggingMarkerId = HitMarkerId;
				SetMarkerState(HitMarkerId, EMapMarkerState::Dragging);
				return FReply::Handled().CaptureMouse(TakeWidget());
			}
			else
			{
				// Clicked on non-draggable marker
				OnMarkerClicked.Broadcast(HitMarkerId, MarkerData->WorldPosition);
			}
		}
		else
		{
			// Clicked on empty map area
			FVector WorldPos = LocalToWorld(LocalPosition);
			OnMapClicked.Broadcast(WorldPos);
		}
	}

	// Check for pan button
	if (InMouseEvent.GetEffectingButton() == MapConfig.PanButton && MapConfig.bAllowPanning)
	{
		CurrentInputMode = EMapInputMode::Panning;
		return FReply::Handled().CaptureMouse(TakeWidget());
	}

	return FReply::Handled();
}

FReply UMapWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (CurrentInputMode == EMapInputMode::DraggingMarker)
	{
		// Finalize marker position
		FMapMarkerData* MarkerData = Markers.Find(DraggingMarkerId);
		if (MarkerData)
		{
			SetMarkerState(DraggingMarkerId, MarkerData->bIsValidPosition ? EMapMarkerState::Idle : EMapMarkerState::Invalid);
			OnMarkerMoved.Broadcast(DraggingMarkerId, MarkerData->WorldPosition);
		}
		DraggingMarkerId = NAME_None;
	}

	CurrentInputMode = EMapInputMode::None;
	return FReply::Handled().ReleaseMouseCapture();
}

FReply UMapWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FVector2D LocalPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	FVector2D MouseDelta = LocalPosition - LastMousePosition;

	switch (CurrentInputMode)
	{
	case EMapInputMode::Panning:
		HandlePanning(MouseDelta);
		break;

	case EMapInputMode::DraggingMarker:
		HandleMarkerDrag(LocalPosition);
		break;

	case EMapInputMode::None:
		{
			// Update hover states
			FName HoveredMarkerId = FindMarkerAtPosition(LocalPosition);

			for (auto& Pair : Markers)
			{
				if (Pair.Value.MarkerState == EMapMarkerState::Hovered && Pair.Key != HoveredMarkerId)
				{
					SetMarkerState(Pair.Key, EMapMarkerState::Idle);
				}
			}

			if (!HoveredMarkerId.IsNone())
			{
				FMapMarkerData* MarkerData = Markers.Find(HoveredMarkerId);
				if (MarkerData && MarkerData->MarkerState == EMapMarkerState::Idle)
				{
					SetMarkerState(HoveredMarkerId, EMapMarkerState::Hovered);
				}
			}
		}
		break;
	}

	LastMousePosition = LocalPosition;
	return FReply::Handled();
}

FReply UMapWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!MapConfig.bAllowZooming)
	{
		return FReply::Unhandled();
	}

	FVector2D LocalPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	float WheelDelta = InMouseEvent.GetWheelDelta();

	HandleZoom(WheelDelta * MapConfig.ZoomSensitivity, LocalPosition);

	return FReply::Handled();
}

void UMapWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	bIsMouseOver = true;
}

void UMapWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	bIsMouseOver = false;

	// Clear hover states
	for (auto& Pair : Markers)
	{
		if (Pair.Value.MarkerState == EMapMarkerState::Hovered)
		{
			SetMarkerState(Pair.Key, EMapMarkerState::Idle);
		}
	}
}

void UMapWidget::InitializeMap(UMapCaptureComponent* InMapCapture)
{
	MapCaptureComponent = InMapCapture;

	if (MapCaptureComponent && MapImage)
	{
		// Set the render target as the image brush
		if (UTextureRenderTarget2D* RenderTarget = MapCaptureComponent->GetMapTexture())
		{
			FSlateBrush Brush;
			Brush.SetResourceObject(RenderTarget);
			Brush.ImageSize = FVector2D(RenderTarget->SizeX, RenderTarget->SizeY);
			MapImage->SetBrush(Brush);
		}
	}
}

void UMapWidget::SetMapImage(UImage* InMapImage)
{
	MapImage = InMapImage;

	if (MapCaptureComponent && MapImage)
	{
		if (UTextureRenderTarget2D* RenderTarget = MapCaptureComponent->GetMapTexture())
		{
			FSlateBrush Brush;
			Brush.SetResourceObject(RenderTarget);
			Brush.ImageSize = FVector2D(RenderTarget->SizeX, RenderTarget->SizeY);
			MapImage->SetBrush(Brush);
		}
	}
}

void UMapWidget::SetMarkerCanvas(UCanvasPanel* InMarkerCanvas)
{
	MarkerCanvas = InMarkerCanvas;
}

bool UMapWidget::AddMarker(const FMapMarkerData& MarkerData)
{
	if (MarkerData.MarkerId.IsNone())
	{
		return false;
	}

	if (Markers.Contains(MarkerData.MarkerId))
	{
		return false;
	}

	Markers.Add(MarkerData.MarkerId, MarkerData);
	return true;
}

bool UMapWidget::RemoveMarker(FName MarkerId)
{
	return Markers.Remove(MarkerId) > 0;
}

bool UMapWidget::UpdateMarker(const FMapMarkerData& MarkerData)
{
	FMapMarkerData* Existing = Markers.Find(MarkerData.MarkerId);
	if (!Existing)
	{
		return false;
	}

	*Existing = MarkerData;
	return true;
}

bool UMapWidget::GetMarker(FName MarkerId, FMapMarkerData& OutMarkerData) const
{
	const FMapMarkerData* Found = Markers.Find(MarkerId);
	if (Found)
	{
		OutMarkerData = *Found;
		return true;
	}
	return false;
}

TArray<FMapMarkerData> UMapWidget::GetAllMarkers() const
{
	TArray<FMapMarkerData> Result;
	Markers.GenerateValueArray(Result);
	return Result;
}

bool UMapWidget::SetMarkerWorldPosition(FName MarkerId, FVector NewWorldPosition, bool bValidatePosition)
{
	FMapMarkerData* MarkerData = Markers.Find(MarkerId);
	if (!MarkerData)
	{
		return false;
	}

	if (bValidatePosition && MapCaptureComponent && MapConfig.bSnapToValidPositions)
	{
		FVector ValidPosition;
		if (MapCaptureComponent->ValidateWorldPosition(NewWorldPosition, ValidPosition))
		{
			MarkerData->WorldPosition = ValidPosition;
			MarkerData->bIsValidPosition = true;
		}
		else
		{
			MarkerData->WorldPosition = NewWorldPosition;
			MarkerData->bIsValidPosition = false;
		}
	}
	else
	{
		MarkerData->WorldPosition = NewWorldPosition;
		MarkerData->bIsValidPosition = true;
	}

	return true;
}

void UMapWidget::ClearAllMarkers()
{
	Markers.Empty();
}

FName UMapWidget::CreateOriginMarker(FVector WorldPosition)
{
	FName MarkerId = FName(*FString::Printf(TEXT("Origin_%d"), MarkerIdCounter++));
	FMapMarkerData MarkerData(MarkerId, EMapMarkerType::Origin);
	MarkerData.WorldPosition = WorldPosition;

	if (MapCaptureComponent && MapConfig.bSnapToValidPositions)
	{
		FVector ValidPosition;
		if (MapCaptureComponent->ValidateWorldPosition(WorldPosition, ValidPosition))
		{
			MarkerData.WorldPosition = ValidPosition;
			MarkerData.bIsValidPosition = true;
		}
	}

	AddMarker(MarkerData);
	return MarkerId;
}

FName UMapWidget::CreateDestinationMarker(FVector WorldPosition)
{
	FName MarkerId = FName(*FString::Printf(TEXT("Destination_%d"), MarkerIdCounter++));
	FMapMarkerData MarkerData(MarkerId, EMapMarkerType::Destination);
	MarkerData.WorldPosition = WorldPosition;

	if (MapCaptureComponent && MapConfig.bSnapToValidPositions)
	{
		FVector ValidPosition;
		if (MapCaptureComponent->ValidateWorldPosition(WorldPosition, ValidPosition))
		{
			MarkerData.WorldPosition = ValidPosition;
			MarkerData.bIsValidPosition = true;
		}
	}

	AddMarker(MarkerData);
	return MarkerId;
}

void UMapWidget::CenterOnWorldPosition(FVector WorldPosition)
{
	if (MapCaptureComponent)
	{
		MapCaptureComponent->SetMapCenter(FVector2D(WorldPosition.X, WorldPosition.Y));
	}
}

void UMapWidget::CenterOnMarker(FName MarkerId)
{
	FMapMarkerData* MarkerData = Markers.Find(MarkerId);
	if (MarkerData)
	{
		CenterOnWorldPosition(MarkerData->WorldPosition);
	}
}

void UMapWidget::FitMarkersInView(float ViewPadding)
{
	if (Markers.Num() == 0 || !MapCaptureComponent)
	{
		return;
	}

	// Find bounds of all markers
	FVector2D MinBounds(TNumericLimits<float>::Max(), TNumericLimits<float>::Max());
	FVector2D MaxBounds(TNumericLimits<float>::Lowest(), TNumericLimits<float>::Lowest());

	for (const auto& Pair : Markers)
	{
		FVector2D Pos(Pair.Value.WorldPosition.X, Pair.Value.WorldPosition.Y);
		MinBounds.X = FMath::Min(MinBounds.X, Pos.X);
		MinBounds.Y = FMath::Min(MinBounds.Y, Pos.Y);
		MaxBounds.X = FMath::Max(MaxBounds.X, Pos.X);
		MaxBounds.Y = FMath::Max(MaxBounds.Y, Pos.Y);
	}

	// Calculate center
	FVector2D Center = (MinBounds + MaxBounds) * 0.5f;
	MapCaptureComponent->SetMapCenter(Center);

	// Calculate required width
	float RequiredWidth = FMath::Max(MaxBounds.X - MinBounds.X, MaxBounds.Y - MinBounds.Y);
	RequiredWidth *= (1.0f + ViewPadding * 2.0f);

	// Calculate zoom to fit
	float BaseOrthoWidth = MapCaptureComponent->BaseOrthoWidth;
	float NewZoom = BaseOrthoWidth / RequiredWidth;
	NewZoom = FMath::Clamp(NewZoom, MapCaptureComponent->MinZoom, MapCaptureComponent->MaxZoom);

	MapCaptureComponent->SetZoom(NewZoom);
}

void UMapWidget::SetZoom(float NewZoom)
{
	if (MapCaptureComponent)
	{
		float OldZoom = MapCaptureComponent->CurrentZoom;
		MapCaptureComponent->SetZoom(NewZoom);
		OnZoomChanged.Broadcast(OldZoom, MapCaptureComponent->CurrentZoom);
	}
}

float UMapWidget::GetZoom() const
{
	return MapCaptureComponent ? MapCaptureComponent->CurrentZoom : 1.0f;
}

FVector UMapWidget::LocalToWorld(FVector2D LocalPosition) const
{
	if (!MapCaptureComponent)
	{
		return FVector::ZeroVector;
	}

	FVector2D UV = LocalToMapUV(LocalPosition);
	return MapCaptureComponent->MapUVToWorld(UV);
}

FVector2D UMapWidget::WorldToLocal(FVector WorldPosition) const
{
	if (!MapCaptureComponent)
	{
		return FVector2D::ZeroVector;
	}

	FVector2D UV = MapCaptureComponent->WorldToMapUV(WorldPosition);

	// Convert UV to local position
	FVector2D LocalSize = CachedGeometry.GetLocalSize();
	return FVector2D(UV.X * LocalSize.X, UV.Y * LocalSize.Y);
}

FVector2D UMapWidget::LocalToMapUV(FVector2D LocalPosition) const
{
	FVector2D LocalSize = CachedGeometry.GetLocalSize();

	if (LocalSize.X <= 0.0f || LocalSize.Y <= 0.0f)
	{
		return FVector2D(0.5f, 0.5f);
	}

	return FVector2D(LocalPosition.X / LocalSize.X, LocalPosition.Y / LocalSize.Y);
}

bool UMapWidget::GetMarkerAtPosition(FVector2D LocalPosition, FMapMarkerData& OutMarkerData) const
{
	FName MarkerId = FindMarkerAtPosition(LocalPosition);
	if (!MarkerId.IsNone())
	{
		return GetMarker(MarkerId, OutMarkerData);
	}
	return false;
}

bool UMapWidget::IsValidMarkerPosition(FVector2D LocalPosition, FVector& OutWorldPosition) const
{
	if (!MapCaptureComponent)
	{
		return false;
	}

	FVector2D UV = LocalToMapUV(LocalPosition);
	return MapCaptureComponent->FindValidSnapPosition(UV, OutWorldPosition);
}

void UMapWidget::UpdateMarkerPositions()
{
	// This would update visual marker widgets on the canvas
	// For Blueprint implementation, markers are handled via events
}

void UMapWidget::SetMarkerState(FName MarkerId, EMapMarkerState NewState)
{
	FMapMarkerData* MarkerData = Markers.Find(MarkerId);
	if (MarkerData && MarkerData->MarkerState != NewState)
	{
		EMapMarkerState OldState = MarkerData->MarkerState;
		MarkerData->MarkerState = NewState;
		OnMarkerStateChanged.Broadcast(MarkerId, OldState, NewState);
	}
}

FName UMapWidget::FindMarkerAtPosition(FVector2D LocalPosition) const
{
	float BestDistance = MarkerHitRadius;
	FName BestMarker = NAME_None;

	for (const auto& Pair : Markers)
	{
		if (!Pair.Value.bIsVisible)
		{
			continue;
		}

		FVector2D MarkerLocalPos = WorldToLocal(Pair.Value.WorldPosition);
		float Distance = FVector2D::Distance(LocalPosition, MarkerLocalPos);

		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestMarker = Pair.Key;
		}
	}

	return BestMarker;
}

void UMapWidget::HandlePanning(FVector2D MouseDelta)
{
	if (!MapCaptureComponent)
	{
		return;
	}

	FVector2D LocalSize = CachedGeometry.GetLocalSize();
	if (LocalSize.X <= 0.0f || LocalSize.Y <= 0.0f)
	{
		return;
	}

	// Normalize delta to -1 to 1 range
	FVector2D NormalizedDelta;
	NormalizedDelta.X = MouseDelta.X / LocalSize.X;
	NormalizedDelta.Y = MouseDelta.Y / LocalSize.Y;

	if (MapConfig.bInvertPanY)
	{
		NormalizedDelta.Y = -NormalizedDelta.Y;
	}

	MapCaptureComponent->PanMap(NormalizedDelta * MapConfig.PanSensitivity);
}

void UMapWidget::HandleMarkerDrag(FVector2D LocalPosition)
{
	if (DraggingMarkerId.IsNone() || !MapCaptureComponent)
	{
		return;
	}

	FVector2D UV = LocalToMapUV(LocalPosition);
	FVector WorldPos = MapCaptureComponent->MapUVToWorld(UV);

	FMapMarkerData* MarkerData = Markers.Find(DraggingMarkerId);
	if (!MarkerData)
	{
		return;
	}

	if (MapConfig.bSnapToValidPositions)
	{
		FVector ValidPosition;
		if (MapCaptureComponent->FindValidSnapPosition(UV, ValidPosition))
		{
			MarkerData->WorldPosition = ValidPosition;
			MarkerData->bIsValidPosition = true;
		}
		else
		{
			// Keep last valid position, mark as invalid temporarily
			MarkerData->bIsValidPosition = false;
		}
	}
	else
	{
		MarkerData->WorldPosition = WorldPos;
		MarkerData->bIsValidPosition = true;
	}
}

void UMapWidget::HandleZoom(float ZoomDelta, FVector2D LocalPosition)
{
	if (!MapCaptureComponent)
	{
		return;
	}

	float OldZoom = MapCaptureComponent->CurrentZoom;
	FVector2D UV = LocalToMapUV(LocalPosition);
	MapCaptureComponent->ZoomMap(ZoomDelta, UV);

	if (!FMath::IsNearlyEqual(OldZoom, MapCaptureComponent->CurrentZoom))
	{
		OnZoomChanged.Broadcast(OldZoom, MapCaptureComponent->CurrentZoom);
	}
}
