// Copyright Ai27. All Rights Reserved.

#include "MapMarkerWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

UMapMarkerWidget::UMapMarkerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UMapMarkerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateVisuals();
}

FReply UMapMarkerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (MarkerData.bIsDraggable)
		{
			bIsDragging = true;
			DragStartPosition = InMouseEvent.GetScreenSpacePosition();
			SetMarkerState(EMapMarkerState::Dragging);
			OnDragStarted.Broadcast(MarkerData.MarkerId, DragStartPosition);
			return FReply::Handled().CaptureMouse(TakeWidget());
		}
		else
		{
			OnClicked.Broadcast(MarkerData.MarkerId);
		}
	}

	return FReply::Handled();
}

FReply UMapMarkerWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = false;
		FVector2D EndPosition = InMouseEvent.GetScreenSpacePosition();

		// If didn't move much, treat as click
		float DragDistance = FVector2D::Distance(DragStartPosition, EndPosition);
		if (DragDistance < 5.0f)
		{
			OnClicked.Broadcast(MarkerData.MarkerId);
		}

		OnDragEnded.Broadcast(MarkerData.MarkerId, EndPosition);
		SetMarkerState(bIsHovered ? EMapMarkerState::Hovered : EMapMarkerState::Idle);
		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Handled();
}

FReply UMapMarkerWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging)
	{
		FVector2D CurrentPosition = InMouseEvent.GetScreenSpacePosition();
		OnDragged.Broadcast(MarkerData.MarkerId, CurrentPosition);
	}

	return FReply::Handled();
}

void UMapMarkerWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	bIsHovered = true;

	if (!bIsDragging)
	{
		SetMarkerState(EMapMarkerState::Hovered);
	}
}

void UMapMarkerWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	bIsHovered = false;

	if (!bIsDragging)
	{
		SetMarkerState(MarkerData.bIsValidPosition ? EMapMarkerState::Idle : EMapMarkerState::Invalid);
	}
}

void UMapMarkerWidget::InitializeMarker(const FMapMarkerData& InMarkerData)
{
	MarkerData = InMarkerData;

	// Set default colors based on marker type
	switch (MarkerData.MarkerType)
	{
	case EMapMarkerType::Origin:
		IdleColor = FLinearColor::Green;
		break;
	case EMapMarkerType::Destination:
		IdleColor = FLinearColor::Red;
		break;
	default:
		IdleColor = MarkerData.Color;
		break;
	}

	UpdateVisuals();
	OnMarkerInitialized();
}

void UMapMarkerWidget::UpdateMarkerData(const FMapMarkerData& InMarkerData)
{
	EMapMarkerState OldState = MarkerData.MarkerState;
	MarkerData = InMarkerData;

	if (OldState != MarkerData.MarkerState)
	{
		UpdateVisuals();
	}
}

void UMapMarkerWidget::SetMarkerState(EMapMarkerState NewState)
{
	if (MarkerData.MarkerState != NewState)
	{
		MarkerData.MarkerState = NewState;
		UpdateVisuals();
	}
}

void UMapMarkerWidget::UpdateVisuals()
{
	// Call Blueprint event for custom visual updates
	OnUpdateVisuals();

	// Apply scale transform based on state
	FVector2D NewScale(1.0f, 1.0f);

	if (MarkerData.MarkerState == EMapMarkerState::Hovered ||
	    MarkerData.MarkerState == EMapMarkerState::Dragging)
	{
		NewScale = FVector2D(HoveredScale, HoveredScale);
	}

	SetRenderScale(NewScale);
}
