#include "NarrativeManager.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Components/AudioComponent.h"
#include "Sound/DialogueWave.h"
#include "Sound/DialogueVoice.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"
#include "Engine/Engine.h"

UNarrativeManager::UNarrativeManager()
{
    bIsDialogueActive = false;
    CurrentDialogueAudio = nullptr;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initializing narrative system"));
    
    // Initialize default narrative state
    NarrativeState.StoryProgress = 0;
    NarrativeState.CurrentChapter = FGameplayTag::RequestGameplayTag(FName("Story.Chapter.Arrival"));
    
    // Load story events from data table if available
    LoadStoryEventsFromDataTable();
    
    // Register core story events
    RegisterCoreStoryEvents();
}

void UNarrativeManager::Deinitialize()
{
    if (CurrentDialogueAudio && IsValid(CurrentDialogueAudio))
    {
        CurrentDialogueAudio->Stop();
        CurrentDialogueAudio = nullptr;
    }
    
    Super::Deinitialize();
}

void UNarrativeManager::TriggerStoryEvent(FGameplayTag EventTag, const FString& EventData)
{
    if (!EventTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Invalid event tag"));
        return;
    }
    
    // Check if event can be triggered
    if (!CanTriggerEvent(EventTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Cannot trigger event %s - prerequisites not met"), *EventTag.ToString());
        return;
    }
    
    // Check if already completed and not repeatable
    if (IsEventCompleted(EventTag))
    {
        const FStoryEvent* Event = RegisteredEvents.Find(EventTag);
        if (Event && !Event->bIsRepeatable)
        {
            UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Event %s already completed and not repeatable"), *EventTag.ToString());
            return;
        }
    }
    
    // Mark event as completed
    NarrativeState.CompletedEvents.AddUnique(EventTag);
    NarrativeState.ActiveEvents.Remove(EventTag);
    
    // Process event
    if (const FStoryEvent* Event = RegisteredEvents.Find(EventTag))
    {
        ProcessEventUnlocks(*Event);
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Triggered story event: %s - %s"), 
               *EventTag.ToString(), *Event->EventTitle);
    }
    
    // Broadcast event
    OnStoryEventTriggered.Broadcast(EventTag, EventData);
    
    // Auto-advance story progress for major events
    if (EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Story.Major"))))
    {
        AdvanceStoryProgress(10);
    }
    else if (EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Story.Minor"))))
    {
        AdvanceStoryProgress(1);
    }
}

bool UNarrativeManager::IsEventCompleted(FGameplayTag EventTag) const
{
    return NarrativeState.CompletedEvents.Contains(EventTag);
}

bool UNarrativeManager::CanTriggerEvent(FGameplayTag EventTag) const
{
    const FStoryEvent* Event = RegisteredEvents.Find(EventTag);
    if (!Event)
    {
        return false;
    }
    
    // Check prerequisites
    for (const FGameplayTag& Prerequisite : Event->PrerequisiteEvents)
    {
        if (!IsEventCompleted(Prerequisite))
        {
            return false;
        }
    }
    
    return true;
}

void UNarrativeManager::RegisterStoryEvent(const FStoryEvent& NewEvent)
{
    if (NewEvent.EventTag.IsValid())
    {
        RegisteredEvents.Add(NewEvent.EventTag, NewEvent);
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Registered story event: %s"), *NewEvent.EventTag.ToString());
    }
}

void UNarrativeManager::StartDialogue(UDialogueWave* DialogueWave, AActor* Speaker, AActor* Listener)
{
    if (!DialogueWave || !Speaker)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Invalid dialogue wave or speaker"));
        return;
    }
    
    if (bIsDialogueActive)
    {
        EndDialogue();
    }
    
    bIsDialogueActive = true;
    
    // Create audio component for dialogue
    CurrentDialogueAudio = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(), 
        DialogueWave, 
        Speaker->GetActorLocation()
    );
    
    if (CurrentDialogueAudio)
    {
        CurrentDialogueAudio->OnAudioFinished.AddDynamic(this, &UNarrativeManager::OnDialogueAudioFinished);
    }
    
    // Broadcast dialogue started
    OnDialogueStarted.Broadcast(DialogueWave, Speaker, Listener);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Started dialogue from %s"), 
           Speaker ? *Speaker->GetName() : TEXT("Unknown"));
}

void UNarrativeManager::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    bIsDialogueActive = false;
    
    if (CurrentDialogueAudio && IsValid(CurrentDialogueAudio))
    {
        CurrentDialogueAudio->Stop();
        CurrentDialogueAudio = nullptr;
    }
    
    OnDialogueEnded.Broadcast();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Ended dialogue"));
}

void UNarrativeManager::SetCurrentChapter(FGameplayTag ChapterTag)
{
    if (ChapterTag.IsValid())
    {
        NarrativeState.CurrentChapter = ChapterTag;
        
        // Trigger chapter change event
        FGameplayTag ChapterEvent = FGameplayTag::RequestGameplayTag(
            FName(*FString::Printf(TEXT("Story.Chapter.%s.Started"), *ChapterTag.ToString()))
        );
        TriggerStoryEvent(ChapterEvent);
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Set current chapter to %s"), *ChapterTag.ToString());
    }
}

void UNarrativeManager::AdvanceStoryProgress(int32 ProgressAmount)
{
    NarrativeState.StoryProgress += ProgressAmount;
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Advanced story progress by %d (Total: %d)"), 
           ProgressAmount, NarrativeState.StoryProgress);
    
    // Check for chapter advancement
    CheckChapterAdvancement();
}

