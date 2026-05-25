#include "DialogueSystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UDialogueSystem::UDialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentDialogueIndex = 0;
    bIsDialogueActive = false;
}

void UDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    LoadNarrativeData();
}

void UDialogueSystem::StartDialogue(const FString& EventID)
{
    for (const FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventID == EventID && Event.DialogueSequence.Num() > 0)
        {
            DialogueEntries = Event.DialogueSequence;
            CurrentDialogueIndex = 0;
            bIsDialogueActive = true;
            
            UE_LOG(LogTemp, Warning, TEXT("Started dialogue for event: %s"), *EventID);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dialogue event not found: %s"), *EventID);
}

void UDialogueSystem::NextDialogue()
{
    if (!bIsDialogueActive || DialogueEntries.Num() == 0)
    {
        return;
    }

    CurrentDialogueIndex++;
    
    if (CurrentDialogueIndex >= DialogueEntries.Num())
    {
        EndDialogue();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Advanced to dialogue index: %d"), CurrentDialogueIndex);
    }
}

void UDialogueSystem::EndDialogue()
{
    bIsDialogueActive = false;
    CurrentDialogueIndex = 0;
    DialogueEntries.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

FNarr_DialogueEntry UDialogueSystem::GetCurrentDialogue() const
{
    if (bIsDialogueActive && DialogueEntries.IsValidIndex(CurrentDialogueIndex))
    {
        return DialogueEntries[CurrentDialogueIndex];
    }
    
    return FNarr_DialogueEntry();
}

void UDialogueSystem::TriggerNarrativeEvent(const FString& EventID, const FVector& PlayerLocation)
{
    for (FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventID == EventID && !Event.bIsTriggered)
        {
            Event.bIsTriggered = true;
            StartDialogue(EventID);
            
            UE_LOG(LogTemp, Warning, TEXT("Triggered narrative event: %s at location: %s"), 
                *EventID, *PlayerLocation.ToString());
            return;
        }
    }
}

void UDialogueSystem::AddDialogueEntry(const FNarr_DialogueEntry& NewEntry)
{
    DialogueEntries.Add(NewEntry);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue entry from: %s"), *NewEntry.SpeakerName);
}

void UDialogueSystem::LoadNarrativeData()
{
    // Initialize with basic survival narrative events
    FNarr_NarrativeEvent IntroEvent;
    IntroEvent.EventID = TEXT("game_intro");
    IntroEvent.EventDescription = FText::FromString(TEXT("Player awakens in the prehistoric valley"));
    IntroEvent.bIsTriggered = false;
    IntroEvent.TriggerRadius = 1000.0f;

    FNarr_DialogueEntry IntroDialogue;
    IntroDialogue.SpeakerName = TEXT("Narrator");
    IntroDialogue.DialogueText = FText::FromString(TEXT("The ancient valley holds many secrets, survivor. Listen carefully to the wind - it carries the scent of predators and the promise of danger."));
    IntroDialogue.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1779676657904_Narrator.mp3");
    IntroDialogue.Duration = 17.0f;

    IntroEvent.DialogueSequence.Add(IntroDialogue);
    RegisterNarrativeEvent(IntroEvent);

    // Pack hunter warning event
    FNarr_NarrativeEvent PackWarningEvent;
    PackWarningEvent.EventID = TEXT("pack_hunter_warning");
    PackWarningEvent.EventDescription = FText::FromString(TEXT("Player enters raptor territory"));
    PackWarningEvent.bIsTriggered = false;
    PackWarningEvent.TriggerRadius = 2000.0f;

    FNarr_DialogueEntry WarningDialogue;
    WarningDialogue.SpeakerName = TEXT("SurvivalGuide");
    WarningDialogue.DialogueText = FText::FromString(TEXT("Warning! Pack hunters detected in the eastern ravines. Their coordinated movements suggest they are tracking prey."));
    WarningDialogue.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1779676664062_SurvivalGuide.mp3");
    WarningDialogue.Duration = 15.0f;

    PackWarningEvent.DialogueSequence.Add(WarningDialogue);
    RegisterNarrativeEvent(PackWarningEvent);

    UE_LOG(LogTemp, Warning, TEXT("Loaded %d narrative events"), NarrativeEvents.Num());
}

void UDialogueSystem::RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event)
{
    NarrativeEvents.Add(Event);
    UE_LOG(LogTemp, Log, TEXT("Registered narrative event: %s"), *Event.EventID);
}