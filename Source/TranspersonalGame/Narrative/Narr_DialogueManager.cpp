#include "Narr_DialogueManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    bDialogueActive = false;
    CurrentDialogueIndex = 0;
    CurrentQuestID = TEXT("");
    CurrentSpeaker = TEXT("");
    CurrentDialogueText = TEXT("");
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Register tutorial dialogues
    RegisterTutorialDialogue();
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue Manager initialized"));
}

void UNarr_DialogueManager::Deinitialize()
{
    EndDialogue();
    QuestDialogues.Empty();
    TutorialDialogues.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueManager::StartDialogue(const FString& QuestID, const FString& SpeakerName)
{
    if (bDialogueActive)
    {
        EndDialogue();
    }

    if (!QuestDialogues.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest dialogue not found: %s"), *QuestID);
        return;
    }

    const FNarr_QuestDialogue& QuestDialogue = QuestDialogues[QuestID];
    if (QuestDialogue.DialogueEntries.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No dialogue entries for quest: %s"), *QuestID);
        return;
    }

    bDialogueActive = true;
    CurrentQuestID = QuestID;
    CurrentDialogueIndex = 0;
    CurrentSpeaker = SpeakerName.IsEmpty() ? QuestDialogue.DialogueEntries[0].SpeakerName : SpeakerName;
    
    ProcessCurrentDialogue();
    
    OnDialogueStarted.Broadcast(CurrentSpeaker, CurrentDialogueText);
}

void UNarr_DialogueManager::EndDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    bDialogueActive = false;
    CurrentQuestID = TEXT("");
    CurrentDialogueIndex = 0;
    CurrentSpeaker = TEXT("");
    CurrentDialogueText = TEXT("");

    // Clear any active timer
    if (GetWorld() && DialogueTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }

    OnDialogueEnded.Broadcast();
}

void UNarr_DialogueManager::SelectChoice(int32 ChoiceIndex)
{
    if (!bDialogueActive || !QuestDialogues.Contains(CurrentQuestID))
    {
        return;
    }

    const FNarr_QuestDialogue& QuestDialogue = QuestDialogues[CurrentQuestID];
    if (!QuestDialogue.DialogueEntries.IsValidIndex(CurrentDialogueIndex))
    {
        return;
    }

    const FNarr_DialogueEntry& CurrentEntry = QuestDialogue.DialogueEntries[CurrentDialogueIndex];
    
    if (CurrentEntry.NextDialogueIDs.IsValidIndex(ChoiceIndex))
    {
        int32 NextID = CurrentEntry.NextDialogueIDs[ChoiceIndex];
        
        if (NextID == -1)
        {
            // End dialogue
            EndDialogue();
            return;
        }
        
        // Find dialogue entry with matching ID
        for (int32 i = 0; i < QuestDialogue.DialogueEntries.Num(); i++)
        {
            if (i == NextID)
            {
                CurrentDialogueIndex = i;
                ProcessCurrentDialogue();
                break;
            }
        }
    }
    else
    {
        // No valid next dialogue, end conversation
        EndDialogue();
    }
}

void UNarr_DialogueManager::AddQuestDialogue(const FString& QuestID, const TArray<FNarr_DialogueEntry>& Entries)
{
    FNarr_QuestDialogue NewQuestDialogue;
    NewQuestDialogue.QuestID = QuestID;
    NewQuestDialogue.DialogueEntries = Entries;
    NewQuestDialogue.bIsCompleted = false;

    QuestDialogues.Add(QuestID, NewQuestDialogue);
    
    UE_LOG(LogTemp, Log, TEXT("Added quest dialogue: %s with %d entries"), *QuestID, Entries.Num());
}

bool UNarr_DialogueManager::IsDialogueActive() const
{
    return bDialogueActive;
}

FString UNarr_DialogueManager::GetCurrentSpeaker() const
{
    return CurrentSpeaker;
}

FString UNarr_DialogueManager::GetCurrentDialogueText() const
{
    return CurrentDialogueText;
}

