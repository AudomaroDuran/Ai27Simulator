// Copyright 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista
// Project: AI27 Simulator

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Components/Widget.h"
#include "CommonScrollBoxManager.generated.h"

class UScrollBoxEntryWidget;
class UCommonHierarchicalScrollBox;
class UPanelWidget;

// ========================================
// Delegates
// ========================================

/** Delegate fired when a widget is added to the scroll box */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScrollBoxWidgetAdded, UScrollBoxEntryWidget*, Widget, int32, Index);

/** Delegate fired when a widget is removed from the scroll box */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScrollBoxWidgetRemoved, UScrollBoxEntryWidget*, Widget, int32, Index);

/** Delegate fired when a widget is selected */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScrollBoxWidgetSelected, UScrollBoxEntryWidget*, Widget, int32, Index);

/** Delegate fired when the scroll box content is updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScrollBoxUpdated, int32, WidgetCount);

/** Delegate fired when selection changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScrollBoxSelectionChanged, UScrollBoxEntryWidget*, NewSelection, UScrollBoxEntryWidget*, OldSelection);

/** Delegate for filtering widgets */
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FScrollBoxFilterPredicate, UScrollBoxEntryWidget*, Widget);

/** Delegate for sorting widgets */
DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FScrollBoxSortPredicate, UScrollBoxEntryWidget*, A, UScrollBoxEntryWidget*, B);

// ========================================
// Enums
// ========================================

/**
 * Selection mode for the scroll box
 */
UENUM(BlueprintType)
enum class EScrollBoxSelectionMode : uint8
{
	/** No selection allowed */
	None UMETA(DisplayName = "None"),

	/** Only one item can be selected at a time */
	Single UMETA(DisplayName = "Single"),

	/** Multiple items can be selected */
	Multi UMETA(DisplayName = "Multi")
};

/**
 * Sort order for entries
 */
UENUM(BlueprintType)
enum class EScrollBoxSortOrder : uint8
{
	/** Sort in ascending order */
	Ascending UMETA(DisplayName = "Ascending"),

	/** Sort in descending order */
	Descending UMETA(DisplayName = "Descending")
};

