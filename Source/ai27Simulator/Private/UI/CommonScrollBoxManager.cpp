// Copyright 2025 AI27. All Rights Reserved.
// Designer: Aldo Maradon Durán Bautista
// Project: AI27 Simulator

#include "UI/CommonScrollBoxManager.h"
#include "UI/ScrollBoxEntryWidget.h"
#include "CommonHierarchicalScrollBox.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetTree.h"

UCommonScrollBoxManager::UCommonScrollBoxManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SelectionMode(EScrollBoxSelectionMode::Single)
	, bEnableNavigation(true)
	, bAutoScrollToNewEntries(false)
	, bAutoScrollToSelection(true)
	, DefaultEntryClass(nullptr)
	, ScrollBox(nullptr)
	, CurrentSelection(nullptr)
	, ActiveScrollBoxWidget(nullptr)
{
	bAutoActivate = true;
}

void UCommonScrollBoxManager::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Try to resolve scroll box early
	if (ScrollBox)
	{
		ActiveScrollBoxWidget = ScrollBox;
		UE_LOG(LogTemp, Log, TEXT("CommonScrollBoxManager::NativePreConstruct - ScrollBox already bound: %s"),
			*ScrollBox->GetName());
	}
}

void UCommonScrollBoxManager::NativeConstruct()
{
	Super::NativeConstruct();

	// If ScrollBox property is set (via BindWidget or manually), use it
	if (ScrollBox)
	{
		ActiveScrollBoxWidget = ScrollBox;
		UE_LOG(LogTemp, Log, TEXT("CommonScrollBoxManager: Using bound ScrollBox widget '%s'"),
			*ScrollBox->GetName());
	}
	else
	{
		// Try to auto-find scroll box
		TryAutoBindScrollBox();
	}

	// Final check
	if (ActiveScrollBoxWidget)
	{
		UE_LOG(LogTemp, Log, TEXT("CommonScrollBoxManager: Ready with scroll box '%s'"),
			*ActiveScrollBoxWidget->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CommonScrollBoxManager: NO SCROLL BOX FOUND! "
			"Entries will be added to internal array but NOT displayed. "
			"Please ensure you have a PanelWidget named 'ScrollBox' in your widget, "
			"or manually call SetScrollBoxWidget()."));
	}
}

void UCommonScrollBoxManager::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Focus first entry if navigation enabled
	if (bEnableNavigation && ManagedEntries.Num() > 0)
	{
		FocusFirstEntry();
	}
}

FReply UCommonScrollBoxManager::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bEnableNavigation)
	{
		FKey Key = InKeyEvent.GetKey();

		// Handle navigation keys
		if (Key == EKeys::Up || Key == EKeys::Gamepad_DPad_Up)
		{
			SelectPreviousEntry(true);
			return FReply::Handled();
		}
		else if (Key == EKeys::Down || Key == EKeys::Gamepad_DPad_Down)
		{
			SelectNextEntry(true);
			return FReply::Handled();
		}
		else if (Key == EKeys::Home)
		{
			SelectEntryAtIndex(0, true);
			return FReply::Handled();
		}
		else if (Key == EKeys::End)
		{
			SelectEntryAtIndex(ManagedEntries.Num() - 1, true);
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UCommonScrollBoxManager::TryAutoBindScrollBox()
{
	// Already have one?
	if (ActiveScrollBoxWidget)
	{
		return;
	}

	// Try WidgetTree first
	if (WidgetTree)
	{
		// Try common names
		static const FName ScrollBoxNames[] = {
			FName("ScrollBox"),
			FName("EntryScrollBox"),
			FName("ContentScrollBox"),
			FName("ItemScrollBox"),
			FName("ListScrollBox")
		};

		for (const FName& Name : ScrollBoxNames)
		{
			UWidget* FoundWidget = WidgetTree->FindWidget(Name);
			if (FoundWidget)
			{
				UPanelWidget* Panel = Cast<UPanelWidget>(FoundWidget);
				if (Panel)
				{
					ActiveScrollBoxWidget = Panel;
					UE_LOG(LogTemp, Log, TEXT("CommonScrollBoxManager: Auto-found scroll box '%s' via WidgetTree"),
						*Name.ToString());
					return;
				}
			}
		}

		// If not found by name, search all widgets for a ScrollBox type
		WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			if (!ActiveScrollBoxWidget)
			{
				if (UScrollBox* SB = Cast<UScrollBox>(Widget))
				{
					ActiveScrollBoxWidget = SB;
					UE_LOG(LogTemp, Log, TEXT("CommonScrollBoxManager: Auto-found ScrollBox widget '%s' by type"),
						*Widget->GetName());
				}
			}
		});
	}

	if (!ActiveScrollBoxWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("CommonScrollBoxManager::TryAutoBindScrollBox - Could not find any scroll box widget"));
	}
}