void UNarr_DialogueManager::StartTutorialDialogue(ETutorialStep TutorialStep)
{
    if (!TutorialDialogues.Contains(TutorialStep))
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutorial dialogue not found for step: %d"), (int32)TutorialStep);
        return;
    }

    const FNarr_DialogueEntry& TutorialEntry = TutorialDialogues[TutorialStep];
    
    bDialogueActive = true;
    CurrentQuestID = FString::Printf(TEXT("Tutorial_%d"), (int32)TutorialStep);
    CurrentDialogueIndex = 0;
    CurrentSpeaker = TutorialEntry.SpeakerName;
    CurrentDialogueText = TutorialEntry.DialogueText;

    OnDialogueStarted.Broadcast(CurrentSpeaker, CurrentDialogueText);

    // Auto-end tutorial dialogue after display duration
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, 
            [this]() { EndDialogue(); }, 
            TutorialEntry.DisplayDuration, 
            false);
    }
}

void UNarr_DialogueManager::RegisterTutorialDialogue()
{
    // Movement tutorial
    FNarr_DialogueEntry MovementTutorial;
    MovementTutorial.SpeakerName = TEXT("Survival Guide");
    MovementTutorial.DialogueText = TEXT("Use WASD to move. Watch your footing on uneven terrain. Predators can hear loud footsteps.");
    MovementTutorial.DisplayDuration = 4.0f;
    TutorialDialogues.Add(ETutorialStep::Movement, MovementTutorial);

    // Resource gathering tutorial
    FNarr_DialogueEntry ResourceTutorial;
    ResourceTutorial.SpeakerName = TEXT("Survival Guide");
    ResourceTutorial.DialogueText = TEXT("Gather stones, wood, and fiber to survive. Look for glowing resources scattered across the land.");
    ResourceTutorial.DisplayDuration = 4.0f;
    TutorialDialogues.Add(ETutorialStep::ResourceGathering, ResourceTutorial);

    // Crafting tutorial
    FNarr_DialogueEntry CraftingTutorial;
    CraftingTutorial.SpeakerName = TEXT("Survival Guide");
    CraftingTutorial.DialogueText = TEXT("Find crafting stations to create tools and weapons. You'll need them to survive the dangers ahead.");
    CraftingTutorial.DisplayDuration = 4.0f;
    TutorialDialogues.Add(ETutorialStep::Crafting, CraftingTutorial);

    // Dinosaur encounter tutorial
    FNarr_DialogueEntry DinosaurTutorial;
    DinosaurTutorial.SpeakerName = TEXT("Survival Guide");
    DinosaurTutorial.DialogueText = TEXT("Massive predators roam these lands. Stay hidden, move quietly, and always have an escape route.");
    DinosaurTutorial.DisplayDuration = 5.0f;
    TutorialDialogues.Add(ETutorialStep::DinosaurEncounter, DinosaurTutorial);
}

void UNarr_DialogueManager::ProcessCurrentDialogue()
{
    if (!QuestDialogues.Contains(CurrentQuestID))
    {
        return;
    }

    const FNarr_QuestDialogue& QuestDialogue = QuestDialogues[CurrentQuestID];
    if (!QuestDialogue.DialogueEntries.IsValidIndex(CurrentDialogueIndex))
    {
        EndDialogue();
        return;
    }

    const FNarr_DialogueEntry& CurrentEntry = QuestDialogue.DialogueEntries[CurrentDialogueIndex];
    CurrentDialogueText = CurrentEntry.DialogueText;
    CurrentSpeaker = CurrentEntry.SpeakerName;

    // Check if there are player choices
    if (CurrentEntry.PlayerChoices.Num() > 0)
    {
        ShowChoices(CurrentEntry.PlayerChoices, CurrentEntry.NextDialogueIDs);
    }
    else
    {
        // Auto-advance after display duration
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, 
                [this]() {
                    CurrentDialogueIndex++;
                    ProcessCurrentDialogue();
                }, 
                CurrentEntry.DisplayDuration, 
                false);
        }
    }
}

void UNarr_DialogueManager::ShowChoices(const TArray<FString>& Choices, const TArray<int32>& NextIDs)
{
    OnChoicePresented.Broadcast(Choices, NextIDs);
}