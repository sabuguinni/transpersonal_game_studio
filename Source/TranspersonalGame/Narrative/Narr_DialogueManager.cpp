#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    DialogueDisplayTime = 4.0f;
    bAutoAdvanceDialogue = true;
    MaxConcurrentDialogues = 1;
    
    CurrentSpeaker = TEXT("");
    CurrentDialogueText = TEXT("");
    bIsDialogueActive = false;
    CurrentLineIndex = 0;
    CurrentSequenceID = TEXT("");
    CurrentLineTimer = 0.0f;
    ActiveDialogueCount = 0;
}

void UNarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Initialized with %d dialogue sequences"), DialogueSequences.Num());
}

void UNarr_DialogueManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsDialogueActive && bAutoAdvanceDialogue)
    {
        CurrentLineTimer += DeltaTime;
        
        FNarr_DialogueSequence* CurrentSequence = FindDialogueSequence(CurrentSequenceID);
        if (CurrentSequence && CurrentLineIndex < CurrentSequence->DialogueLines.Num())
        {
            const FNarr_DialogueLine& CurrentLine = CurrentSequence->DialogueLines[CurrentLineIndex];
            
            if (CurrentLineTimer >= CurrentLine.Duration)
            {
                AdvanceDialogue();
            }
        }
    }
}

void UNarr_DialogueManager::StartDialogueSequence(const FString& SequenceID)
{
    if (ActiveDialogueCount >= MaxConcurrentDialogues)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Max concurrent dialogues reached"));
        return;
    }
    
    FNarr_DialogueSequence* Sequence = FindDialogueSequence(SequenceID);
    if (!Sequence)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Dialogue sequence not found: %s"), *SequenceID);
        return;
    }
    
    if (Sequence->DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Empty dialogue sequence: %s"), *SequenceID);
        return;
    }
    
    CurrentSequenceID = SequenceID;
    CurrentLineIndex = 0;
    bIsDialogueActive = true;
    ActiveDialogueCount++;
    
    ProcessCurrentDialogueLine();
    
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Started dialogue sequence: %s"), *SequenceID);
}

void UNarr_DialogueManager::StopCurrentDialogue()
{
    if (bIsDialogueActive)
    {
        bIsDialogueActive = false;
        CurrentSpeaker = TEXT("");
        CurrentDialogueText = TEXT("");
        CurrentSequenceID = TEXT("");
        CurrentLineIndex = 0;
        CurrentLineTimer = 0.0f;
        ActiveDialogueCount = FMath::Max(0, ActiveDialogueCount - 1);
        
        UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Stopped current dialogue"));
    }
}

void UNarr_DialogueManager::AdvanceDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    FNarr_DialogueSequence* CurrentSequence = FindDialogueSequence(CurrentSequenceID);
    if (!CurrentSequence)
    {
        StopCurrentDialogue();
        return;
    }
    
    CurrentLineIndex++;
    CurrentLineTimer = 0.0f;
    
    if (CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        OnDialogueLineComplete();
        StopCurrentDialogue();
        return;
    }
    
    ProcessCurrentDialogueLine();
}

void UNarr_DialogueManager::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    // Remove existing sequence with same ID
    DialogueSequences.RemoveAll([&](const FNarr_DialogueSequence& Existing)
    {
        return Existing.SequenceID == NewSequence.SequenceID;
    });
    
    DialogueSequences.Add(NewSequence);
    
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Added dialogue sequence: %s"), *NewSequence.SequenceID);
}

bool UNarr_DialogueManager::HasDialogueSequence(const FString& SequenceID) const
{
    return DialogueSequences.ContainsByPredicate([&](const FNarr_DialogueSequence& Sequence)
    {
        return Sequence.SequenceID == SequenceID;
    });
}

void UNarr_DialogueManager::TriggerContextualDialogue(ENarr_SurvivalEvent EventType)
{
    FString ContextualSequenceID;
    
    switch (EventType)
    {
        case ENarr_SurvivalEvent::FirstHunt:
            ContextualSequenceID = TEXT("FirstHunt_Dialogue");
            break;
        case ENarr_SurvivalEvent::PackEncounter:
            ContextualSequenceID = TEXT("PackEncounter_Dialogue");
            break;
        case ENarr_SurvivalEvent::WaterFound:
            ContextualSequenceID = TEXT("WaterFound_Dialogue");
            break;
        case ENarr_SurvivalEvent::ShelterBuilt:
            ContextualSequenceID = TEXT("ShelterBuilt_Dialogue");
            break;
        case ENarr_SurvivalEvent::FireDiscovered:
            ContextualSequenceID = TEXT("FireDiscovered_Dialogue");
            break;
        case ENarr_SurvivalEvent::TerritoryMarked:
            ContextualSequenceID = TEXT("TerritoryMarked_Dialogue");
            break;
        case ENarr_SurvivalEvent::AlphaChallenge:
            ContextualSequenceID = TEXT("AlphaChallenge_Dialogue");
            break;
        case ENarr_SurvivalEvent::StormSurvived:
            ContextualSequenceID = TEXT("StormSurvived_Dialogue");
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Unknown survival event type"));
            return;
    }
    
    if (HasDialogueSequence(ContextualSequenceID))
    {
        StartDialogueSequence(ContextualSequenceID);
    }
}

