// Copyright 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista
// Project: AI27 Simulator

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "ScrollBoxEntryWidget.generated.h"

class UCommonScrollBoxManager;

/**
 * Delegate fired when entry selection state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEntrySelectionChanged, UScrollBoxEntryWidget*, Entry, bool, bIsSelected);

/**
 * Delegate fired when entry is clicked/activated
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntryClicked, UScrollBoxEntryWidget*, Entry);

/**
 * Base class for entries in the CommonScrollBoxManager
 *
 * This widget serves as the base class for all items displayed in the scroll box.
 * It provides:
 * - Selection state management
 * - Index tracking within the scroll box
 * - Data binding support
 * - Blueprint-extensible events
 *
 * Usage:
 * 1. Create a Blueprint subclass of this widget
 * 2. Design the visual appearance in UMG
 * 3. Override BP_OnDataSet to populate your UI
 * 4. Use SetEntryData() to bind data to the entry
 */
UCLASS(Abstract, Blueprintable, BlueprintType, meta = (DisableNativeTick))
class AI27SIMULATOR_API UScrollBoxEntryWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UScrollBoxEntryWidget(const FObjectInitializer& ObjectInitializer);

	// ========================================
	// Properties
	// ========================================

	/** Current index of this entry in the scroll box */
	UPROPERTY(BlueprintReadOnly, Category = "Entry|State", meta = (Tooltip = "Current index of this entry within the parent scroll box"))
	int32 EntryIndex;

	/** Is this entry currently selected? */
	UPROPERTY(BlueprintReadOnly, Category = "Entry|State", meta = (Tooltip = "Whether this entry is currently selected"))
	bool bIsSelected;

	/** Is this entry enabled for interaction? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry|State", meta = (Tooltip = "Whether this entry can be interacted with"))
	bool bIsEntryEnabled;

	/** Is the mouse currently hovering over this entry? */
	UPROPERTY(BlueprintReadOnly, Category = "Entry|State", meta = (Tooltip = "Whether the mouse is currently hovering over this entry"))
	bool bIsHovered;

	/** Optional tag for categorization/filtering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry|Data", meta = (Tooltip = "Optional tag for filtering or categorization"))
	FName EntryTag;

	/** Sort priority (lower values sort first) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry|Data", meta = (Tooltip = "Priority for sorting (lower values appear first)"))
	int32 SortPriority;

	// ========================================
	// Events
	// ========================================

	/** Called when selection state changes */
	UPROPERTY(BlueprintAssignable, Category = "Entry|Events", meta = (Tooltip = "Event fired when selection state changes"))
	FOnEntrySelectionChanged OnSelectionChanged;

	/** Called when entry is clicked/activated */
	UPROPERTY(BlueprintAssignable, Category = "Entry|Events", meta = (Tooltip = "Event fired when entry is clicked or activated"))
	FOnEntryClicked OnClicked;

	// ========================================
	// Data Binding
	// ========================================

	/**
	 * Set generic data for this entry
	 * @param InData UObject containing data for this entry
	 */
	UFUNCTION(BlueprintCallable, Category = "Entry|Data", meta = (Tooltip = "Set the data object for this entry"))
	virtual void SetEntryData(UObject* InData);

	/**
	 * Get the bound data object
	 * @return The data object bound to this entry
	 */
	UFUNCTION(BlueprintPure, Category = "Entry|Data", meta = (Tooltip = "Get the data object bound to this entry"))
	UObject* GetEntryData() const { return EntryData; }

	/**
	 * Set entry data from a struct (for Blueprints)
	 * Store any struct type as entry data
	 */
	UFUNCTION(BlueprintCallable, Category = "Entry|Data", CustomThunk, meta = (CustomStructureParam = "StructData", Tooltip = "Set struct data for this entry"))
	void SetEntryDataFromStruct(const int32& StructData);
	DECLARE_FUNCTION(execSetEntryDataFromStruct);

	// ========================================
	// State Management
	// ========================================

	/**
	 * Set the selection state of this entry
	 * @param bNewSelected New selection state
	 */
	UFUNCTION(BlueprintCallable, Category = "Entry|State", meta = (Tooltip = "Set whether this entry is selected"))
	virtual void SetSelected(bool bNewSelected);

	/**
	 * Toggle selection state
	 */
	UFUNCTION(BlueprintCallable, Category = "Entry|State", meta = (Tooltip = "Toggle the selection state of this entry"))
	void ToggleSelection();

	/**
	 * Set the entry index (called by manager)
	 * @param NewIndex New index in the scroll box
	 */
	UFUNCTION(BlueprintCallable, Category = "Entry|State", meta = (Tooltip = "Set the index of this entry (usually called by scroll box manager)"))
	void SetEntryIndex(int32 NewIndex);

	/**
	 * Set the owning scroll box manager
	 * @param InManager The parent scroll box manager
	 */
	void SetOwningManager(UCommonScrollBoxManager* InManager);

	/**
	 * Get the owning scroll box manager
	 * @return Parent scroll box manager
	 */
	UFUNCTION(BlueprintPure, Category = "Entry|State", meta = (Tooltip = "Get the parent scroll box manager"))
	UCommonScrollBoxManager* GetOwningManager() const { return OwningManager.Get(); }

	// ========================================
	// Query Functions
	// ========================================

	/**
	 * Check if this entry has data bound
	 * @return True if data is bound
	 */
	UFUNCTION(BlueprintPure, Category = "Entry|Data", meta = (Tooltip = "Check if this entry has data bound"))
	bool HasData() const { return EntryData != nullptr; }

	/**
	 * Check if this entry matches a tag
	 * @param Tag Tag to check
	 * @return True if entry has this tag
	 */
	UFUNCTION(BlueprintPure, Category = "Entry|Data", meta = (Tooltip = "Check if entry has the specified tag"))
	bool HasTag(FName Tag) const { return EntryTag == Tag; }

	// ========================================
	// Blueprint Implementable Events
	// ========================================

