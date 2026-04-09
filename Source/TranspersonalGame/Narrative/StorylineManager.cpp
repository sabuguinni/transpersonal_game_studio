#include "StorylineManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"

UStorylineManager::UStorylineManager()
{
    bAutoSaveProgress = true;
    AutoSaveInterval = 300.0f; // 5 minutes
    MaxConcurrentStorylines = 5;
    bIsInitialized = false;
    LastSaveTime = 0.0f;
}

void UStorylineManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("StorylineManager: Initializing..."));
    
    LoadStorylineData();
    
    // Set up auto-save timer
    if (bAutoSaveProgress && UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(AutoSaveTimer, this, &UStorylineManager::SaveProgressToFile, AutoSaveInterval, true);
    }
    
    // Set up story processing timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(StoryProcessingTimer, this, &UStorylineManager::CheckForAutoUnlocks, 1.0f, true);
    }
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("StorylineManager: Initialization complete"));
}

void UStorylineManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("StorylineManager: Deinitializing..."));
    
    if (bAutoSaveProgress)
    {
        SaveProgressToFile();
    }
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoSaveTimer);
        World->GetTimerManager().ClearTimer(StoryProcessingTimer);
    }
    
    ActiveStorylines.Empty();
    StoryBeatTimers.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

void UStorylineManager::Tick(float DeltaTime)
{
    if (!bIsInitialized)
    {
        return;
    }
    
    // Update total play time
    CurrentProgress.TotalPlayTime += DeltaTime;
    
    // Process story beat timers
    ProcessStoryBeatTimers(DeltaTime);
}

bool UStorylineManager::StartStoryline(FName StorylineID)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("StorylineManager: Attempted to start storyline before initialization"));
        return false;
    }
    
    FStoryline* Storyline = FindStoryline(StorylineID);
    if (!Storyline)
    {
        UE_LOG(LogTemp, Warning, TEXT("StorylineManager: Storyline not found: %s"), *StorylineID.ToString());
        return false;
    }
    
    if (!CanStartStoryline(StorylineID))
    {
        UE_LOG(LogTemp, Log, TEXT("StorylineManager: Cannot start storyline (conditions not met): %s"), *StorylineID.ToString());
        return false;
    }
    
    if (IsStorylineActive(StorylineID))
    {
        UE_LOG(LogTemp, Log, TEXT("StorylineManager: Storyline already active: %s"), *StorylineID.ToString());
        return true;
    }
    
    // Check if we're at max concurrent storylines
    if (ActiveStorylines.Num() >= MaxConcurrentStorylines)
    {
        UE_LOG(LogTemp, Warning, TEXT("StorylineManager: Maximum concurrent storylines reached"));
        return false;
    }
    
    // Check for conflicting storylines
    for (const FName& ActiveStorylineID : ActiveStorylines)
    {
        FStoryline* ActiveStoryline = FindStoryline(ActiveStorylineID);
        if (ActiveStoryline && ActiveStoryline->ConflictingStorylines.HasTag(FGameplayTag::RequestGameplayTag(StorylineID)))
        {
            UE_LOG(LogTemp, Warning, TEXT("StorylineManager: Storyline conflicts with active storyline: %s"), *ActiveStorylineID.ToString());
            return false;
        }
    }
    
    // Start the storyline
    Storyline->bIsActive = true;
    Storyline->CurrentBeatIndex = 0;
    ActiveStorylines.Add(StorylineID);
    CurrentProgress.StorylineProgress.Add(StorylineID, 0);
    
    // Sort by priority
    SortActiveStorylinesByPriority();
    
    // Trigger the first story beat if available
    if (Storyline->StoryBeats.Num() > 0)
    {
        TriggerStoryBeat(Storyline->StoryBeats[0]);
    }
    
    UE_LOG(LogTemp, Log, TEXT("StorylineManager: Started storyline: %s"), *StorylineID.ToString());
    return true;
}