void UNarrativeManager::SetCharacterRelationship(const FString& CharacterName, const FString& RelationshipStatus)
{
    NarrativeState.CharacterRelationships.Add(CharacterName, RelationshipStatus);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Set %s relationship to %s"), 
           *CharacterName, *RelationshipStatus);
}

FString UNarrativeManager::GetCharacterRelationship(const FString& CharacterName) const
{
    if (const FString* Relationship = NarrativeState.CharacterRelationships.Find(CharacterName))
    {
        return *Relationship;
    }
    return TEXT("Unknown");
}

void UNarrativeManager::LoadStoryEventsFromDataTable()
{
    if (StoryEventsDataTable)
    {
        TArray<FStoryEvent*> StoryEvents;
        StoryEventsDataTable->GetAllRows<FStoryEvent>(TEXT("LoadStoryEvents"), StoryEvents);
        
        for (const FStoryEvent* Event : StoryEvents)
        {
            if (Event && Event->EventTag.IsValid())
            {
                RegisterStoryEvent(*Event);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Loaded %d story events from data table"), StoryEvents.Num());
    }
}

void UNarrativeManager::ProcessEventPrerequisites(const FStoryEvent& Event)
{
    // Prerequisites are checked in CanTriggerEvent
    // This function could be used for additional prerequisite processing
}

void UNarrativeManager::ProcessEventUnlocks(const FStoryEvent& Event)
{
    for (const FGameplayTag& UnlockEvent : Event.UnlockEvents)
    {
        if (UnlockEvent.IsValid() && !NarrativeState.ActiveEvents.Contains(UnlockEvent))
        {
            NarrativeState.ActiveEvents.AddUnique(UnlockEvent);
            UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Unlocked event %s"), *UnlockEvent.ToString());
        }
    }
}

void UNarrativeManager::OnDialogueAudioFinished()
{
    EndDialogue();
}

void UNarrativeManager::RegisterCoreStoryEvents()
{
    // Register core story events for the prehistoric survival game
    
    // Chapter 1: Arrival
    FStoryEvent ArrivalEvent;
    ArrivalEvent.EventTag = FGameplayTag::RequestGameplayTag(FName("Story.Major.Arrival"));
    ArrivalEvent.EventTitle = TEXT("Arrival in the Past");
    ArrivalEvent.EventDescription = TEXT("The paleontologist touches the mysterious gem and is transported to prehistoric times");
    ArrivalEvent.bIsRepeatable = false;
    ArrivalEvent.Priority = 100;
    RegisterStoryEvent(ArrivalEvent);
    
    // First Dinosaur Encounter
    FStoryEvent FirstDinoEvent;
    FirstDinoEvent.EventTag = FGameplayTag::RequestGameplayTag(FName("Story.Major.FirstDinosaur"));
    FirstDinoEvent.EventTitle = TEXT("First Dinosaur Encounter");
    FirstDinoEvent.EventDescription = TEXT("Player encounters their first living dinosaur");
    FirstDinoEvent.PrerequisiteEvents.Add(ArrivalEvent.EventTag);
    FirstDinoEvent.bIsRepeatable = false;
    FirstDinoEvent.Priority = 90;
    RegisterStoryEvent(FirstDinoEvent);
    
    // First Shelter
    FStoryEvent FirstShelterEvent;
    FirstShelterEvent.EventTag = FGameplayTag::RequestGameplayTag(FName("Story.Minor.FirstShelter"));
    FirstShelterEvent.EventTitle = TEXT("First Shelter Built");
    FirstShelterEvent.EventDescription = TEXT("Player constructs their first shelter for protection");
    FirstShelterEvent.PrerequisiteEvents.Add(FirstDinoEvent.EventTag);
    FirstShelterEvent.bIsRepeatable = false;
    FirstShelterEvent.Priority = 80;
    RegisterStoryEvent(FirstShelterEvent);
    
    // Discovery of the Return Gem
    FStoryEvent GemDiscoveryEvent;
    GemDiscoveryEvent.EventTag = FGameplayTag::RequestGameplayTag(FName("Story.Major.GemDiscovery"));
    GemDiscoveryEvent.EventTitle = TEXT("Discovery of the Return Gem");
    GemDiscoveryEvent.EventDescription = TEXT("Player discovers clues about the gem that can return them home");
    GemDiscoveryEvent.PrerequisiteEvents.Add(FirstShelterEvent.EventTag);
    GemDiscoveryEvent.bIsRepeatable = false;
    GemDiscoveryEvent.Priority = 70;
    RegisterStoryEvent(GemDiscoveryEvent);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Registered core story events"));
}

void UNarrativeManager::CheckChapterAdvancement()
{
    // Check if story progress warrants chapter advancement
    if (NarrativeState.StoryProgress >= 100 && 
        NarrativeState.CurrentChapter == FGameplayTag::RequestGameplayTag(FName("Story.Chapter.Arrival")))
    {
        SetCurrentChapter(FGameplayTag::RequestGameplayTag(FName("Story.Chapter.Survival")));
    }
    else if (NarrativeState.StoryProgress >= 250 && 
             NarrativeState.CurrentChapter == FGameplayTag::RequestGameplayTag(FName("Story.Chapter.Survival")))
    {
        SetCurrentChapter(FGameplayTag::RequestGameplayTag(FName("Story.Chapter.Discovery")));
    }
    else if (NarrativeState.StoryProgress >= 500 && 
             NarrativeState.CurrentChapter == FGameplayTag::RequestGameplayTag(FName("Story.Chapter.Discovery")))
    {
        SetCurrentChapter(FGameplayTag::RequestGameplayTag(FName("Story.Chapter.Return")));
    }
}