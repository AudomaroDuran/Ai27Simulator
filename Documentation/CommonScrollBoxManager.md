# CommonScrollBoxManager

## Overview

`UCommonScrollBoxManager` is a CommonUI-based widget that provides comprehensive management of activatable child widgets in a hierarchical scroll box. It offers full Blueprint exposure for UMG integration.

**File Location:** `Source/ai27Simulator/Public/UI/CommonScrollBoxManager.h`
**Implementation:** `Source/ai27Simulator/Private/UI/CommonScrollBoxManager.cpp`

## Class Declaration

```cpp
UCLASS(Blueprintable, BlueprintType, meta = (DisableNativeTick))
class AI27SIMULATOR_API UCommonScrollBoxManager : public UCommonActivatableWidget
```

## Features

- **Widget Management**: Add, remove, update, and organize child widgets
- **Selection Modes**: None, Single, and Multi-selection support
- **Sorting & Filtering**: Sort by priority or custom predicates, filter by tag or predicate
- **Navigation**: Keyboard/gamepad navigation between entries
- **Scrolling Control**: Scroll to specific entries, auto-scroll on selection
- **Full Blueprint Exposure**: All functions available in Blueprint
- **Event System**: Comprehensive delegates for all operations

## Enumerations

### EScrollBoxSelectionMode

```cpp
UENUM(BlueprintType)
enum class EScrollBoxSelectionMode : uint8
{
    None,    // No selection allowed
    Single,  // Only one item selected at a time
    Multi    // Multiple items can be selected
};
```

### EScrollBoxSortOrder

```cpp
UENUM(BlueprintType)
enum class EScrollBoxSortOrder : uint8
{
    Ascending,   // Lower values first
    Descending   // Higher values first
};
```

## Configuration Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `SelectionMode` | `EScrollBoxSelectionMode` | Single | How entries can be selected |
| `bEnableNavigation` | `bool` | true | Allow keyboard/gamepad navigation |
| `bAutoScrollToNewEntries` | `bool` | false | Scroll to newly added entries |
| `bAutoScrollToSelection` | `bool` | true | Scroll to selected entries |
| `DefaultEntryClass` | `TSubclassOf<UScrollBoxEntryWidget>` | nullptr | Default class for creating entries |

## Events (Delegates)

### OnWidgetAdded

Fired when an entry is added to the scroll box.

```cpp
UPROPERTY(BlueprintAssignable, Category = "ScrollBox|Events")
FOnScrollBoxWidgetAdded OnWidgetAdded;

// Signature: (UScrollBoxEntryWidget* Widget, int32 Index)
```

### OnWidgetRemoved

Fired when an entry is removed from the scroll box.

```cpp
UPROPERTY(BlueprintAssignable, Category = "ScrollBox|Events")
FOnScrollBoxWidgetRemoved OnWidgetRemoved;

// Signature: (UScrollBoxEntryWidget* Widget, int32 Index)
```

### OnWidgetSelected

Fired when an entry is selected.

```cpp
UPROPERTY(BlueprintAssignable, Category = "ScrollBox|Events")
FOnScrollBoxWidgetSelected OnWidgetSelected;

// Signature: (UScrollBoxEntryWidget* Widget, int32 Index)
```

### OnScrollBoxUpdated

Fired after scroll box content is updated.

```cpp
UPROPERTY(BlueprintAssignable, Category = "ScrollBox|Events")
FOnScrollBoxUpdated OnScrollBoxUpdated;

// Signature: (int32 WidgetCount)
```

### OnSelectionChanged

Fired when the selected entry changes.

```cpp
UPROPERTY(BlueprintAssignable, Category = "ScrollBox|Events")
FOnScrollBoxSelectionChanged OnSelectionChanged;

// Signature: (UScrollBoxEntryWidget* NewSelection, UScrollBoxEntryWidget* OldSelection)
```

## Adding Widgets

### AddEntry

Add an existing entry widget to the end.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add")
int32 AddEntry(UScrollBoxEntryWidget* Entry);
```

**Returns:** Index where added, or -1 if failed.

### AddEntryAtIndex

Add an entry at a specific position.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add")
int32 AddEntryAtIndex(UScrollBoxEntryWidget* Entry, int32 Index);
```

### CreateAndAddEntry

Create and add a new entry of specified class.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add")
UScrollBoxEntryWidget* CreateAndAddEntry(TSubclassOf<UScrollBoxEntryWidget> EntryClass);
```

### CreateAndAddDefaultEntry

Create and add using the default entry class.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add")
UScrollBoxEntryWidget* CreateAndAddDefaultEntry();
```

### CreateAndAddEntryWithData

Create entry with data object bound.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add")
UScrollBoxEntryWidget* CreateAndAddEntryWithData(
    TSubclassOf<UScrollBoxEntryWidget> EntryClass,
    UObject* Data);
```

### AddEntries

Add multiple entries at once.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Add")
int32 AddEntries(const TArray<UScrollBoxEntryWidget*>& Entries);
```