void UCommonScrollBoxManager::SetScrollBoxWidget(UPanelWidget* InScrollBox)
{
	if (InScrollBox)
	{
		ActiveScrollBoxWidget = InScrollBox;
		ScrollBox = InScrollBox;
		UE_LOG(LogTemp, Log, TEXT("CommonScrollBoxManager: Manually set scroll box to '%s'"),
			*InScrollBox->GetName());

		// If we already have entries, regenerate content
		if (ManagedEntries.Num() > 0)
		{
			RegenerateContent();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CommonScrollBoxManager::SetScrollBoxWidget - Received null widget"));
	}
}

UPanelWidget* UCommonScrollBoxManager::GetScrollBoxWidget() const
{
	return ActiveScrollBoxWidget;
}

// ========================================
// Adding Widgets
// ========================================

int32 UCommonScrollBoxManager::AddEntry(UScrollBoxEntryWidget* Entry)
{
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("CommonScrollBoxManager: Cannot add null entry"));
		return INDEX_NONE;
	}

	int32 NewIndex = ManagedEntries.Num();

	// Check if Blueprint wants to cancel
	if (!BP_OnEntryAdding(Entry, NewIndex))
	{
		return INDEX_NONE;
	}

	// Add to internal array
	ManagedEntries.Add(Entry);

	// Setup entry
	Entry->SetOwningManager(this);
	Entry->SetEntryIndex(NewIndex);

	// Bind to entry events
	Entry->OnClicked.AddDynamic(this, &UCommonScrollBoxManager::HandleEntryClicked);
	Entry->OnSelectionChanged.AddDynamic(this, &UCommonScrollBoxManager::HandleEntrySelectionChanged);

	// Add to scroll box widget
	AddEntryToScrollBox(Entry);

	// Notify
	OnWidgetAdded.Broadcast(Entry, NewIndex);
	BP_OnEntryAdded(Entry, NewIndex);
	OnScrollBoxUpdated.Broadcast(ManagedEntries.Num());

	// Auto scroll if enabled
	if (bAutoScrollToNewEntries)
	{
		ScrollToEntry(Entry, true);
	}

	UE_LOG(LogTemp, Log, TEXT("CommonScrollBoxManager: Added entry at index %d (total: %d)"),
		NewIndex, ManagedEntries.Num());

	return NewIndex;
}

int32 UCommonScrollBoxManager::AddEntryAtIndex(UScrollBoxEntryWidget* Entry, int32 Index)
{
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("CommonScrollBoxManager: Cannot add null entry"));
		return INDEX_NONE;
	}

	// Clamp index to valid range
	Index = FMath::Clamp(Index, 0, ManagedEntries.Num());

	// Check if Blueprint wants to cancel
	if (!BP_OnEntryAdding(Entry, Index))
	{
		return INDEX_NONE;
	}

	// Insert into array
	ManagedEntries.Insert(Entry, Index);

	// Setup entry
	Entry->SetOwningManager(this);

	// Bind to entry events
	Entry->OnClicked.AddDynamic(this, &UCommonScrollBoxManager::HandleEntryClicked);
	Entry->OnSelectionChanged.AddDynamic(this, &UCommonScrollBoxManager::HandleEntrySelectionChanged);

	// Update all indices
	UpdateEntryIndices();

	// Regenerate scroll box content (needed for insertion)
	RegenerateContent();

	// Notify
	OnWidgetAdded.Broadcast(Entry, Index);
	BP_OnEntryAdded(Entry, Index);
	OnScrollBoxUpdated.Broadcast(ManagedEntries.Num());

	if (bAutoScrollToNewEntries)
	{
		ScrollToEntry(Entry, true);
	}

	UE_LOG(LogTemp, Log, TEXT("CommonScrollBoxManager: Inserted entry at index %d (total: %d)"),
		Index, ManagedEntries.Num());

	return Index;
}