bool UStorylineManager::AdvanceStoryline(FName StorylineID)
{
    FStoryline* Storyline = FindStoryline(StorylineID);
    if (!Storyline || !Storyline->bIsActive)
    {
        return false;
    }
    
    // Move to next story beat
    Storyline->CurrentBeatIndex++;
    CurrentProgress.StorylineProgress[StorylineID] = Storyline->CurrentBeatIndex;
    
    // Check if storyline is complete
    if (Storyline->CurrentBeatIndex >= Storyline->StoryBeats.Num())
    {
        Storyline->bIsCompleted = true;
        Storyline->bIsActive = false;
        ActiveStorylines.Remove(StorylineID);
        CurrentProgress.CompletedStorylines.Add(StorylineID);
        
        // Broadcast completion event
        OnStorylineCompleted.Broadcast(StorylineID, true);
        
        UE_LOG(LogTemp, Log, TEXT("StorylineManager: Completed storyline: %s"), *StorylineID.ToString());
        
        // Check for unlocked branching storylines
        for (const FName& BranchStoryline : Storyline->BranchingStorylines)
        {
            UnlockStoryline(BranchStoryline);
        }
    }
    else
    {
        // Trigger next story beat
        FName NextBeatID = Storyline->StoryBeats[Storyline->CurrentBeatIndex];
        TriggerStoryBeat(NextBeatID);
        
        // Broadcast progress event
        OnStorylineProgressed.Broadcast(StorylineID, Storyline->CurrentBeatIndex, Storyline->StoryBeats.Num());
    }
    
    return true;
}

bool UStorylineManager::TriggerStoryBeat(FName StoryBeatID, bool bForceActivation)
{
    FStoryBeat* StoryBeat = FindStoryBeat(StoryBeatID);
    if (!StoryBeat)
    {
        UE_LOG(LogTemp, Warning, TEXT("StorylineManager: Story beat not found: %s"), *StoryBeatID.ToString());
        return false;
    }
    
    if (!bForceActivation && !ValidateStoryBeatConditions(*StoryBeat))
    {
        UE_LOG(LogTemp, Log, TEXT("StorylineManager: Story beat conditions not met: %s"), *StoryBeatID.ToString());
        return false;
    }
    
    // Start minimum duration timer if required
    if (StoryBeat->MinimumDuration > 0.0f)
    {
        StoryBeatTimers.Add(StoryBeatID, StoryBeat->MinimumDuration);
    }
    
    // Add completion tags to story progress
    for (const FGameplayTag& Tag : StoryBeat->CompletionTags.GetGameplayTagArray())
    {
        AddStoryTag(Tag);
    }
    
    // Broadcast story beat triggered event
    OnStoryBeatTriggered.Broadcast(StoryBeatID, StoryBeat->Description);
    
    UE_LOG(LogTemp, Log, TEXT("StorylineManager: Triggered story beat: %s"), *StoryBeatID.ToString());
    
    // Auto-complete if no minimum duration
    if (StoryBeat->MinimumDuration <= 0.0f)
    {
        CompleteStoryBeat(StoryBeatID);
    }
    
    return true;
}

void UStorylineManager::CompleteStoryBeat(FName StoryBeatID)
{
    FStoryBeat* StoryBeat = FindStoryBeat(StoryBeatID);
    if (!StoryBeat)
    {
        return;
    }
    
    // Check if minimum duration has elapsed
    if (StoryBeatTimers.Contains(StoryBeatID) && StoryBeatTimers[StoryBeatID] > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("StorylineManager: Story beat minimum duration not elapsed: %s"), *StoryBeatID.ToString());
        return;
    }
    
    // Mark as completed
    CurrentProgress.CompletedStoryBeats.AddUnique(StoryBeatID);
    StoryBeatTimers.Remove(StoryBeatID);
    
    // Unlock follow-up story beats
    for (const FName& UnlockedBeatID : StoryBeat->UnlockedStoryBeats)
    {
        TriggerStoryBeat(UnlockedBeatID);
    }
    
    UE_LOG(LogTemp, Log, TEXT("StorylineManager: Completed story beat: %s"), *StoryBeatID.ToString());
}

bool UStorylineManager::IsStorylineActive(FName StorylineID) const
{
    return ActiveStorylines.Contains(StorylineID);
}

