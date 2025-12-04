# ScrollBoxEntryWidget

## Overview

`UScrollBoxEntryWidget` is the base class for entry widgets displayed in the `UCommonScrollBoxManager`. It provides selection state, data binding, and Blueprint-extensible events.

**File Location:** `Source/ai27Simulator/Public/UI/ScrollBoxEntryWidget.h`
**Implementation:** `Source/ai27Simulator/Private/UI/ScrollBoxEntryWidget.cpp`

## Class Declaration

```cpp
UCLASS(Abstract, Blueprintable, BlueprintType, meta = (DisableNativeTick))
class AI27SIMULATOR_API UScrollBoxEntryWidget : public UCommonActivatableWidget
```

**Note:** This class is `Abstract` - you must create a Blueprint subclass to use it.

## Features

- **Selection State**: Track and respond to selection changes
- **Index Tracking**: Know position within parent scroll box
- **Data Binding**: Bind UObject or struct data to entries
- **Tag System**: Categorize entries for filtering
- **Sort Priority**: Control ordering in sorted lists
- **Blueprint Events**: Override behavior in Blueprint

## Properties

### State Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `EntryIndex` | `int32` | INDEX_NONE | Current index in scroll box (read-only) |
| `bIsSelected` | `bool` | false | Is currently selected (read-only) |
| `bIsEntryEnabled` | `bool` | true | Can be interacted with |

### Data Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `EntryTag` | `FName` | NAME_None | Tag for filtering/categorization |
| `SortPriority` | `int32` | 0 | Priority for sorting (lower = first) |

## Events (Delegates)

### OnSelectionChanged

Fired when the selection state changes.

```cpp
UPROPERTY(BlueprintAssignable, Category = "Entry|Events")
FOnEntrySelectionChanged OnSelectionChanged;

// Signature: (UScrollBoxEntryWidget* Entry, bool bIsSelected)
```

### OnClicked

Fired when the entry is clicked or activated.

```cpp
UPROPERTY(BlueprintAssignable, Category = "Entry|Events")
FOnEntryClicked OnClicked;

// Signature: (UScrollBoxEntryWidget* Entry)
```

## Data Binding Functions

### SetEntryData

Bind a UObject to this entry.

```cpp
UFUNCTION(BlueprintCallable, Category = "Entry|Data")
virtual void SetEntryData(UObject* InData);
```

This function:
1. Stores the data object
2. Calls `BP_OnDataSet` for Blueprint handling

### GetEntryData

Get the bound data object.

```cpp
UFUNCTION(BlueprintPure, Category = "Entry|Data")
UObject* GetEntryData() const;
```

### SetEntryDataFromStruct

Bind struct data (for Blueprint usage).

```cpp
UFUNCTION(BlueprintCallable, Category = "Entry|Data", CustomThunk)
void SetEntryDataFromStruct(const int32& StructData);
```

**Note:** This uses CustomThunk to accept any struct type from Blueprint.

### HasData

Check if data is bound.

```cpp
UFUNCTION(BlueprintPure, Category = "Entry|Data")
bool HasData() const;
```

## State Management Functions

### SetSelected

Change the selection state.

```cpp
UFUNCTION(BlueprintCallable, Category = "Entry|State")
virtual void SetSelected(bool bNewSelected);
```

This function:
1. Updates `bIsSelected`
2. Broadcasts `OnSelectionChanged`
3. Calls `BP_OnSelectionChanged`

### ToggleSelection

Toggle between selected/deselected.

```cpp
UFUNCTION(BlueprintCallable, Category = "Entry|State")
void ToggleSelection();
```

### SetEntryIndex

Set the entry's index (usually called by manager).

```cpp
UFUNCTION(BlueprintCallable, Category = "Entry|State")
void SetEntryIndex(int32 NewIndex);
```

### GetOwningManager

Get the parent scroll box manager.

```cpp
UFUNCTION(BlueprintPure, Category = "Entry|State")
UCommonScrollBoxManager* GetOwningManager() const;
```

## Query Functions

### HasTag

Check if entry has a specific tag.

```cpp
UFUNCTION(BlueprintPure, Category = "Entry|Data")
bool HasTag(FName Tag) const;
```

## Blueprint Implementable Events

Override these in your Blueprint subclass:

### BP_OnDataSet