UScrollBoxEntryWidget* UCommonScrollBoxManager::CreateAndAddEntry(TSubclassOf<UScrollBoxEntryWidget> EntryClass)
{
	if (!EntryClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CommonScrollBoxManager: Entry class is null"));
		return nullptr;
	}

	// Create widget
	UScrollBoxEntryWidget* NewEntry = CreateWidget<UScrollBoxEntryWidget>(this, EntryClass);
	if (NewEntry)
	{
		AddEntry(NewEntry);
	}

	return NewEntry;
}

UScrollBoxEntryWidget* UCommonScrollBoxManager::CreateAndAddDefaultEntry()
{
	if (!DefaultEntryClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CommonScrollBoxManager: DefaultEntryClass is not set"));
		return nullptr;
	}

	return CreateAndAddEntry(DefaultEntryClass);
}

UScrollBoxEntryWidget* UCommonScrollBoxManager::CreateAndAddEntryWithData(TSubclassOf<UScrollBoxEntryWidget> EntryClass, UObject* Data)
{
	UScrollBoxEntryWidget* NewEntry = CreateAndAddEntry(EntryClass);
	if (NewEntry && Data)
	{
		NewEntry->SetEntryData(Data);
	}
	return NewEntry;
}

int32 UCommonScrollBoxManager::AddEntries(const TArray<UScrollBoxEntryWidget*>& Entries)
{
	int32 AddedCount = 0;

	for (UScrollBoxEntryWidget* Entry : Entries)
	{
		if (AddEntry(Entry) != INDEX_NONE)
		{
			AddedCount++;
		}
	}

	return AddedCount;
}

// ========================================
// Removing Widgets
// ========================================

bool UCommonScrollBoxManager::RemoveEntry(UScrollBoxEntryWidget* Entry)
{
	if (!Entry)
	{
		return false;
	}

	int32 Index = ManagedEntries.Find(Entry);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	// Notify Blueprint
	BP_OnEntryRemoving(Entry);

	// If selected, clear selection
	if (CurrentSelection == Entry)
	{
		CurrentSelection = nullptr;
	}
	SelectedEntries.Remove(Entry);

	// Unbind events
	Entry->OnClicked.RemoveDynamic(this, &UCommonScrollBoxManager::HandleEntryClicked);
	Entry->OnSelectionChanged.RemoveDynamic(this, &UCommonScrollBoxManager::HandleEntrySelectionChanged);

	// Remove from scroll box
	RemoveEntryFromScrollBox(Entry);

	// Remove from array
	ManagedEntries.RemoveAt(Index);

	// Clear owner reference
	Entry->SetOwningManager(nullptr);

	// Update indices
	UpdateEntryIndices();

	// Notify
	OnWidgetRemoved.Broadcast(Entry, Index);
	OnScrollBoxUpdated.Broadcast(ManagedEntries.Num());

	UE_LOG(LogTemp, Log, TEXT("CommonScrollBoxManager: Removed entry at index %d (total: %d)"),
		Index, ManagedEntries.Num());

	return true;
}

UScrollBoxEntryWidget* UCommonScrollBoxManager::RemoveEntryAtIndex(int32 Index)
{
	if (!ManagedEntries.IsValidIndex(Index))
	{
		return nullptr;
	}

	UScrollBoxEntryWidget* Entry = ManagedEntries[Index];
	RemoveEntry(Entry);
	return Entry;
}

int32 UCommonScrollBoxManager::ClearAllEntries(bool bDestroyWidgets)
{
	int32 RemovedCount = ManagedEntries.Num();

	// Clear selection first
	ClearSelection();

	// Remove all entries from scroll box
	for (UScrollBoxEntryWidget* Entry : ManagedEntries)
	{
		if (Entry)
		{
			Entry->OnClicked.RemoveDynamic(this, &UCommonScrollBoxManager::HandleEntryClicked);
			Entry->OnSelectionChanged.RemoveDynamic(this, &UCommonScrollBoxManager::HandleEntrySelectionChanged);
			RemoveEntryFromScrollBox(Entry);
			Entry->SetOwningManager(nullptr);

			if (bDestroyWidgets)
			{
				Entry->RemoveFromParent();
			}
		}
	}

	ManagedEntries.Empty();

	// Notify
	BP_OnContentCleared();
	OnScrollBoxUpdated.Broadcast(0);

	UE_LOG(LogTemp, Log, TEXT("CommonScrollBoxManager: Cleared all %d entries"), RemovedCount);

	return RemovedCount;
}