bool UStorylineManager::IsStoryBeatCompleted(FName StoryBeatID) const
{
    return CurrentProgress.CompletedStoryBeats.Contains(StoryBeatID);
}

bool UStorylineManager::CanStartStoryline(FName StorylineID) const
{
    FStoryline* Storyline = FindStoryline(StorylineID);
    if (!Storyline)
    {
        return false;
    }
    
    return ValidateStorylineConditions(*Storyline);
}

int32 UStorylineManager::GetStorylineProgress(FName StorylineID) const
{
    if (CurrentProgress.StorylineProgress.Contains(StorylineID))
    {
        return CurrentProgress.StorylineProgress[StorylineID];
    }
    return 0;
}

FStoryBeat UStorylineManager::GetCurrentStoryBeat(FName StorylineID) const
{
    FStoryline* Storyline = FindStoryline(StorylineID);
    if (!Storyline || !Storyline->bIsActive)
    {
        return FStoryBeat();
    }
    
    if (Storyline->CurrentBeatIndex < Storyline->StoryBeats.Num())
    {
        FName CurrentBeatID = Storyline->StoryBeats[Storyline->CurrentBeatIndex];
        FStoryBeat* CurrentBeat = FindStoryBeat(CurrentBeatID);
        if (CurrentBeat)
        {
            return *CurrentBeat;
        }
    }
    
    return FStoryBeat();
}

TArray<FName> UStorylineManager::GetActiveStorylines() const
{
    return ActiveStorylines;
}

bool UStorylineManager::MakeStoryChoice(FName ChoiceID, FName SelectedOption)
{
    // Add story choice tag
    FGameplayTag ChoiceTag = FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("Story.Choice.%s.%s"), *ChoiceID.ToString(), *SelectedOption.ToString())));
    AddStoryTag(ChoiceTag);
    
    UE_LOG(LogTemp, Log, TEXT("StorylineManager: Made story choice: %s -> %s"), *ChoiceID.ToString(), *SelectedOption.ToString());
    return true;
}

TArray<FName> UStorylineManager::GetAvailableStoryChoices() const
{
    TArray<FName> AvailableChoices;
    // TODO: Implement based on current story state and active storylines
    return AvailableChoices;
}

void UStorylineManager::UnlockStoryline(FName StorylineID)
{
    if (!CurrentProgress.UnlockedStorylines.Contains(StorylineID))
    {
        CurrentProgress.UnlockedStorylines.Add(StorylineID);
        UE_LOG(LogTemp, Log, TEXT("StorylineManager: Unlocked storyline: %s"), *StorylineID.ToString());
    }
}

void UStorylineManager::LoadStoryProgress(const FStoryProgress& Progress)
{
    CurrentProgress = Progress;
    
    // Rebuild active storylines list
    ActiveStorylines.Empty();
    for (const auto& StorylinePair : CurrentProgress.StorylineProgress)
    {
        FStoryline* Storyline = FindStoryline(StorylinePair.Key);
        if (Storyline && Storyline->bIsActive && !Storyline->bIsCompleted)
        {
            ActiveStorylines.Add(StorylinePair.Key);
        }
    }
    
    SortActiveStorylinesByPriority();
    
    UE_LOG(LogTemp, Log, TEXT("StorylineManager: Loaded story progress with %d active storylines"), ActiveStorylines.Num());
}

void UStorylineManager::ResetStoryProgress()
{
    CurrentProgress = FStoryProgress();
    ActiveStorylines.Empty();
    StoryBeatTimers.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("StorylineManager: Reset story progress"));
}

void UStorylineManager::AddStoryTag(FGameplayTag Tag)
{
    CurrentProgress.StoryTags.AddTag(Tag);
}

void UStorylineManager::RemoveStoryTag(FGameplayTag Tag)
{
    CurrentProgress.StoryTags.RemoveTag(Tag);
}

bool UStorylineManager::HasStoryTag(FGameplayTag Tag) const
{
    return CurrentProgress.StoryTags.HasTag(Tag);
}