protected:
	/**
	 * Called when data is set - override in Blueprint to populate UI
	 * @param Data The data object that was set
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events", meta = (DisplayName = "On Data Set", Tooltip = "Called when SetEntryData is called - use to populate UI"))
	void BP_OnDataSet(UObject* Data);

	/**
	 * Called when selection state changes - override for visual feedback
	 * @param bSelected New selection state
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events", meta = (DisplayName = "On Selection Changed", Tooltip = "Called when selection state changes - use for visual feedback"))
	void BP_OnSelectionChanged(bool bSelected);

	/**
	 * Called when the entry is clicked/activated
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events", meta = (DisplayName = "On Entry Clicked", Tooltip = "Called when this entry is clicked or activated"))
	void BP_OnClicked();

	/**
	 * Called when index changes - override if index display needed
	 * @param NewIndex The new index
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events", meta = (DisplayName = "On Index Changed", Tooltip = "Called when the entry index changes"))
	void BP_OnIndexChanged(int32 NewIndex);

	/**
	 * Called when entry is added to scroll box
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events", meta = (DisplayName = "On Added To ScrollBox", Tooltip = "Called when this entry is added to a scroll box"))
	void BP_OnAddedToScrollBox();

	/**
	 * Called when entry is about to be removed from scroll box
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events", meta = (DisplayName = "On Removing From ScrollBox", Tooltip = "Called just before this entry is removed from a scroll box"))
	void BP_OnRemovingFromScrollBox();

	/**
	 * Called when mouse enters the entry
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events", meta = (DisplayName = "On Hovered", Tooltip = "Called when mouse enters this entry"))
	void BP_OnHovered();

	/**
	 * Called when mouse leaves the entry
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events", meta = (DisplayName = "On Unhovered", Tooltip = "Called when mouse leaves this entry"))
	void BP_OnUnhovered();

	// ========================================
	// Click/Activation
	// ========================================

public:
	/**
	 * Trigger click event on this entry
	 * Call this from Blueprint when your button or interactive element is clicked
	 */
	UFUNCTION(BlueprintCallable, Category = "Entry|Interaction", meta = (Tooltip = "Trigger the click event on this entry. Call from your button's OnClicked."))
	void TriggerClick();

protected:
	// ========================================
	// Native Overrides
	// ========================================

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	/** Internal click handler */
	void HandleClicked();

	// ========================================
	// Internal Data
	// ========================================

private:
	/** Bound data object */
	UPROPERTY()
	TObjectPtr<UObject> EntryData;

	/** Weak reference to owning manager */
	TWeakObjectPtr<UCommonScrollBoxManager> OwningManager;

	/** Stored struct data for Blueprint struct binding */
	TSharedPtr<FStructOnScope> StoredStructData;

	/** Track if mouse button is pressed on this widget */
	bool bMouseButtonDown;

	/** Friend class for internal access */
	friend class UCommonScrollBoxManager;
};
