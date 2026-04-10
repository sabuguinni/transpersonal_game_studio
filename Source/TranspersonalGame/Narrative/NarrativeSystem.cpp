#include "NarrativeSystem.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UNarrativeSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsDialogueActive = false;
    CurrentDialogueID = "";
    CurrentSpeaker = nullptr;
    
    InitializeNarrativeEvents();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeSystem initialized successfully"));
}

void UNarrativeSystem::StartDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already active. Ending current dialogue first."));
        EndDialogue();
    }

    if (!DialogueTable)
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueTable is null. Cannot start dialogue."));
        return;
    }

    FDialogueEntry* DialogueEntry = DialogueTable->FindRow<FDialogueEntry>(FName(*DialogueID), TEXT(""));
    if (!DialogueEntry)
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue entry not found: %s"), *DialogueID);
        return;
    }

    CurrentDialogueID = DialogueID;
    CurrentSpeaker = Speaker;
    bIsDialogueActive = true;

    OnDialogueStarted.Broadcast(DialogueID, DialogueEntry->SpeakerName);
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s with speaker: %s"), *DialogueID, *DialogueEntry->SpeakerName);
}

void UNarrativeSystem::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    FString EndedDialogueID = CurrentDialogueID;
    
    bIsDialogueActive = false;
    CurrentDialogueID = "";
    CurrentSpeaker = nullptr;

    OnDialogueEnded.Broadcast(EndedDialogueID);
    
    UE_LOG(LogTemp, Log, TEXT("Ended dialogue: %s"), *EndedDialogueID);
}

void UNarrativeSystem::SelectDialogueResponse(int32 ResponseIndex)
{
    if (!bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("No active dialogue to respond to."));
        return;
    }

    FDialogueEntry* CurrentEntry = DialogueTable->FindRow<FDialogueEntry>(FName(*CurrentDialogueID), TEXT(""));
    if (!CurrentEntry)
    {
        UE_LOG(LogTemp, Error, TEXT("Current dialogue entry not found: %s"), *CurrentDialogueID);
        return;
    }

    if (ResponseIndex < 0 || ResponseIndex >= CurrentEntry->ResponseOptions.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid response index: %d"), ResponseIndex);
        return;
    }

    // Process the selected response
    FString SelectedResponse = CurrentEntry->ResponseOptions[ResponseIndex];
    UE_LOG(LogTemp, Log, TEXT("Player selected response: %s"), *SelectedResponse);

    // Move to next dialogue if specified
    if (!CurrentEntry->NextDialogueID.IsEmpty())
    {
        StartDialogue(CurrentEntry->NextDialogueID, CurrentSpeaker);
    }
    else
    {
        EndDialogue();
    }
}

void UNarrativeSystem::TriggerNarrativeEvent(const FString& EventID)
{
    FNarrativeEvent* Event = NarrativeEvents.Find(EventID);
    if (!Event)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narrative event not found: %s"), *EventID);
        return;
    }

    if (Event->bIsTriggered)
    {
        UE_LOG(LogTemp, Log, TEXT("Narrative event already triggered: %s"), *EventID);
        return;
    }

    // Check prerequisites
    for (const FString& Prerequisite : Event->Prerequisites)
    {
        if (!IsEventTriggered(Prerequisite))
        {
            UE_LOG(LogTemp, Warning, TEXT("Prerequisite not met for event %s: %s"), *EventID, *Prerequisite);
            return;
        }
    }

    Event->bIsTriggered = true;
    OnNarrativeEvent.Broadcast(EventID, Event->EventDescription);
    
    UE_LOG(LogTemp, Log, TEXT("Triggered narrative event: %s"), *EventID);
}

bool UNarrativeSystem::IsEventTriggered(const FString& EventID) const
{
    const FNarrativeEvent* Event = NarrativeEvents.Find(EventID);
    return Event ? Event->bIsTriggered : false;
}

FDialogueEntry UNarrativeSystem::GetDialogueEntry(const FString& DialogueID) const
{
    if (!DialogueTable)
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueTable is null"));
        return FDialogueEntry();
    }

    FDialogueEntry* Entry = DialogueTable->FindRow<FDialogueEntry>(FName(*DialogueID), TEXT(""));
    return Entry ? *Entry : FDialogueEntry();
}

void UNarrativeSystem::LoadDialogueTable(UDataTable* DialogueDataTable)
{
    DialogueTable = DialogueDataTable;
    UE_LOG(LogTemp, Log, TEXT("Dialogue table loaded successfully"));
}

void UNarrativeSystem::InitializeNarrativeEvents()
{
    // Initialize core narrative events
    FNarrativeEvent TutorialComplete;
    TutorialComplete.EventID = "tutorial_complete";
    TutorialComplete.EventDescription = FText::FromString("The player has completed the tutorial");
    TutorialComplete.bIsTriggered = false;
    NarrativeEvents.Add(TutorialComplete.EventID, TutorialComplete);

    FNarrativeEvent FirstHunt;
    FirstHunt.EventID = "first_hunt";
    FirstHunt.EventDescription = FText::FromString("The player has completed their first hunt");
    FirstHunt.bIsTriggered = false;
    FirstHunt.Prerequisites.Add("tutorial_complete");
    NarrativeEvents.Add(FirstHunt.EventID, FirstHunt);

    FNarrativeEvent TribeEncounter;
    TribeEncounter.EventID = "tribe_encounter";
    TribeEncounter.EventDescription = FText::FromString("The player has encountered another tribe");
    TribeEncounter.bIsTriggered = false;
    NarrativeEvents.Add(TribeEncounter.EventID, TribeEncounter);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d narrative events"), NarrativeEvents.Num());
}

void UNarrativeSystem::ProcessDialogueLogic(const FString& DialogueID)
{
    FDialogueEntry* Entry = DialogueTable->FindRow<FDialogueEntry>(FName(*DialogueID), TEXT(""));
    if (!Entry)
    {
        return;
    }

    // Handle quest-related dialogue
    if (Entry->bIsQuestDialogue && !Entry->QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Processing quest dialogue for quest: %s"), *Entry->QuestID);
        // Quest system integration would go here
    }
}