void UStorylineManager::LoadStorylineData()
{
    // Load storyline data table
    if (StorylineDataTable.IsValid())
    {
        UDataTable* DataTable = StorylineDataTable.LoadSynchronous();
        if (DataTable)
        {
            UE_LOG(LogTemp, Log, TEXT("StorylineManager: Loaded storyline data table with %d entries"), DataTable->GetRowNames().Num());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("StorylineManager: Failed to load storyline data table"));
        }
    }
    
    // Load story beat data table
    if (StoryBeatDataTable.IsValid())
    {
        UDataTable* DataTable = StoryBeatDataTable.LoadSynchronous();
        if (DataTable)
        {
            UE_LOG(LogTemp, Log, TEXT("StorylineManager: Loaded story beat data table with %d entries"), DataTable->GetRowNames().Num());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("StorylineManager: Failed to load story beat data table"));
        }
    }
}

FStoryline* UStorylineManager::FindStoryline(FName StorylineID) const
{
    if (!StorylineDataTable.IsValid())
    {
        return nullptr;
    }
    
    UDataTable* DataTable = StorylineDataTable.LoadSynchronous();
    if (!DataTable)
    {
        return nullptr;
    }
    
    return DataTable->FindRow<FStoryline>(StorylineID, TEXT("FindStoryline"));
}

FStoryBeat* UStorylineManager::FindStoryBeat(FName StoryBeatID) const
{
    if (!StoryBeatDataTable.IsValid())
    {
        return nullptr;
    }
    
    UDataTable* DataTable = StoryBeatDataTable.LoadSynchronous();
    if (!DataTable)
    {
        return nullptr;
    }
    
    return DataTable->FindRow<FStoryBeat>(StoryBeatID, TEXT("FindStoryBeat"));
}

bool UStorylineManager::ValidateStorylineConditions(const FStoryline& Storyline) const
{
    // Check if storyline is unlocked
    if (!CurrentProgress.UnlockedStorylines.Contains(Storyline.StorylineID))
    {
        return false;
    }
    
    // Check prerequisite tags
    if (Storyline.PrerequisiteTags.Num() > 0)
    {
        if (!CurrentProgress.StoryTags.HasAll(Storyline.PrerequisiteTags))
        {
            return false;
        }
    }
    
    return true;
}

bool UStorylineManager::ValidateStoryBeatConditions(const FStoryBeat& StoryBeat) const
{
    // Check trigger conditions
    if (StoryBeat.TriggerConditions.Num() > 0)
    {
        if (!CurrentProgress.StoryTags.HasAll(StoryBeat.TriggerConditions))
        {
            return false;
        }
    }
    
    return true;
}

void UStorylineManager::ProcessStoryBeatTimers(float DeltaTime)
{
    TArray<FName> CompletedBeats;
    
    for (auto& TimerPair : StoryBeatTimers)
    {
        TimerPair.Value -= DeltaTime;
        if (TimerPair.Value <= 0.0f)
        {
            CompletedBeats.Add(TimerPair.Key);
        }
    }
    
    // Complete story beats whose timers have elapsed
    for (const FName& BeatID : CompletedBeats)
    {
        CompleteStoryBeat(BeatID);
    }
}

void UStorylineManager::CheckForAutoUnlocks()
{
    // Check if any storylines should be auto-unlocked based on current story tags
    // This would typically be implemented based on specific game logic
}

void UStorylineManager::SaveProgressToFile()
{
    CurrentProgress.LastSaveTime = FDateTime::Now();
    
    // TODO: Implement actual file saving using UE5 save game system
    // For now, just log that we would save
    UE_LOG(LogTemp, Log, TEXT("StorylineManager: Auto-saving story progress (placeholder)"));
}

void UStorylineManager::SortActiveStorylinesByPriority()
{
    ActiveStorylines.Sort([this](const FName& A, const FName& B)
    {
        FStoryline* StorylineA = FindStoryline(A);
        FStoryline* StorylineB = FindStoryline(B);
        
        if (!StorylineA || !StorylineB)
        {
            return false;
        }
        
        return StorylineA->Priority > StorylineB->Priority;
    });
}