int32 UCommonScrollBoxManager::RemoveEntriesWhere(FScrollBoxFilterPredicate Predicate)
{
	if (!Predicate.IsBound())
	{
		return 0;
	}

	TArray<UScrollBoxEntryWidget*> ToRemove;

	for (UScrollBoxEntryWidget* Entry : ManagedEntries)
	{
		if (Entry && Predicate.Execute(Entry))
		{
			ToRemove.Add(Entry);
		}
	}

	for (UScrollBoxEntryWidget* Entry : ToRemove)
	{
		RemoveEntry(Entry);
	}

	return ToRemove.Num();
}

int32 UCommonScrollBoxManager::RemoveEntriesByTag(FName Tag)
{
	TArray<UScrollBoxEntryWidget*> ToRemove;

	for (UScrollBoxEntryWidget* Entry : ManagedEntries)
	{
		if (Entry && Entry->HasTag(Tag))
		{
			ToRemove.Add(Entry);
		}
	}

	for (UScrollBoxEntryWidget* Entry : ToRemove)
	{
		RemoveEntry(Entry);
	}

	return ToRemove.Num();
}

// ========================================
// Accessing Widgets
// ========================================

UScrollBoxEntryWidget* UCommonScrollBoxManager::GetEntryAtIndex(int32 Index) const
{
	if (ManagedEntries.IsValidIndex(Index))
	{
		return ManagedEntries[Index];
	}
	return nullptr;
}