Called when `SetEntryData` is called. Use this to populate your UI elements.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events")
void BP_OnDataSet(UObject* Data);
```

**Example Implementation:**
```
Event BP_OnDataSet (Data):
    -> Cast To MyDataClass
    -> Set Text (NameText, Data.Name)
    -> Set Image (IconImage, Data.Icon)
```

### BP_OnSelectionChanged

Called when selection state changes. Use for visual feedback.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events")
void BP_OnSelectionChanged(bool bSelected);
```

**Example Implementation:**
```
Event BP_OnSelectionChanged (bSelected):
    -> Branch (bSelected)
        True -> Set Brush Color (Background, Selected Color)
        False -> Set Brush Color (Background, Normal Color)
```

### BP_OnClicked

Called when the entry is clicked or activated.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events")
void BP_OnClicked();
```

### BP_OnIndexChanged

Called when the entry's index changes.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events")
void BP_OnIndexChanged(int32 NewIndex);
```

### BP_OnAddedToScrollBox

Called when entry is added to a scroll box.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events")
void BP_OnAddedToScrollBox();
```

### BP_OnRemovingFromScrollBox

Called just before entry is removed from scroll box.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Entry|Events")
void BP_OnRemovingFromScrollBox();
```

## Creating a Blueprint Entry Widget

### Step 1: Create Blueprint Class

1. Right-click in Content Browser
2. Select **Blueprint Class**
3. Search for `ScrollBoxEntryWidget` as parent
4. Name it (e.g., `WBP_MyEntry`)

### Step 2: Design the Widget

1. Open the widget Blueprint
2. In the Designer tab, create your visual layout:
   - Add a Border or Image for background
   - Add Text blocks for data display
   - Add any buttons or interactive elements

### Step 3: Implement BP_OnDataSet

1. Go to the Graph tab
2. Override `BP_OnDataSet`
3. Cast `Data` to your data class
4. Populate UI elements with data

```
Event BP_OnDataSet (Data):
    -> Cast To UMyItemData (Data)
        -> Set Text (ItemNameText, AsMyItemData.ItemName)
        -> Set Text (ItemDescText, AsMyItemData.Description)
        -> Set Image Brush (ItemIcon, AsMyItemData.IconTexture)
        -> Set (SortPriority, AsMyItemData.Priority)
```

### Step 4: Implement Selection Feedback

Override `BP_OnSelectionChanged`:

```
Event BP_OnSelectionChanged (bSelected):
    -> Select (bSelected):
        True:  Set Color (BackgroundBorder, (0.2, 0.4, 0.8, 1.0))
        False: Set Color (BackgroundBorder, (0.1, 0.1, 0.1, 1.0))
```

### Step 5: Handle Clicks (Optional)

If you have a button in your entry:

```
Event OnButtonClicked:
    -> Handle Clicked (call internal function)
```

## C++ Subclass Example

```cpp
UCLASS()
class UMyGameEntryWidget : public UScrollBoxEntryWidget
{
    GENERATED_BODY()

public:
    virtual void SetEntryData(UObject* InData) override
    {
        Super::SetEntryData(InData);

        if (UMyItemData* ItemData = Cast<UMyItemData>(InData))
        {
            // Update UI directly in C++
            if (ItemNameText)
            {
                ItemNameText->SetText(FText::FromString(ItemData->ItemName));
            }

            // Set sort priority
            SortPriority = ItemData->Priority;
        }
    }

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ItemNameText;

    UPROPERTY(meta = (BindWidget))
    UImage* ItemIcon;
};
```

## Data Class Example

```cpp
UCLASS(BlueprintType)
class UMyItemData : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* IconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority;
};
```

## Focus Behavior

The entry widget handles focus events from CommonUI:

- **OnFocusReceived**: Entry receives focus (keyboard/gamepad navigation)
- **OnFocusLost**: Entry loses focus

You can customize focus behavior in your Blueprint subclass.

## Best Practices

1. **Keep Entries Lightweight**: Avoid complex logic in entries
2. **Use Data Objects**: Separate data from presentation
3. **Implement Visual Feedback**: Always show selection state clearly
4. **Set Sort Priority**: Set priority in `BP_OnDataSet` for proper sorting
5. **Use Tags**: Tag entries for easy filtering

## Related Classes

- [`UCommonScrollBoxManager`](CommonScrollBoxManager.md) - Parent container

---

*Copyright 2025 AI27. All Rights Reserved.*
*Designer: Aldo Maradon Duran Bautista*
