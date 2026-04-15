#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNarrativeManager::UNarrativeManager()
{
    CurrentNarrativeState = ENarr_NarrativeState::Intro;
    bIsDialogueActive = false;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initializing narrative system"));
    
    LoadNarrativeData();
    InitializeStoryBeats();
    
    // Set initial narrative flags
    SetNarrativeFlag(TEXT("GameStarted"), true);
    SetNarrativeFlag(TEXT("HasFire"), false);
    SetNarrativeFlag(TEXT("HasWeapon"), false);
    SetNarrativeFlag(TEXT("MetTribe"), false);
    SetNarrativeFlag(TEXT("FirstHuntComplete"), false);
}

void UNarrativeManager::Deinitialize()
{
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DialogueTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    Super::Deinitialize();
}

void UNarrativeManager::TriggerNarrativeEvent(const FString& EventID, const FString& EventData)
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Triggering event %s with data: %s"), *EventID, *EventData);
    
    // Handle specific narrative events
    if (EventID == TEXT("PlayerDeath"))
    {
        SetNarrativeFlag(TEXT("HasDied"), true);
    }
    else if (EventID == TEXT("FirstKill"))
    {
        SetNarrativeFlag(TEXT("FirstKillComplete"), true);
        AdvanceStoryBeat(TEXT("FirstHunt"));
    }
    else if (EventID == TEXT("FireCreated"))
    {
        SetNarrativeFlag(TEXT("HasFire"), true);
        AdvanceStoryBeat(TEXT("Survival_Fire"));
    }
    else if (EventID == TEXT("WeaponCrafted"))
    {
        SetNarrativeFlag(TEXT("HasWeapon"), true);
    }
    else if (EventID == TEXT("TribalEncounter"))
    {
        SetNarrativeFlag(TEXT("MetTribe"), true);
        SetNarrativeState(ENarr_NarrativeState::TribalContact);
    }
    
    // Broadcast the event
    OnNarrativeEvent.Broadcast(EventID, EventData);
}

void UNarrativeManager::SetNarrativeFlag(const FString& FlagName, bool bValue)
{
    NarrativeFlags.Add(FlagName, bValue);
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Set flag %s to %s"), *FlagName, bValue ? TEXT("true") : TEXT("false"));
}

bool UNarrativeManager::GetNarrativeFlag(const FString& FlagName) const
{
    const bool* FoundFlag = NarrativeFlags.Find(FlagName);
    return FoundFlag ? *FoundFlag : false;
}

void UNarrativeManager::AdvanceStoryBeat(const FString& BeatID)
{
    FNarr_StoryBeat* Beat = StoryBeats.Find(BeatID);
    if (Beat && !Beat->bIsCompleted)
    {
        if (CheckConditions(Beat->TriggerConditions))
        {
            Beat->bIsCompleted = true;
            ProcessCompletionFlags(Beat->CompletionFlags);
            
            UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Completed story beat %s"), *BeatID);
            TriggerNarrativeEvent(TEXT("StoryBeatCompleted"), BeatID);
        }
    }
}

bool UNarrativeManager::IsStoryBeatCompleted(const FString& BeatID) const
{
    const FNarr_StoryBeat* Beat = StoryBeats.Find(BeatID);
    return Beat ? Beat->bIsCompleted : false;
}

void UNarrativeManager::StartDialogue(const FString& DialogueID)
{
    if (bIsDialogueActive)
    {
        EndDialogue();
    }
    
    // Load dialogue from data table
    if (DialogueDataTable.IsValid())
    {
        UDataTable* Table = DialogueDataTable.LoadSynchronous();
        if (Table)
        {
            FNarr_DialogueEntry* DialogueEntry = Table->FindRow<FNarr_DialogueEntry>(FName(*DialogueID), TEXT(""));
            if (DialogueEntry && CheckConditions(DialogueEntry->RequiredFlags))
            {
                CurrentDialogue = *DialogueEntry;
                bIsDialogueActive = true;
                
                // Process flags set by this dialogue
                ProcessCompletionFlags(DialogueEntry->SetFlags);
                
                // Start dialogue timer
                if (GetWorld())
                {
                    GetWorld()->GetTimerManager().SetTimer(
                        DialogueTimerHandle,
                        this,
                        &UNarrativeManager::OnDialogueTimeout,
                        CurrentDialogue.Duration,
                        false
                    );
                }
                
                OnDialogueStarted.Broadcast(CurrentDialogue);
                UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Started dialogue %s"), *DialogueID);
            }
        }
    }
}