## Removing Widgets

### RemoveEntry

Remove a specific entry.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Remove")
bool RemoveEntry(UScrollBoxEntryWidget* Entry);
```

### RemoveEntryAtIndex

Remove entry at index.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Remove")
UScrollBoxEntryWidget* RemoveEntryAtIndex(int32 Index);
```

### ClearAllEntries

Remove all entries.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Remove")
int32 ClearAllEntries(bool bDestroyWidgets = false);
```

### RemoveEntriesWhere

Remove entries matching a predicate.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Remove")
int32 RemoveEntriesWhere(FScrollBoxFilterPredicate Predicate);
```

### RemoveEntriesByTag

Remove all entries with a specific tag.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Remove")
int32 RemoveEntriesByTag(FName Tag);
```

## Accessing Widgets

### GetEntryAtIndex

```cpp
UFUNCTION(BlueprintPure, Category = "ScrollBox|Access")
UScrollBoxEntryWidget* GetEntryAtIndex(int32 Index) const;
```

### GetAllEntries

```cpp
UFUNCTION(BlueprintPure, Category = "ScrollBox|Access")
TArray<UScrollBoxEntryWidget*> GetAllEntries() const;
```

### GetEntryCount

```cpp
UFUNCTION(BlueprintPure, Category = "ScrollBox|Access")
int32 GetEntryCount() const;
```

### IsEmpty

```cpp
UFUNCTION(BlueprintPure, Category = "ScrollBox|Access")
bool IsEmpty() const;
```

### GetEntryIndex

```cpp
UFUNCTION(BlueprintPure, Category = "ScrollBox|Access")
int32 GetEntryIndex(UScrollBoxEntryWidget* Entry) const;
```

### ContainsEntry

```cpp
UFUNCTION(BlueprintPure, Category = "ScrollBox|Access")
bool ContainsEntry(UScrollBoxEntryWidget* Entry) const;
```

### FindEntry / FindEntries

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Access")
UScrollBoxEntryWidget* FindEntry(FScrollBoxFilterPredicate Predicate) const;

UFUNCTION(BlueprintCallable, Category = "ScrollBox|Access")
TArray<UScrollBoxEntryWidget*> FindEntries(FScrollBoxFilterPredicate Predicate) const;
```

### FindEntriesByTag

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Access")
TArray<UScrollBoxEntryWidget*> FindEntriesByTag(FName Tag) const;
```

### GetFirstEntry / GetLastEntry

```cpp
UFUNCTION(BlueprintPure, Category = "ScrollBox|Access")
UScrollBoxEntryWidget* GetFirstEntry() const;

UFUNCTION(BlueprintPure, Category = "ScrollBox|Access")
UScrollBoxEntryWidget* GetLastEntry() const;
```

## Selection Management

### SelectEntry

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection")
void SelectEntry(UScrollBoxEntryWidget* Entry, bool bNotify = true);
```

### SelectEntryAtIndex

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection")
void SelectEntryAtIndex(int32 Index, bool bNotify = true);
```

### DeselectEntry

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection")
void DeselectEntry(UScrollBoxEntryWidget* Entry);
```

### ClearSelection

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection")
void ClearSelection();
```

### GetSelectedEntry / GetSelectedEntries

```cpp
UFUNCTION(BlueprintPure, Category = "ScrollBox|Selection")
UScrollBoxEntryWidget* GetSelectedEntry() const;

UFUNCTION(BlueprintPure, Category = "ScrollBox|Selection")
TArray<UScrollBoxEntryWidget*> GetSelectedEntries() const;
```

### GetSelectedIndex

```cpp
UFUNCTION(BlueprintPure, Category = "ScrollBox|Selection")
int32 GetSelectedIndex() const;
```

### HasSelection

```cpp
UFUNCTION(BlueprintPure, Category = "ScrollBox|Selection")
bool HasSelection() const;
```

### SelectNextEntry / SelectPreviousEntry

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection")
void SelectNextEntry(bool bWrap = true);

UFUNCTION(BlueprintCallable, Category = "ScrollBox|Selection")
void SelectPreviousEntry(bool bWrap = true);
```

## Sorting and Organization

### SortByPriority

Sort entries by their `SortPriority` property.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Sort")
void SortByPriority(EScrollBoxSortOrder Order = EScrollBoxSortOrder::Ascending);
```

### SortEntries

Sort using custom comparison.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Sort")
void SortEntries(FScrollBoxSortPredicate Predicate);
```

### MoveEntry

Move an entry to a new index.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Sort")
bool MoveEntry(UScrollBoxEntryWidget* Entry, int32 NewIndex);
```

### SwapEntries

Swap two entries by index.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Sort")
bool SwapEntries(int32 IndexA, int32 IndexB);
```

### ReverseOrder

