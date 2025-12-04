// Copyright Ai27. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapTypes.h"
#include "MapMarkerWidget.generated.h"

class UImage;
class UTextBlock;
class UBorder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMarkerWidgetClicked, FName, MarkerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMarkerWidgetDragStarted, FName, MarkerId, FVector2D, ScreenPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMarkerWidgetDragged, FName, MarkerId, FVector2D, ScreenPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMarkerWidgetDragEnded, FName, MarkerId, FVector2D, ScreenPosition);

/**
 * Widget for displaying a single map marker.
 * Can be used as a visual representation of markers on the map.
 */
UCLASS(Blueprintable, BlueprintType)
class MAPSYSTEM_API UMapMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMapMarkerWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

public:
	// ==================== Marker Data ====================

	/** The marker data this widget represents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FMapMarkerData MarkerData;

	// ==================== Visual Configuration ====================

	/** Color when idle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Visual")
	FLinearColor IdleColor = FLinearColor::White;

	/** Color when hovered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Visual")
	FLinearColor HoveredColor = FLinearColor(1.2f, 1.2f, 1.2f, 1.0f);

	/** Color when dragging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Visual")
	FLinearColor DraggingColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);

	/** Color when in invalid position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Visual")
	FLinearColor InvalidColor = FLinearColor(1.0f, 0.3f, 0.3f, 1.0f);

	/** Scale when hovered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker|Visual")
	float HoveredScale = 1.2f;

	// ==================== Events ====================

	UPROPERTY(BlueprintAssignable, Category = "Marker|Events")
	FOnMarkerWidgetClicked OnClicked;

	UPROPERTY(BlueprintAssignable, Category = "Marker|Events")
	FOnMarkerWidgetDragStarted OnDragStarted;

	UPROPERTY(BlueprintAssignable, Category = "Marker|Events")
	FOnMarkerWidgetDragged OnDragged;

	UPROPERTY(BlueprintAssignable, Category = "Marker|Events")
	FOnMarkerWidgetDragEnded OnDragEnded;

	// ==================== Functions ====================

	/** Initialize the marker widget with data */
	UFUNCTION(BlueprintCallable, Category = "Marker")
	void InitializeMarker(const FMapMarkerData& InMarkerData);

	/** Update the marker data */
	UFUNCTION(BlueprintCallable, Category = "Marker")
	void UpdateMarkerData(const FMapMarkerData& InMarkerData);

	/** Set the marker state and update visuals */
	UFUNCTION(BlueprintCallable, Category = "Marker")
	void SetMarkerState(EMapMarkerState NewState);

	/** Update visual appearance based on current state */
	UFUNCTION(BlueprintCallable, Category = "Marker")
	void UpdateVisuals();

	/** Get the marker ID */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Marker")
	FName GetMarkerId() const { return MarkerData.MarkerId; }

	/** Check if currently being dragged */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Marker")
	bool IsDragging() const { return bIsDragging; }

	// ==================== Blueprint Implementable Events ====================

	/** Called when visuals should be updated - override in Blueprint to customize appearance */
	UFUNCTION(BlueprintImplementableEvent, Category = "Marker")
	void OnUpdateVisuals();

	/** Called when marker is initialized */
	UFUNCTION(BlueprintImplementableEvent, Category = "Marker")
	void OnMarkerInitialized();

protected:
	/** Internal dragging state */
	bool bIsDragging = false;

	/** Internal hovered state */
	bool bIsHovered = false;

	/** Starting position when drag began */
	FVector2D DragStartPosition;
};