void UNarrativeManager::EndDialogue()
{
    if (bIsDialogueActive)
    {
        bIsDialogueActive = false;
        
        if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DialogueTimerHandle))
        {
            GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
        }
        
        OnDialogueEnded.Broadcast();
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Ended dialogue"));
    }
}

void UNarrativeManager::SetNarrativeState(ENarr_NarrativeState NewState)
{
    if (CurrentNarrativeState != NewState)
    {
        ENarr_NarrativeState PreviousState = CurrentNarrativeState;
        CurrentNarrativeState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: State changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
        
        // Trigger state-specific events
        FString StateEventID = FString::Printf(TEXT("StateChanged_%d"), (int32)NewState);
        TriggerNarrativeEvent(StateEventID, FString::Printf(TEXT("%d"), (int32)PreviousState));
    }
}

void UNarrativeManager::LoadNarrativeData()
{
    // Set up data table references
    DialogueDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DialogueTable")));
    StoryBeatsDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/StoryBeatsTable")));
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Loaded narrative data references"));
}

void UNarrativeManager::InitializeStoryBeats()
{
    // Initialize core story beats
    FNarr_StoryBeat IntroductionBeat;
    IntroductionBeat.BeatID = TEXT("Introduction");
    IntroductionBeat.Title = FText::FromString(TEXT("Awakening"));
    IntroductionBeat.Description = FText::FromString(TEXT("The player awakens in the prehistoric world"));
    IntroductionBeat.TriggerConditions.Add(TEXT("GameStarted"));
    IntroductionBeat.CompletionFlags.Add(TEXT("IntroComplete"));
    StoryBeats.Add(IntroductionBeat.BeatID, IntroductionBeat);
    
    FNarr_StoryBeat FirstHuntBeat;
    FirstHuntBeat.BeatID = TEXT("FirstHunt");
    FirstHuntBeat.Title = FText::FromString(TEXT("First Hunt"));
    FirstHuntBeat.Description = FText::FromString(TEXT("Player's first successful hunt"));
    FirstHuntBeat.TriggerConditions.Add(TEXT("FirstKillComplete"));
    FirstHuntBeat.CompletionFlags.Add(TEXT("FirstHuntComplete"));
    StoryBeats.Add(FirstHuntBeat.BeatID, FirstHuntBeat);
    
    FNarr_StoryBeat SurvivalFireBeat;
    SurvivalFireBeat.BeatID = TEXT("Survival_Fire");
    SurvivalFireBeat.Title = FText::FromString(TEXT("Mastery of Fire"));
    SurvivalFireBeat.Description = FText::FromString(TEXT("Player learns to create and maintain fire"));
    SurvivalFireBeat.TriggerConditions.Add(TEXT("HasFire"));
    SurvivalFireBeat.CompletionFlags.Add(TEXT("FireMasteryComplete"));
    StoryBeats.Add(SurvivalFireBeat.BeatID, SurvivalFireBeat);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized %d story beats"), StoryBeats.Num());
}

bool UNarrativeManager::CheckConditions(const TArray<FString>& Conditions) const
{
    for (const FString& Condition : Conditions)
    {
        if (!GetNarrativeFlag(Condition))
        {
            return false;
        }
    }
    return true;
}

void UNarrativeManager::ProcessCompletionFlags(const TArray<FString>& Flags)
{
    for (const FString& Flag : Flags)
    {
        SetNarrativeFlag(Flag, true);
    }
}

void UNarrativeManager::OnDialogueTimeout()
{
    EndDialogue();
}