/**
 * CommonUI-based scroll box manager for activatable widgets
 *
 * This widget provides comprehensive management of child widgets in a
 * CommonHierarchicalScrollBox. It supports:
 * - Adding/removing/updating widgets
 * - Single and multi-selection modes
 * - Sorting and filtering
 * - Focus and navigation management
 * - Full Blueprint exposure
 *
 * Usage:
 * 1. Create a Blueprint subclass
 * 2. In the UMG designer, add a CommonHierarchicalScrollBox named "ScrollBox"
 * 3. Use AddEntry/RemoveEntry functions to manage content
 * 4. Bind to events for selection and updates
 *
 * @see UScrollBoxEntryWidget for entry widget base class
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisableNativeTick))
class AI27SIMULATOR_API UCommonScrollBoxManager : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UCommonScrollBoxManager(const FObjectInitializer& ObjectInitializer);

	// ========================================
	// Configuration Properties
	// ========================================

	/** Selection mode for entries */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScrollBox|Config", meta = (Tooltip = "How entries can be selected"))
	EScrollBoxSelectionMode SelectionMode;

	/** Should entries be focusable via gamepad/keyboard? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScrollBox|Config", meta = (Tooltip = "Allow gamepad/keyboard navigation between entries"))
	bool bEnableNavigation;

	/** Auto-scroll to newly added entries? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScrollBox|Config", meta = (Tooltip = "Automatically scroll to show newly added entries"))
	bool bAutoScrollToNewEntries;

	/** Auto-scroll to selected entries? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScrollBox|Config", meta = (Tooltip = "Automatically scroll to show selected entries"))
	bool bAutoScrollToSelection;

	/** Default entry widget class for CreateEntry functions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScrollBox|Config", meta = (Tooltip = "Default widget class to use when creating entries"))
	TSubclassOf<UScrollBoxEntryWidget> DefaultEntryClass;

	// ========================================
	// Events
	// ========================================

	/** Called when a widget is added */
	UPROPERTY(BlueprintAssignable, Category = "ScrollBox|Events", meta = (Tooltip = "Event fired when an entry is added"))
	FOnScrollBoxWidgetAdded OnWidgetAdded;

	/** Called when a widget is removed */
	UPROPERTY(BlueprintAssignable, Category = "ScrollBox|Events", meta = (Tooltip = "Event fired when an entry is removed"))
	FOnScrollBoxWidgetRemoved OnWidgetRemoved;

	/** Called when a widget is selected */
	UPROPERTY(BlueprintAssignable, Category = "ScrollBox|Events", meta = (Tooltip = "Event fired when an entry is selected"))
	FOnScrollBoxWidgetSelected OnWidgetSelected;

	/** Called when the scroll box is updated */
	UPROPERTY(BlueprintAssignable, Category = "ScrollBox|Events", meta = (Tooltip = "Event fired after scroll box content is updated"))
	FOnScrollBoxUpdated OnScrollBoxUpdated;

	/** Called when selection changes */
	UPROPERTY(BlueprintAssignable, Category = "ScrollBox|Events", meta = (Tooltip = "Event fired when selected entry changes"))
	FOnScrollBoxSelectionChanged OnSelectionChanged;

	// ========================================
	// Adding Widgets
	// ========================================

	/**
	 * Add an existing entry widget to the end of the scroll box
	 * @param Entry The entry widget to add
	 * @return Index where the widget was added, or -1 if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add", meta = (Tooltip = "Add an existing entry widget to the end of the scroll box"))
	int32 AddEntry(UScrollBoxEntryWidget* Entry);

	/**
	 * Add an entry widget at a specific index
	 * @param Entry The entry widget to add
	 * @param Index Index where to insert (entries after will shift)
	 * @return Actual index where inserted, or -1 if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add", meta = (Tooltip = "Add an entry widget at a specific index"))
	int32 AddEntryAtIndex(UScrollBoxEntryWidget* Entry, int32 Index);

	/**
	 * Create and add a new entry of the specified class
	 * @param EntryClass Class of entry to create (must inherit UScrollBoxEntryWidget)
	 * @return Created entry widget, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add", meta = (Tooltip = "Create and add a new entry widget of the specified class", DeterminesOutputType = "EntryClass"))
	UScrollBoxEntryWidget* CreateAndAddEntry(TSubclassOf<UScrollBoxEntryWidget> EntryClass);

	/**
	 * Create and add a new entry using the default entry class
	 * @return Created entry widget, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add", meta = (Tooltip = "Create and add a new entry using the default entry class"))
	UScrollBoxEntryWidget* CreateAndAddDefaultEntry();

	/**
	 * Create and add a new entry with data
	 * @param EntryClass Class of entry to create
	 * @param Data Data object to bind to the entry
	 * @return Created entry widget, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add", meta = (Tooltip = "Create entry with data", DeterminesOutputType = "EntryClass"))
	UScrollBoxEntryWidget* CreateAndAddEntryWithData(TSubclassOf<UScrollBoxEntryWidget> EntryClass, UObject* Data);

	/**
	 * Add multiple entries at once
	 * @param Entries Array of entry widgets to add
	 * @return Number of entries successfully added
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add", meta = (Tooltip = "Add multiple entries at once"))
	int32 AddEntries(const TArray<UScrollBoxEntryWidget*>& Entries);

	// ========================================
	// Removing Widgets
	// ========================================

	/**
	 * Remove an entry widget from the scroll box
	 * @param Entry The entry to remove
	 * @return True if removed successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Remove", meta = (Tooltip = "Remove an entry widget from the scroll box"))
	bool RemoveEntry(UScrollBoxEntryWidget* Entry);

	/**
	 * Remove entry at specific index
	 * @param Index Index of entry to remove
	 * @return The removed entry, or nullptr if index invalid
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Remove", meta = (Tooltip = "Remove entry at specific index"))
	UScrollBoxEntryWidget* RemoveEntryAtIndex(int32 Index);

	/**
	 * Remove all entries from the scroll box
	 * @param bDestroyWidgets If true, destroy the widget instances
	 * @return Number of entries removed
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Remove", meta = (Tooltip = "Remove all entries from the scroll box"))
	int32 ClearAllEntries(bool bDestroyWidgets = false);

	/**
	 * Remove entries matching a predicate
	 * @param Predicate Filter function - return true to remove
	 * @return Number of entries removed
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Remove", meta = (Tooltip = "Remove entries matching a filter predicate"))
	int32 RemoveEntriesWhere(FScrollBoxFilterPredicate Predicate);

	/**
	 * Remove entries by tag
	 * @param Tag Tag to match
	 * @return Number of entries removed
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Remove", meta = (Tooltip = "Remove all entries with a specific tag"))
	int32 RemoveEntriesByTag(FName Tag);

	// ========================================
	// Accessing Widgets
	// ========================================

	/**
	 * Get entry at specific index
	 * @param Index Index of entry
	 * @return Entry widget or nullptr if invalid index
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Access", meta = (Tooltip = "Get entry at specific index"))
	UScrollBoxEntryWidget* GetEntryAtIndex(int32 Index) const;

	/**
	 * Get all entries
	 * @return Array of all entry widgets
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Access", meta = (Tooltip = "Get all entries in the scroll box"))
	TArray<UScrollBoxEntryWidget*> GetAllEntries() const;

	/**
	 * Get number of entries
	 * @return Total number of entries
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Access", meta = (Tooltip = "Get the total number of entries"))
	int32 GetEntryCount() const;

	/**
	 * Check if scroll box is empty
	 * @return True if no entries
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Access", meta = (Tooltip = "Check if scroll box has no entries"))
	bool IsEmpty() const;

	/**
	 * Get index of an entry
	 * @param Entry Entry to find
	 * @return Index of entry, or -1 if not found
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Access", meta = (Tooltip = "Get the index of a specific entry"))
	int32 GetEntryIndex(UScrollBoxEntryWidget* Entry) const;

	/**
	 * Check if entry exists in scroll box
	 * @param Entry Entry to check
	 * @return True if entry is in scroll box
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Access", meta = (Tooltip = "Check if an entry exists in the scroll box"))
	bool ContainsEntry(UScrollBoxEntryWidget* Entry) const;

	/**
	 * Find entry by predicate
	 * @param Predicate Filter function
	 * @return First matching entry or nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Access", meta = (Tooltip = "Find first entry matching a predicate"))
	UScrollBoxEntryWidget* FindEntry(FScrollBoxFilterPredicate Predicate) const;

	/**
	 * Find all entries matching predicate
	 * @param Predicate Filter function
	 * @return Array of matching entries
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Access", meta = (Tooltip = "Find all entries matching a predicate"))
	TArray<UScrollBoxEntryWidget*> FindEntries(FScrollBoxFilterPredicate Predicate) const;

	/**
	 * Find entries by tag
	 * @param Tag Tag to search for
	 * @return Array of entries with matching tag
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Access", meta = (Tooltip = "Find all entries with a specific tag"))
	TArray<UScrollBoxEntryWidget*> FindEntriesByTag(FName Tag) const;

	/**
	 * Get first entry
	 * @return First entry or nullptr if empty
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Access", meta = (Tooltip = "Get the first entry in the scroll box"))
	UScrollBoxEntryWidget* GetFirstEntry() const;

	/**
	 * Get last entry
	 * @return Last entry or nullptr if empty
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Access", meta = (Tooltip = "Get the last entry in the scroll box"))
	UScrollBoxEntryWidget* GetLastEntry() const;

	// ========================================
	// Selection Management
	// ========================================

	/**
	 * Select an entry
	 * @param Entry Entry to select
	 * @param bNotify Broadcast selection events
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection", meta = (Tooltip = "Select an entry"))
	void SelectEntry(UScrollBoxEntryWidget* Entry, bool bNotify = true);

	/**
	 * Select entry at index
	 * @param Index Index to select
	 * @param bNotify Broadcast selection events
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection", meta = (Tooltip = "Select entry at specific index"))
	void SelectEntryAtIndex(int32 Index, bool bNotify = true);

	/**
	 * Deselect an entry
	 * @param Entry Entry to deselect
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection", meta = (Tooltip = "Deselect an entry"))
	void DeselectEntry(UScrollBoxEntryWidget* Entry);

	/**
	 * Clear all selections
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection", meta = (Tooltip = "Clear all selections"))
	void ClearSelection();

	/**
	 * Get currently selected entry (single selection mode)
	 * @return Selected entry or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Selection", meta = (Tooltip = "Get the currently selected entry"))
	UScrollBoxEntryWidget* GetSelectedEntry() const;

	/**
	 * Get all selected entries (multi selection mode)
	 * @return Array of selected entries
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Selection", meta = (Tooltip = "Get all selected entries"))
	TArray<UScrollBoxEntryWidget*> GetSelectedEntries() const;

	/**
	 * Get selected entry index
	 * @return Index of selected entry, or -1 if none
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Selection", meta = (Tooltip = "Get the index of the selected entry"))
	int32 GetSelectedIndex() const;

	/**
	 * Check if any entry is selected
	 * @return True if at least one entry is selected
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Selection", meta = (Tooltip = "Check if any entry is selected"))
	bool HasSelection() const;

	/**
	 * Select next entry (for navigation)
	 * @param bWrap Wrap around to first when at end
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection", meta = (Tooltip = "Select the next entry (for navigation)"))
	void SelectNextEntry(bool bWrap = true);

	/**
	 * Select previous entry (for navigation)
	 * @param bWrap Wrap around to last when at beginning
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection", meta = (Tooltip = "Select the previous entry (for navigation)"))
	void SelectPreviousEntry(bool bWrap = true);

	// ========================================
	// Sorting and Organization
	// ========================================

	/**
	 * Sort entries by priority
	 * @param Order Ascending or descending
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Sort", meta = (Tooltip = "Sort entries by their SortPriority property"))
	void SortByPriority(EScrollBoxSortOrder Order = EScrollBoxSortOrder::Ascending);

	/**
	 * Sort entries using custom predicate
	 * @param Predicate Comparison function (return true if A should come before B)
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Sort", meta = (Tooltip = "Sort entries using a custom comparison"))
	void SortEntries(FScrollBoxSortPredicate Predicate);

	/**
	 * Move entry to new index
	 * @param Entry Entry to move
	 * @param NewIndex Target index
	 * @return True if moved successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Sort", meta = (Tooltip = "Move an entry to a new index"))
	bool MoveEntry(UScrollBoxEntryWidget* Entry, int32 NewIndex);

	/**
	 * Swap two entries
	 * @param IndexA First index
	 * @param IndexB Second index
	 * @return True if swapped successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Sort", meta = (Tooltip = "Swap two entries by index"))
	bool SwapEntries(int32 IndexA, int32 IndexB);

	/**
	 * Reverse the order of all entries
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Sort", meta = (Tooltip = "Reverse the order of all entries"))
	void ReverseOrder();

	// ========================================
	// Scrolling
	// ========================================

	/**
	 * Scroll to show specific entry
	 * @param Entry Entry to scroll to
	 * @param bAnimated Use smooth animation
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Scroll", meta = (Tooltip = "Scroll to show a specific entry"))
	void ScrollToEntry(UScrollBoxEntryWidget* Entry, bool bAnimated = true);

	/**
	 * Scroll to entry at index
	 * @param Index Index to scroll to
	 * @param bAnimated Use smooth animation
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Scroll", meta = (Tooltip = "Scroll to entry at specific index"))
	void ScrollToIndex(int32 Index, bool bAnimated = true);

	/**
	 * Scroll to top of list
	 * @param bAnimated Use smooth animation
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Scroll", meta = (Tooltip = "Scroll to the top of the list"))
	void ScrollToTop(bool bAnimated = true);

	/**
	 * Scroll to bottom of list
	 * @param bAnimated Use smooth animation
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Scroll", meta = (Tooltip = "Scroll to the bottom of the list"))
	void ScrollToBottom(bool bAnimated = true);

	/**
	 * Set scroll offset directly
	 * @param Offset Scroll offset value
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Scroll", meta = (Tooltip = "Set the scroll offset directly"))
	void SetScrollOffset(float Offset);

	/**
	 * Get current scroll offset
	 * @return Current scroll offset
	 */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Scroll", meta = (Tooltip = "Get the current scroll offset"))
	float GetScrollOffset() const;

	// ========================================
	// Update and Refresh
	// ========================================

	/**
	 * Refresh all entry indices
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Update", meta = (Tooltip = "Refresh the indices of all entries"))
	void RefreshEntryIndices();

	/**
	 * Force visual refresh of scroll box
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Update", meta = (Tooltip = "Force a visual refresh of the scroll box"))
	void ForceRefresh();

	/**
	 * Regenerate scroll box content from entries array
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Update", meta = (Tooltip = "Regenerate scroll box content from internal entries"))
	void RegenerateContent();

	// ========================================
	// Focus and Navigation
	// ========================================

	/**
	 * Focus the first entry
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Focus", meta = (Tooltip = "Set focus to the first entry"))
	void FocusFirstEntry();

	/**
	 * Focus a specific entry
	 * @param Entry Entry to focus
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Focus", meta = (Tooltip = "Set focus to a specific entry"))
	void FocusEntry(UScrollBoxEntryWidget* Entry);

	/**
	 * Focus entry at index
	 * @param Index Index of entry to focus
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Focus", meta = (Tooltip = "Set focus to entry at specific index"))
	void FocusEntryAtIndex(int32 Index);

	// ========================================
	// Blueprint Implementable Events
	// ========================================

protected:
	/**
	 * Called when an entry is about to be added
	 * @param Entry Entry being added
	 * @param Index Target index
	 * @return True to allow adding, false to cancel
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ScrollBox|Events", meta = (DisplayName = "On Entry Adding"))
	bool BP_OnEntryAdding(UScrollBoxEntryWidget* Entry, int32 Index);
	virtual bool BP_OnEntryAdding_Implementation(UScrollBoxEntryWidget* Entry, int32 Index) { return true; }

	/**
	 * Called after an entry is added
	 * @param Entry Entry that was added
	 * @param Index Index where added
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ScrollBox|Events", meta = (DisplayName = "On Entry Added"))
	void BP_OnEntryAdded(UScrollBoxEntryWidget* Entry, int32 Index);

	/**
	 * Called when an entry is about to be removed
	 * @param Entry Entry being removed
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ScrollBox|Events", meta = (DisplayName = "On Entry Removing"))
	void BP_OnEntryRemoving(UScrollBoxEntryWidget* Entry);

	/**
	 * Called after content is cleared
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ScrollBox|Events", meta = (DisplayName = "On Content Cleared"))
	void BP_OnContentCleared();

	// ========================================
	// Widget Binding
	// ========================================

	/**
	 * The scroll box widget to use for displaying entries.
	 * Can be bound via BindWidget meta or set manually in Blueprint.
	 * If not set, will try to find widget named "ScrollBox" automatically.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ScrollBox|Binding", meta = (BindWidget, OptionalWidget = true, Tooltip = "The scroll box widget for entries. Will auto-find 'ScrollBox' if not set."))
	TObjectPtr<UPanelWidget> ScrollBox;

	/** Get the scroll box widget */
	UFUNCTION(BlueprintPure, Category = "ScrollBox|Internal", meta = (Tooltip = "Get the underlying scroll box widget"))
	UPanelWidget* GetScrollBoxWidget() const;

	/**
	 * Manually set the scroll box widget (useful if auto-binding fails)
	 * @param InScrollBox The panel widget to use as scroll box container
	 */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Binding", meta = (Tooltip = "Manually set the scroll box widget"))
	void SetScrollBoxWidget(UPanelWidget* InScrollBox);

	/** Try to find and bind scroll box widget automatically */
	UFUNCTION(BlueprintCallable, Category = "ScrollBox|Binding", meta = (Tooltip = "Try to find scroll box widget automatically"))
	void TryAutoBindScrollBox();

	// ========================================
	// Internal
	// ========================================

	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativePreConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	/** Internal array of managed entries */
	UPROPERTY()
	TArray<TObjectPtr<UScrollBoxEntryWidget>> ManagedEntries;

	/** Currently selected entry (single selection) */
	UPROPERTY()
	TObjectPtr<UScrollBoxEntryWidget> CurrentSelection;

	/** Selected entries (multi selection) */
	UPROPERTY()
	TArray<TObjectPtr<UScrollBoxEntryWidget>> SelectedEntries;

	/** Cached reference to active scroll box widget */
	UPROPERTY()
	TObjectPtr<UPanelWidget> ActiveScrollBoxWidget;

	/** Internal helper to add entry to scroll box widget */
	void AddEntryToScrollBox(UScrollBoxEntryWidget* Entry);

	/** Internal helper to remove entry from scroll box widget */
	void RemoveEntryFromScrollBox(UScrollBoxEntryWidget* Entry);

	/** Update entry indices after modification */
	void UpdateEntryIndices();

	/** Handle entry clicked from child */
	UFUNCTION()
	void HandleEntryClicked(UScrollBoxEntryWidget* Entry);

	/** Handle entry selection from child */
	UFUNCTION()
	void HandleEntrySelectionChanged(UScrollBoxEntryWidget* Entry, bool bIsSelected);
};