void UNarr_DialogueManager::SetDialogueDisplayTime(float NewDisplayTime)
{
    DialogueDisplayTime = FMath::Max(1.0f, NewDisplayTime);
}

void UNarr_DialogueManager::ProcessCurrentDialogueLine()
{
    FNarr_DialogueSequence* CurrentSequence = FindDialogueSequence(CurrentSequenceID);
    if (!CurrentSequence || CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = CurrentSequence->DialogueLines[CurrentLineIndex];
    
    CurrentSpeaker = CurrentLine.SpeakerName;
    CurrentDialogueText = CurrentLine.DialogueText;
    CurrentLineTimer = 0.0f;
    
    // Display dialogue on screen
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), *CurrentSpeaker, *CurrentDialogueText);
        GEngine->AddOnScreenDebugMessage(-1, CurrentLine.Duration, FColor::White, DisplayText);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: %s: %s"), *CurrentSpeaker, *CurrentDialogueText);
}

void UNarr_DialogueManager::OnDialogueLineComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Dialogue sequence completed: %s"), *CurrentSequenceID);
}

FNarr_DialogueSequence* UNarr_DialogueManager::FindDialogueSequence(const FString& SequenceID)
{
    return DialogueSequences.FindByPredicate([&](const FNarr_DialogueSequence& Sequence)
    {
        return Sequence.SequenceID == SequenceID;
    });
}

void UNarr_DialogueManager::InitializeDefaultDialogues()
{
    // First Hunt Dialogue
    FNarr_DialogueSequence FirstHuntSequence;
    FirstHuntSequence.SequenceID = TEXT("FirstHunt_Dialogue");
    FirstHuntSequence.bIsRepeatable = false;
    FirstHuntSequence.Priority = 5;
    
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Hunter");
    Line1.DialogueText = TEXT("The scent is fresh. My prey is near.");
    Line1.Duration = 3.5f;
    Line1.EmotionalTone = ENarr_EmotionalTone::Determined;
    FirstHuntSequence.DialogueLines.Add(Line1);
    
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Hunter");
    Line2.DialogueText = TEXT("Steady now. One mistake and I become the hunted.");
    Line2.Duration = 4.0f;
    Line2.EmotionalTone = ENarr_EmotionalTone::Tense;
    FirstHuntSequence.DialogueLines.Add(Line2);
    
    AddDialogueSequence(FirstHuntSequence);
    
    // Pack Encounter Dialogue
    FNarr_DialogueSequence PackEncounterSequence;
    PackEncounterSequence.SequenceID = TEXT("PackEncounter_Dialogue");
    PackEncounterSequence.bIsRepeatable = true;
    PackEncounterSequence.Priority = 4;
    
    FNarr_DialogueLine PackLine1;
    PackLine1.SpeakerName = TEXT("Survivor");
    PackLine1.DialogueText = TEXT("They hunt together. I must be smarter, not stronger.");
    PackLine1.Duration = 3.8f;
    PackLine1.EmotionalTone = ENarr_EmotionalTone::Fearful;
    PackEncounterSequence.DialogueLines.Add(PackLine1);
    
    AddDialogueSequence(PackEncounterSequence);
    
    // Water Found Dialogue
    FNarr_DialogueSequence WaterFoundSequence;
    WaterFoundSequence.SequenceID = TEXT("WaterFound_Dialogue");
    WaterFoundSequence.bIsRepeatable = true;
    WaterFoundSequence.Priority = 3;
    
    FNarr_DialogueLine WaterLine1;
    WaterLine1.SpeakerName = TEXT("Survivor");
    WaterLine1.DialogueText = TEXT("Clean water. Life flows through these lands after all.");
    WaterLine1.Duration = 3.2f;
    WaterLine1.EmotionalTone = ENarr_EmotionalTone::Relieved;
    WaterFoundSequence.DialogueLines.Add(WaterLine1);
    
    AddDialogueSequence(WaterFoundSequence);
    
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager: Initialized %d default dialogue sequences"), DialogueSequences.Num());
}