Reverse the order of all entries.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Sort")
void ReverseOrder();
```

## Scrolling

### ScrollToEntry / ScrollToIndex

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Scroll")
void ScrollToEntry(UScrollBoxEntryWidget* Entry, bool bAnimated = true);

UFUNCTION(BlueprintCallable, Category = "ScrollBox|Scroll")
void ScrollToIndex(int32 Index, bool bAnimated = true);
```

### ScrollToTop / ScrollToBottom

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Scroll")
void ScrollToTop(bool bAnimated = true);

UFUNCTION(BlueprintCallable, Category = "ScrollBox|Scroll")
void ScrollToBottom(bool bAnimated = true);
```

### SetScrollOffset / GetScrollOffset

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Scroll")
void SetScrollOffset(float Offset);

UFUNCTION(BlueprintPure, Category = "ScrollBox|Scroll")
float GetScrollOffset() const;
```

## Update and Refresh

### RefreshEntryIndices

Update indices of all entries.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Update")
void RefreshEntryIndices();
```

### ForceRefresh

Force visual refresh.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Update")
void ForceRefresh();
```

### RegenerateContent

Rebuild scroll box content from internal array.

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Update")
void RegenerateContent();
```

## Focus and Navigation

### FocusFirstEntry

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Focus")
void FocusFirstEntry();
```

### FocusEntry / FocusEntryAtIndex

```cpp
UFUNCTION(BlueprintCallable, Category = "ScrollBox|Focus")
void FocusEntry(UScrollBoxEntryWidget* Entry);

UFUNCTION(BlueprintCallable, Category = "ScrollBox|Focus")
void FocusEntryAtIndex(int32 Index);
```

## Blueprint Implementable Events

### BP_OnEntryAdding

Called before an entry is added. Return `false` to cancel.

```cpp
UFUNCTION(BlueprintNativeEvent, Category = "ScrollBox|Events")
bool BP_OnEntryAdding(UScrollBoxEntryWidget* Entry, int32 Index);
```

### BP_OnEntryAdded

Called after an entry is added.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "ScrollBox|Events")
void BP_OnEntryAdded(UScrollBoxEntryWidget* Entry, int32 Index);
```

### BP_OnEntryRemoving

Called before an entry is removed.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "ScrollBox|Events")
void BP_OnEntryRemoving(UScrollBoxEntryWidget* Entry);
```

### BP_OnContentCleared

Called after all content is cleared.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "ScrollBox|Events")
void BP_OnContentCleared();
```

## Usage Examples

### Creating a Blueprint Subclass

1. Create a new Blueprint class inheriting from `CommonScrollBoxManager`
2. In the UMG designer, add a `ScrollBox` or `CommonHierarchicalScrollBox` widget
3. **Important**: Name the scroll box widget `"ScrollBox"` (or `"EntryScrollBox"` or `"ContentScrollBox"`)
4. Set `DefaultEntryClass` to your entry widget Blueprint class

### Adding Entries in Blueprint

```
Event BeginPlay:
    -> Create And Add Entry With Data (MyEntryClass, DataObject)
    -> Create And Add Entry With Data (MyEntryClass, DataObject2)
    -> Sort By Priority (Ascending)
```

### C++ Usage

```cpp
// Get reference to manager widget
UCommonScrollBoxManager* Manager = FindWidget<UCommonScrollBoxManager>(TEXT("MyScrollBoxManager"));

// Add entries with data
for (UObject* DataItem : DataArray)
{
    UScrollBoxEntryWidget* Entry = Manager->CreateAndAddEntryWithData(
        UMyEntryWidget::StaticClass(),
        DataItem
    );
    Entry->SortPriority = DataItem->GetPriority();
}

// Sort by priority
Manager->SortByPriority(EScrollBoxSortOrder::Ascending);

// Bind to selection event
Manager->OnWidgetSelected.AddDynamic(this, &AMyController::HandleSelection);
```

### Filtering Entries

```cpp
// Find entries with specific tag
TArray<UScrollBoxEntryWidget*> ImportantEntries = Manager->FindEntriesByTag(FName("Important"));

// Remove disabled entries
Manager->RemoveEntriesWhere(FScrollBoxFilterPredicate::CreateLambda(
    [](UScrollBoxEntryWidget* Entry) -> bool
    {
        return !Entry->bIsEntryEnabled;
    }
));
```

## Widget Binding

The manager automatically looks for a scroll box widget with these names (in order):
1. `"ScrollBox"`
2. `"EntryScrollBox"`
3. `"ContentScrollBox"`

The widget must be a `UPanelWidget` (like `UScrollBox` or `UCommonHierarchicalScrollBox`).

## Navigation Keys

When `bEnableNavigation` is true:

| Key | Action |
|-----|--------|
| Up / DPad Up | Select previous entry |
| Down / DPad Down | Select next entry |
| Home | Select first entry |
| End | Select last entry |

## Related Classes

- [`UScrollBoxEntryWidget`](ScrollBoxEntryWidget.md) - Base class for entry widgets

---

*Copyright 2025 AI27. All Rights Reserved.*
*Designer: Aldo Maradon Duran Bautista*
