// Copyright 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista
// Project: AI27 Simulator

#include "UI/ScrollBoxEntryWidget.h"
#include "UI/CommonScrollBoxManager.h"

UScrollBoxEntryWidget::UScrollBoxEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EntryIndex(INDEX_NONE)
	, bIsSelected(false)
	, bIsEntryEnabled(true)
	, bIsHovered(false)
	, EntryTag(NAME_None)
	, SortPriority(0)
	, EntryData(nullptr)
	, bMouseButtonDown(false)
{
	// Configure activation behavior
	bAutoActivate = true;

	// Enable mouse events
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
}

void UScrollBoxEntryWidget::SetEntryData(UObject* InData)
{
	EntryData = InData;

	// Notify Blueprint
	BP_OnDataSet(InData);

	UE_LOG(LogTemp, Verbose, TEXT("ScrollBoxEntry[%d]: Data set to %s"),
		EntryIndex, InData ? *InData->GetName() : TEXT("nullptr"));
}

DEFINE_FUNCTION(UScrollBoxEntryWidget::execSetEntryDataFromStruct)
{
	// Step into the struct parameter
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	void* StructAddr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	if (StructProperty && StructAddr)
	{
		P_THIS->StoredStructData = MakeShared<FStructOnScope>(StructProperty->Struct);
		StructProperty->Struct->CopyScriptStruct(P_THIS->StoredStructData->GetStructMemory(), StructAddr);
	}
}

void UScrollBoxEntryWidget::SetSelected(bool bNewSelected)
{
	if (bIsSelected != bNewSelected)
	{
		bIsSelected = bNewSelected;

		// Notify via delegate
		OnSelectionChanged.Broadcast(this, bIsSelected);

		// Notify Blueprint
		BP_OnSelectionChanged(bIsSelected);

		UE_LOG(LogTemp, Verbose, TEXT("ScrollBoxEntry[%d]: Selection changed to %s"),
			EntryIndex, bIsSelected ? TEXT("true") : TEXT("false"));
	}
}

void UScrollBoxEntryWidget::ToggleSelection()
{
	SetSelected(!bIsSelected);
}

void UScrollBoxEntryWidget::SetEntryIndex(int32 NewIndex)
{
	if (EntryIndex != NewIndex)
	{
		int32 OldIndex = EntryIndex;
		EntryIndex = NewIndex;

		// Notify Blueprint
		BP_OnIndexChanged(NewIndex);

		UE_LOG(LogTemp, Verbose, TEXT("ScrollBoxEntry: Index changed from %d to %d"),
			OldIndex, NewIndex);
	}
}

void UScrollBoxEntryWidget::SetOwningManager(UCommonScrollBoxManager* InManager)
{
	OwningManager = InManager;

	if (InManager)
	{
		BP_OnAddedToScrollBox();
	}
}

void UScrollBoxEntryWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	UE_LOG(LogTemp, Verbose, TEXT("ScrollBoxEntry[%d]: Activated"), EntryIndex);
}

void UScrollBoxEntryWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	UE_LOG(LogTemp, Verbose, TEXT("ScrollBoxEntry[%d]: Deactivated"), EntryIndex);
}

FReply UScrollBoxEntryWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	// When focused, we might want to select this entry
	if (OwningManager.IsValid())
	{
		// Optionally auto-select on focus
		// SetSelected(true);
	}

	return Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
}

void UScrollBoxEntryWidget::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);
}

void UScrollBoxEntryWidget::TriggerClick()
{
	HandleClicked();
}

void UScrollBoxEntryWidget::HandleClicked()
{
	if (!bIsEntryEnabled)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ScrollBoxEntry[%d]: Click ignored - entry disabled"), EntryIndex);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ScrollBoxEntry[%d]: Clicked!"), EntryIndex);

	// Broadcast delegate
	OnClicked.Broadcast(this);

	// Notify Blueprint
	BP_OnClicked();
}

FReply UScrollBoxEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bMouseButtonDown = true;
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UScrollBoxEntryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// Only trigger click if mouse was pressed down on this widget and is still hovering
		if (bMouseButtonDown && bIsHovered)
		{
			HandleClicked();
		}
		bMouseButtonDown = false;
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UScrollBoxEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	bIsHovered = true;
	BP_OnHovered();

	UE_LOG(LogTemp, Verbose, TEXT("ScrollBoxEntry[%d]: Mouse Enter"), EntryIndex);
}

void UScrollBoxEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	bIsHovered = false;
	bMouseButtonDown = false; // Reset if mouse leaves while pressed
	BP_OnUnhovered();

	UE_LOG(LogTemp, Verbose, TEXT("ScrollBoxEntry[%d]: Mouse Leave"), EntryIndex);
}