TArray<UScrollBoxEntryWidget*> UCommonScrollBoxManager::GetAllEntries() const
{
	TArray<UScrollBoxEntryWidget*> Result;
	for (UScrollBoxEntryWidget* Entry : ManagedEntries)
	{
		if (Entry)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

int32 UCommonScrollBoxManager::GetEntryCount() const
{
	return ManagedEntries.Num();
}

bool UCommonScrollBoxManager::IsEmpty() const
{
	return ManagedEntries.Num() == 0;
}

int32 UCommonScrollBoxManager::GetEntryIndex(UScrollBoxEntryWidget* Entry) const
{
	return Entry ? ManagedEntries.Find(Entry) : INDEX_NONE;
}

bool UCommonScrollBoxManager::ContainsEntry(UScrollBoxEntryWidget* Entry) const
{
	return Entry && ManagedEntries.Contains(Entry);
}

UScrollBoxEntryWidget* UCommonScrollBoxManager::FindEntry(FScrollBoxFilterPredicate Predicate) const
{
	if (!Predicate.IsBound())
	{
		return nullptr;
	}

	for (UScrollBoxEntryWidget* Entry : ManagedEntries)
	{
		if (Entry && Predicate.Execute(Entry))
		{
			return Entry;
		}
	}
	return nullptr;
}

TArray<UScrollBoxEntryWidget*> UCommonScrollBoxManager::FindEntries(FScrollBoxFilterPredicate Predicate) const
{
	TArray<UScrollBoxEntryWidget*> Result;

	if (!Predicate.IsBound())
	{
		return Result;
	}

	for (UScrollBoxEntryWidget* Entry : ManagedEntries)
	{
		if (Entry && Predicate.Execute(Entry))
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

TArray<UScrollBoxEntryWidget*> UCommonScrollBoxManager::FindEntriesByTag(FName Tag) const
{
	TArray<UScrollBoxEntryWidget*> Result;

	for (UScrollBoxEntryWidget* Entry : ManagedEntries)
	{
		if (Entry && Entry->HasTag(Tag))
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

UScrollBoxEntryWidget* UCommonScrollBoxManager::GetFirstEntry() const
{
	return ManagedEntries.Num() > 0 ? ManagedEntries[0] : nullptr;
}

UScrollBoxEntryWidget* UCommonScrollBoxManager::GetLastEntry() const
{
	return ManagedEntries.Num() > 0 ? ManagedEntries.Last() : nullptr;
}

// ========================================
// Selection Management
// ========================================

void UCommonScrollBoxManager::SelectEntry(UScrollBoxEntryWidget* Entry, bool bNotify)
{
	if (SelectionMode == EScrollBoxSelectionMode::None)
	{
		return;
	}

	if (!Entry || !ContainsEntry(Entry))
	{
		return;
	}

	UScrollBoxEntryWidget* OldSelection = CurrentSelection;

	if (SelectionMode == EScrollBoxSelectionMode::Single)
	{
		// Deselect previous
		if (CurrentSelection && CurrentSelection != Entry)
		{
			CurrentSelection->SetSelected(false);
		}

		CurrentSelection = Entry;
		Entry->SetSelected(true);
	}
	else if (SelectionMode == EScrollBoxSelectionMode::Multi)
	{
		if (!SelectedEntries.Contains(Entry))
		{
			SelectedEntries.Add(Entry);
		}
		Entry->SetSelected(true);
		CurrentSelection = Entry;
	}

	// Auto-scroll to selection
	if (bAutoScrollToSelection)
	{
		ScrollToEntry(Entry, true);
	}

	// Notify
	if (bNotify)
	{
		OnWidgetSelected.Broadcast(Entry, Entry->EntryIndex);
		OnSelectionChanged.Broadcast(Entry, OldSelection);
	}
}

void UCommonScrollBoxManager::SelectEntryAtIndex(int32 Index, bool bNotify)
{
	UScrollBoxEntryWidget* Entry = GetEntryAtIndex(Index);
	if (Entry)
	{
		SelectEntry(Entry, bNotify);
	}
}

void UCommonScrollBoxManager::DeselectEntry(UScrollBoxEntryWidget* Entry)
{
	if (!Entry)
	{
		return;
	}

	Entry->SetSelected(false);

	if (CurrentSelection == Entry)
	{
		CurrentSelection = nullptr;
	}

	SelectedEntries.Remove(Entry);
}

void UCommonScrollBoxManager::ClearSelection()
{
	if (SelectionMode == EScrollBoxSelectionMode::Single)
	{
		if (CurrentSelection)
		{
			CurrentSelection->SetSelected(false);
			CurrentSelection = nullptr;
		}
	}
	else if (SelectionMode == EScrollBoxSelectionMode::Multi)
	{
		for (UScrollBoxEntryWidget* Entry : SelectedEntries)
		{
			if (Entry)
			{
				Entry->SetSelected(false);
			}
		}
		SelectedEntries.Empty();
		CurrentSelection = nullptr;
	}
}

UScrollBoxEntryWidget* UCommonScrollBoxManager::GetSelectedEntry() const
{
	return CurrentSelection;
}

TArray<UScrollBoxEntryWidget*> UCommonScrollBoxManager::GetSelectedEntries() const
{
	TArray<UScrollBoxEntryWidget*> Result;

	if (SelectionMode == EScrollBoxSelectionMode::Single)
	{
		if (CurrentSelection)
		{
			Result.Add(CurrentSelection);
		}
	}
	else
	{
		for (UScrollBoxEntryWidget* Entry : SelectedEntries)
		{
			if (Entry)
			{
				Result.Add(Entry);
			}
		}
	}

	return Result;
}

int32 UCommonScrollBoxManager::GetSelectedIndex() const
{
	return CurrentSelection ? CurrentSelection->EntryIndex : INDEX_NONE;
}

bool UCommonScrollBoxManager::HasSelection() const
{
	if (SelectionMode == EScrollBoxSelectionMode::Single)
	{
		return CurrentSelection != nullptr;
	}
	else
	{
		return SelectedEntries.Num() > 0;
	}
}

void UCommonScrollBoxManager::SelectNextEntry(bool bWrap)
{
	if (ManagedEntries.Num() == 0)
	{
		return;
	}

	int32 CurrentIndex = GetSelectedIndex();
	int32 NextIndex;

	if (CurrentIndex == INDEX_NONE)
	{
		NextIndex = 0;
	}
	else
	{
		NextIndex = CurrentIndex + 1;
		if (NextIndex >= ManagedEntries.Num())
		{
			NextIndex = bWrap ? 0 : ManagedEntries.Num() - 1;
		}
	}

	SelectEntryAtIndex(NextIndex, true);
}

void UCommonScrollBoxManager::SelectPreviousEntry(bool bWrap)
{
	if (ManagedEntries.Num() == 0)
	{
		return;
	}

	int32 CurrentIndex = GetSelectedIndex();
	int32 PrevIndex;

	if (CurrentIndex == INDEX_NONE)
	{
		PrevIndex = ManagedEntries.Num() - 1;
	}
	else
	{
		PrevIndex = CurrentIndex - 1;
		if (PrevIndex < 0)
		{
			PrevIndex = bWrap ? ManagedEntries.Num() - 1 : 0;
		}
	}

	SelectEntryAtIndex(PrevIndex, true);
}

// ========================================
// Sorting and Organization
// ========================================

void UCommonScrollBoxManager::SortByPriority(EScrollBoxSortOrder Order)
{
	if (Order == EScrollBoxSortOrder::Ascending)
	{
		ManagedEntries.Sort([](const TObjectPtr<UScrollBoxEntryWidget>& A, const TObjectPtr<UScrollBoxEntryWidget>& B)
		{
			return A->SortPriority < B->SortPriority;
		});
	}
	else
	{
		ManagedEntries.Sort([](const TObjectPtr<UScrollBoxEntryWidget>& A, const TObjectPtr<UScrollBoxEntryWidget>& B)
		{
			return A->SortPriority > B->SortPriority;
		});
	}

	UpdateEntryIndices();
	RegenerateContent();
	OnScrollBoxUpdated.Broadcast(ManagedEntries.Num());
}

void UCommonScrollBoxManager::SortEntries(FScrollBoxSortPredicate Predicate)
{
	if (!Predicate.IsBound())
	{
		return;
	}

	ManagedEntries.Sort([&Predicate](const TObjectPtr<UScrollBoxEntryWidget>& A, const TObjectPtr<UScrollBoxEntryWidget>& B)
	{
		return Predicate.Execute(A.Get(), B.Get());
	});

	UpdateEntryIndices();
	RegenerateContent();
	OnScrollBoxUpdated.Broadcast(ManagedEntries.Num());
}

bool UCommonScrollBoxManager::MoveEntry(UScrollBoxEntryWidget* Entry, int32 NewIndex)
{
	if (!Entry || !ContainsEntry(Entry))
	{
		return false;
	}

	int32 CurrentIndex = ManagedEntries.Find(Entry);
	if (CurrentIndex == NewIndex)
	{
		return true;
	}

	NewIndex = FMath::Clamp(NewIndex, 0, ManagedEntries.Num() - 1);

	ManagedEntries.RemoveAt(CurrentIndex);
	ManagedEntries.Insert(Entry, NewIndex);

	UpdateEntryIndices();
	RegenerateContent();
	OnScrollBoxUpdated.Broadcast(ManagedEntries.Num());

	return true;
}

bool UCommonScrollBoxManager::SwapEntries(int32 IndexA, int32 IndexB)
{
	if (!ManagedEntries.IsValidIndex(IndexA) || !ManagedEntries.IsValidIndex(IndexB))
	{
		return false;
	}

	if (IndexA == IndexB)
	{
		return true;
	}

	ManagedEntries.Swap(IndexA, IndexB);

	UpdateEntryIndices();
	RegenerateContent();
	OnScrollBoxUpdated.Broadcast(ManagedEntries.Num());

	return true;
}

void UCommonScrollBoxManager::ReverseOrder()
{
	Algo::Reverse(ManagedEntries);

	UpdateEntryIndices();
	RegenerateContent();
	OnScrollBoxUpdated.Broadcast(ManagedEntries.Num());
}

// ========================================
// Scrolling
// ========================================

void UCommonScrollBoxManager::ScrollToEntry(UScrollBoxEntryWidget* Entry, bool bAnimated)
{
	if (!Entry || !ActiveScrollBoxWidget)
	{
		return;
	}

	// Try to cast to ScrollBox for scrolling functionality
	if (UScrollBox* SB = Cast<UScrollBox>(ActiveScrollBoxWidget))
	{
		SB->ScrollWidgetIntoView(Entry, bAnimated);
	}
}

void UCommonScrollBoxManager::ScrollToIndex(int32 Index, bool bAnimated)
{
	UScrollBoxEntryWidget* Entry = GetEntryAtIndex(Index);
	if (Entry)
	{
		ScrollToEntry(Entry, bAnimated);
	}
}

void UCommonScrollBoxManager::ScrollToTop(bool bAnimated)
{
	if (UScrollBox* SB = Cast<UScrollBox>(ActiveScrollBoxWidget))
	{
		SB->SetScrollOffset(0.0f);
	}
}

void UCommonScrollBoxManager::ScrollToBottom(bool bAnimated)
{
	if (UScrollBox* SB = Cast<UScrollBox>(ActiveScrollBoxWidget))
	{
		SB->ScrollToEnd();
	}
}

void UCommonScrollBoxManager::SetScrollOffset(float Offset)
{
	if (UScrollBox* SB = Cast<UScrollBox>(ActiveScrollBoxWidget))
	{
		SB->SetScrollOffset(Offset);
	}
}

float UCommonScrollBoxManager::GetScrollOffset() const
{
	if (UScrollBox* SB = Cast<UScrollBox>(ActiveScrollBoxWidget))
	{
		return SB->GetScrollOffset();
	}
	return 0.0f;
}

// ========================================
// Update and Refresh
// ========================================

void UCommonScrollBoxManager::RefreshEntryIndices()
{
	UpdateEntryIndices();
}

void UCommonScrollBoxManager::ForceRefresh()
{
	if (ActiveScrollBoxWidget)
	{
		ActiveScrollBoxWidget->InvalidateLayoutAndVolatility();
	}
}

void UCommonScrollBoxManager::RegenerateContent()
{
	if (!ActiveScrollBoxWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("CommonScrollBoxManager::RegenerateContent - No scroll box widget!"));
		return;
	}

	// Clear scroll box
	ActiveScrollBoxWidget->ClearChildren();

	// Re-add all entries in order
	for (UScrollBoxEntryWidget* Entry : ManagedEntries)
	{
		if (Entry)
		{
			ActiveScrollBoxWidget->AddChild(Entry);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("CommonScrollBoxManager: Regenerated content with %d entries"), ManagedEntries.Num());
}

// ========================================
// Focus and Navigation
// ========================================

void UCommonScrollBoxManager::FocusFirstEntry()
{
	if (ManagedEntries.Num() > 0 && ManagedEntries[0])
	{
		FocusEntry(ManagedEntries[0]);
	}
}

void UCommonScrollBoxManager::FocusEntry(UScrollBoxEntryWidget* Entry)
{
	if (Entry)
	{
		Entry->SetFocus();
	}
}

void UCommonScrollBoxManager::FocusEntryAtIndex(int32 Index)
{
	UScrollBoxEntryWidget* Entry = GetEntryAtIndex(Index);
	if (Entry)
	{
		FocusEntry(Entry);
	}
}

// ========================================
// Internal Helpers
// ========================================

void UCommonScrollBoxManager::AddEntryToScrollBox(UScrollBoxEntryWidget* Entry)
{
	if (ActiveScrollBoxWidget && Entry)
	{
		ActiveScrollBoxWidget->AddChild(Entry);
		UE_LOG(LogTemp, Verbose, TEXT("CommonScrollBoxManager: Added entry to scroll box widget"));
	}
	else if (!ActiveScrollBoxWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("CommonScrollBoxManager::AddEntryToScrollBox - No scroll box widget! Entry added to array but not displayed."));
	}
}

void UCommonScrollBoxManager::RemoveEntryFromScrollBox(UScrollBoxEntryWidget* Entry)
{
	if (ActiveScrollBoxWidget && Entry)
	{
		ActiveScrollBoxWidget->RemoveChild(Entry);
	}
}

void UCommonScrollBoxManager::UpdateEntryIndices()
{
	for (int32 i = 0; i < ManagedEntries.Num(); ++i)
	{
		if (ManagedEntries[i])
		{
			ManagedEntries[i]->SetEntryIndex(i);
		}
	}
}

void UCommonScrollBoxManager::HandleEntryClicked(UScrollBoxEntryWidget* Entry)
{
	if (!Entry)
	{
		return;
	}

	// Select clicked entry
	SelectEntry(Entry, true);
}

void UCommonScrollBoxManager::HandleEntrySelectionChanged(UScrollBoxEntryWidget* Entry, bool bIsSelected)
{
	// This is called from the entry itself
	// We can use this for multi-selection tracking
	if (SelectionMode == EScrollBoxSelectionMode::Multi)
	{
		if (bIsSelected)
		{
			SelectedEntries.AddUnique(Entry);
		}
		else
		{
			SelectedEntries.Remove(Entry);
		}
	}
}
