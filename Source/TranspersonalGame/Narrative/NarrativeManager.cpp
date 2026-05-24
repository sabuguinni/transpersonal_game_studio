#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNarrativeManager::UNarrativeManager()
{
    bIsDialogueActive = false;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultEvents();
    LoadStoryData();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager initialized with %d story events"), StoryEvents.Num());
}

void UNarrativeManager::Deinitialize()
{
    SaveStoryProgress();
    StoryEvents.Empty();
    
    Super::Deinitialize();
}

void UNarrativeManager::TriggerStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bIsCompleted)
        {
            UE_LOG(LogTemp, Log, TEXT("Triggering story event: %s"), *EventID);
            
            // Play all dialogues in sequence
            for (const FNarr_DialogueEntry& Dialogue : Event.Dialogues)
            {
                PlayDialogue(Dialogue);
            }
            
            // Mark event as completed
            Event.bIsCompleted = true;
            break;
        }
    }
}

void UNarrativeManager::PlayDialogue(const FNarr_DialogueEntry& DialogueEntry)
{
    CurrentDialogue = DialogueEntry;
    bIsDialogueActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue - Speaker: %s, Text: %s"), 
           *DialogueEntry.SpeakerName, *DialogueEntry.DialogueText);
    
    // Display dialogue on screen (basic implementation)
    if (GEngine)
    {
        FString DisplayText = DialogueEntry.bIsNarration ? 
            DialogueEntry.DialogueText : 
            FString::Printf(TEXT("%s: %s"), *DialogueEntry.SpeakerName, *DialogueEntry.DialogueText);
            
        GEngine->AddOnScreenDebugMessage(-1, DialogueEntry.Duration, FColor::Yellow, DisplayText);
    }
    
    // Schedule dialogue end
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        bIsDialogueActive = false;
    }, DialogueEntry.Duration, false);
}

void UNarrativeManager::RegisterStoryEvent(const FNarr_StoryEvent& NewEvent)
{
    // Check if event already exists
    for (const FNarr_StoryEvent& ExistingEvent : StoryEvents)
    {
        if (ExistingEvent.EventID == NewEvent.EventID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Story event %s already exists"), *NewEvent.EventID);
            return;
        }
    }
    
    StoryEvents.Add(NewEvent);
    UE_LOG(LogTemp, Log, TEXT("Registered new story event: %s"), *NewEvent.EventID);
}

bool UNarrativeManager::IsEventCompleted(const FString& EventID) const
{
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return Event.bIsCompleted;
        }
    }
    return false;
}

void UNarrativeManager::SetEventCompleted(const FString& EventID, bool bCompleted)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            Event.bIsCompleted = bCompleted;
            UE_LOG(LogTemp, Log, TEXT("Set event %s completed: %s"), *EventID, bCompleted ? TEXT("true") : TEXT("false"));
            break;
        }
    }
}

TArray<FNarr_StoryEvent> UNarrativeManager::GetActiveEvents() const
{
    TArray<FNarr_StoryEvent> ActiveEvents;
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bIsCompleted)
        {
            ActiveEvents.Add(Event);
        }
    }
    
    return ActiveEvents;
}

void UNarrativeManager::InitializeDefaultEvents()
{
    // First Contact Event
    FNarr_StoryEvent FirstContactEvent;
    FirstContactEvent.EventID = TEXT("first_contact");
    FirstContactEvent.EventDescription = TEXT("Player's first encounter with the prehistoric world");
    
    FNarr_DialogueEntry NarratorIntro;
    NarratorIntro.SpeakerName = TEXT("Narrator");
    NarratorIntro.DialogueText = TEXT("The ancient valley holds many secrets, survivor. Listen carefully to the wind - it carries the scent of predators and the promise of shelter.");
    NarratorIntro.Duration = 5.0f;
    NarratorIntro.bIsNarration = true;
    FirstContactEvent.Dialogues.Add(NarratorIntro);
    
    RegisterStoryEvent(FirstContactEvent);
    
    // Predator Warning Event
    FNarr_StoryEvent PredatorEvent;
    PredatorEvent.EventID = TEXT("predator_warning");
    PredatorEvent.EventDescription = TEXT("Warning about nearby predators");
    
    FNarr_DialogueEntry Warning;
    Warning.SpeakerName = TEXT("Survival Guide");
    Warning.DialogueText = TEXT("Warning! Pack hunters detected in the eastern ravines. Their coordinated movements suggest they are tracking prey. Find high ground immediately.");
    Warning.Duration = 4.0f;
    Warning.bIsNarration = false;
    PredatorEvent.Dialogues.Add(Warning);
    
    RegisterStoryEvent(PredatorEvent);
    
    // Resource Discovery Event
    FNarr_StoryEvent ResourceEvent;
    ResourceEvent.EventID = TEXT("resource_discovery");
    ResourceEvent.EventDescription = TEXT("Discovery of essential survival resources");
    
    FNarr_DialogueEntry ResourceTip;
    ResourceTip.SpeakerName = TEXT("Narrator");
    ResourceTip.DialogueText = TEXT("Fresh water flows from the rocky outcrop ahead. Mark this location well - survival depends on knowing where life-giving resources can be found.");
    ResourceTip.Duration = 4.5f;
    ResourceTip.bIsNarration = true;
    ResourceEvent.Dialogues.Add(ResourceTip);
    
    RegisterStoryEvent(ResourceEvent);
}

void UNarrativeManager::LoadStoryData()
{
    // TODO: Implement save/load system for story progress
    UE_LOG(LogTemp, Log, TEXT("Story data loading not yet implemented"));
}

void UNarrativeManager::SaveStoryProgress()
{
    // TODO: Implement save system for story progress
    UE_LOG(LogTemp, Log, TEXT("Story progress saving not yet implemented"));
}