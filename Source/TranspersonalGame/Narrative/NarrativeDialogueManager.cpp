#include "NarrativeDialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNarrativeDialogueManager::UNarrativeDialogueManager()
{
    bIsPlayingDialogue = false;
    DialogueTimer = 0.0f;
}

void UNarrativeDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager initialized"));
    
    // Initialize default narrative flags
    SetNarrativeFlag(TEXT("GameStarted"), true);
    SetNarrativeFlag(TEXT("FirstDinosaurEncounter"), false);
    SetNarrativeFlag(TEXT("FirstCraftingAttempt"), false);
    SetNarrativeFlag(TEXT("FirstShelterBuilt"), false);
    SetNarrativeFlag(TEXT("FirstPredatorEscape"), false);
    
    LoadNarrativeData();
}

void UNarrativeDialogueManager::Deinitialize()
{
    RegisteredEvents.Empty();
    NarrativeFlags.Empty();
    CurrentDialogueQueue.Empty();
    
    Super::Deinitialize();
}

void UNarrativeDialogueManager::RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event)
{
    RegisteredEvents.Add(Event.EventID, Event);
    UE_LOG(LogTemp, Warning, TEXT("Registered narrative event: %s"), *Event.EventID);
}

void UNarrativeDialogueManager::TriggerNarrativeEvent(const FString& EventID)
{
    if (RegisteredEvents.Contains(EventID))
    {
        const FNarr_NarrativeEvent& Event = RegisteredEvents[EventID];
        PlayDialogueSequence(Event.DialogueSequence);
        UE_LOG(LogTemp, Warning, TEXT("Triggered narrative event: %s"), *EventID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Narrative event not found: %s"), *EventID);
    }
}

void UNarrativeDialogueManager::PlayDialogueSequence(const TArray<FNarr_DialogueEntry>& DialogueSequence)
{
    if (DialogueSequence.Num() > 0)
    {
        CurrentDialogueQueue = DialogueSequence;
        bIsPlayingDialogue = true;
        DialogueTimer = 0.0f;
        ProcessDialogueQueue();
    }
}

void UNarrativeDialogueManager::SetNarrativeFlag(const FString& FlagName, bool bValue)
{
    NarrativeFlags.Add(FlagName, bValue);
    UE_LOG(LogTemp, Log, TEXT("Set narrative flag %s to %s"), *FlagName, bValue ? TEXT("true") : TEXT("false"));
}

bool UNarrativeDialogueManager::GetNarrativeFlag(const FString& FlagName) const
{
    if (NarrativeFlags.Contains(FlagName))
    {
        return NarrativeFlags[FlagName];
    }
    return false;
}

void UNarrativeDialogueManager::CheckLocationTriggers(const FVector& PlayerLocation)
{
    for (const auto& EventPair : RegisteredEvents)
    {
        const FNarr_NarrativeEvent& Event = EventPair.Value;
        
        if (Event.TriggerType == ENarr_EventTrigger::LocationBased)
        {
            float Distance = FVector::Dist(PlayerLocation, Event.TriggerLocation);
            if (Distance <= Event.TriggerRadius)
            {
                // Check if this event hasn't been triggered recently
                FString TriggerFlagName = FString::Printf(TEXT("Triggered_%s"), *Event.EventID);
                if (!GetNarrativeFlag(TriggerFlagName))
                {
                    TriggerNarrativeEvent(Event.EventID);
                    SetNarrativeFlag(TriggerFlagName, true);
                }
            }
        }
    }
}

void UNarrativeDialogueManager::LoadNarrativeData()
{
    // Create default survival narrative events
    
    // First encounter event
    FNarr_NarrativeEvent FirstEncounter;
    FirstEncounter.EventID = TEXT("FirstDinosaurSighting");
    FirstEncounter.EventDescription = FText::FromString(TEXT("Player's first dinosaur encounter"));
    FirstEncounter.TriggerType = ENarr_EventTrigger::LocationBased;
    FirstEncounter.TriggerLocation = FVector(0, 0, 0); // Savana biome
    FirstEncounter.TriggerRadius = 1000.0f;
    
    FNarr_DialogueEntry IntroDialogue;
    IntroDialogue.DialogueID = TEXT("intro_001");
    IntroDialogue.SpeakerName = TEXT("Narrator");
    IntroDialogue.DialogueText = FText::FromString(TEXT("The ancient valley holds many secrets, survivor. Listen carefully to the wind - it carries the scent of predators on the hunt."));
    IntroDialogue.Duration = 6.0f;
    
    FirstEncounter.DialogueSequence.Add(IntroDialogue);
    RegisterNarrativeEvent(FirstEncounter);
    
    // Predator warning event
    FNarr_NarrativeEvent PredatorWarning;
    PredatorWarning.EventID = TEXT("PredatorPackDetected");
    PredatorWarning.EventDescription = FText::FromString(TEXT("Warning about pack hunters"));
    PredatorWarning.TriggerType = ENarr_EventTrigger::ProximityBased;
    PredatorWarning.TriggerLocation = FVector(5000, 5000, 0);
    PredatorWarning.TriggerRadius = 2000.0f;
    
    FNarr_DialogueEntry WarningDialogue;
    WarningDialogue.DialogueID = TEXT("warning_001");
    WarningDialogue.SpeakerName = TEXT("SurvivalGuide");
    WarningDialogue.DialogueText = FText::FromString(TEXT("Warning! Pack hunters detected in the eastern ravines. Their coordinated movements suggest they are tracking prey."));
    WarningDialogue.Duration = 5.0f;
    
    PredatorWarning.DialogueSequence.Add(WarningDialogue);
    RegisterNarrativeEvent(PredatorWarning);
    
    UE_LOG(LogTemp, Warning, TEXT("Loaded %d narrative events"), RegisteredEvents.Num());
}

TArray<FNarr_DialogueEntry> UNarrativeDialogueManager::GetDialoguesByTags(const TArray<FString>& Tags)
{
    TArray<FNarr_DialogueEntry> MatchingDialogues;
    
    for (const auto& EventPair : RegisteredEvents)
    {
        const FNarr_NarrativeEvent& Event = EventPair.Value;
        for (const FNarr_DialogueEntry& Dialogue : Event.DialogueSequence)
        {
            // Simple tag matching - could be expanded
            for (const FString& Tag : Tags)
            {
                if (Dialogue.SpeakerName.Contains(Tag) || Dialogue.DialogueText.ToString().Contains(Tag))
                {
                    MatchingDialogues.Add(Dialogue);
                    break;
                }
            }
        }
    }
    
    return MatchingDialogues;
}

void UNarrativeDialogueManager::ProcessDialogueQueue()
{
    if (CurrentDialogueQueue.Num() > 0 && bIsPlayingDialogue)
    {
        const FNarr_DialogueEntry& CurrentDialogue = CurrentDialogueQueue[0];
        
        if (CheckDialogueConditions(CurrentDialogue))
        {
            // Display dialogue (in a real implementation, this would trigger UI)
            UE_LOG(LogTemp, Warning, TEXT("[%s]: %s"), *CurrentDialogue.SpeakerName, *CurrentDialogue.DialogueText.ToString());
            
            // Apply any effects from this dialogue
            ApplyDialogueEffects(CurrentDialogue);
            
            // Set timer for dialogue duration
            DialogueTimer = CurrentDialogue.Duration;
        }
        else
        {
            // Skip this dialogue if conditions aren't met
            CurrentDialogueQueue.RemoveAt(0);
            ProcessDialogueQueue();
        }
    }
    else
    {
        FinishCurrentDialogue();
    }
}

void UNarrativeDialogueManager::FinishCurrentDialogue()
{
    bIsPlayingDialogue = false;
    CurrentDialogueQueue.Empty();
    DialogueTimer = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("Dialogue sequence finished"));
}

bool UNarrativeDialogueManager::CheckDialogueConditions(const FNarr_DialogueEntry& Entry) const
{
    // Check if all required flags are set
    for (const FString& RequiredFlag : Entry.RequiredFlags)
    {
        if (!GetNarrativeFlag(RequiredFlag))
        {
            return false;
        }
    }
    return true;
}

void UNarrativeDialogueManager::ApplyDialogueEffects(const FNarr_DialogueEntry& Entry)
{
    // Set any flags that this dialogue should set
    for (const FString& FlagToSet : Entry.SetFlags)
    {
        SetNarrativeFlag(FlagToSet, true);
    }
}

void UNarrativeDialogueManager::Tick(float DeltaTime)
{
    if (bIsPlayingDialogue && DialogueTimer > 0.0f)
    {
        DialogueTimer -= DeltaTime;
        
        if (DialogueTimer <= 0.0f)
        {
            // Move to next dialogue in queue
            if (CurrentDialogueQueue.Num() > 0)
            {
                CurrentDialogueQueue.RemoveAt(0);
                ProcessDialogueQueue();
            }
            else
            {
                FinishCurrentDialogue();
            }
        }
    }
}