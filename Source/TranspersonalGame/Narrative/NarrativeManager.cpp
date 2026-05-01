#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Engine/World.h"

UNarrativeManager::UNarrativeManager()
{
    CurrentDialogueState = ENarr_DialogueState::Inactive;
    CurrentDialogueIndex = 0;
    DialogueTimer = 0.0f;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initializing narrative system"));
    
    InitializeStoryEvents();
    CurrentDialogueState = ENarr_DialogueState::Inactive;
    CurrentDialogueIndex = 0;
}

void UNarrativeManager::Deinitialize()
{
    StopCurrentDialogue();
    Super::Deinitialize();
}

void UNarrativeManager::InitializeStoryEvents()
{
    StoryEvents.Empty();

    // First water discovery event
    FNarr_StoryEvent WaterEvent;
    WaterEvent.EventType = ENarr_NarrativeEvent::FirstWaterFound;
    WaterEvent.EventDescription = TEXT("Player discovers first water source");
    
    FNarr_DialogueLine WaterThought;
    WaterThought.SpeakerName = TEXT("Player");
    WaterThought.DialogueText = TEXT("Water... finally. But I must be careful - predators hunt near water sources.");
    WaterThought.bIsPlayerThought = true;
    WaterThought.Duration = 4.0f;
    WaterEvent.TriggerDialogue.Add(WaterThought);
    
    StoryEvents.Add(WaterEvent);

    // First dinosaur encounter
    FNarr_StoryEvent DinoEvent;
    DinoEvent.EventType = ENarr_NarrativeEvent::FirstDinosaurEncounter;
    DinoEvent.EventDescription = TEXT("Player's first close dinosaur encounter");
    
    FNarr_DialogueLine DinoThought;
    DinoThought.SpeakerName = TEXT("Player");
    DinoThought.DialogueText = TEXT("Magnificent... and terrifying. These creatures ruled this world long before humans existed.");
    DinoThought.bIsPlayerThought = true;
    DinoThought.Duration = 5.0f;
    DinoEvent.TriggerDialogue.Add(DinoThought);
    
    StoryEvents.Add(DinoEvent);

    // T-Rex encounter
    FNarr_StoryEvent TRexEvent;
    TRexEvent.EventType = ENarr_NarrativeEvent::TRexEncounter;
    TRexEvent.EventDescription = TEXT("Player encounters the apex predator");
    
    FNarr_DialogueLine TRexWarning;
    TRexWarning.SpeakerName = TEXT("Narrator");
    TRexWarning.DialogueText = TEXT("The earth trembles. The king of predators has arrived. Stay perfectly still.");
    TRexWarning.bIsPlayerThought = false;
    TRexWarning.Duration = 6.0f;
    TRexEvent.TriggerDialogue.Add(TRexWarning);
    
    StoryEvents.Add(TRexEvent);

    // Raptor pack sighting
    FNarr_StoryEvent RaptorEvent;
    RaptorEvent.EventType = ENarr_NarrativeEvent::RaptorPackSighting;
    RaptorEvent.EventDescription = TEXT("Player spots a raptor hunting pack");
    
    FNarr_DialogueLine RaptorWarning;
    RaptorWarning.SpeakerName = TEXT("Narrator");
    RaptorWarning.DialogueText = TEXT("The pack hunters move with deadly coordination. Intelligence and instinct combined.");
    RaptorWarning.bIsPlayerThought = false;
    RaptorWarning.Duration = 5.0f;
    RaptorEvent.TriggerDialogue.Add(RaptorWarning);
    
    StoryEvents.Add(RaptorEvent);

    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initialized %d story events"), StoryEvents.Num());
}

void UNarrativeManager::PlayDialogue(const TArray<FNarr_DialogueLine>& DialogueLines)
{
    if (DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Cannot play empty dialogue"));
        return;
    }

    // Stop any current dialogue
    StopCurrentDialogue();

    CurrentDialogue = DialogueLines;
    CurrentDialogueIndex = 0;
    CurrentDialogueState = ENarr_DialogueState::Playing;

    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Starting dialogue with %d lines"), DialogueLines.Num());

    // Start first dialogue line
    AdvanceDialogue();
}

void UNarrativeManager::StopCurrentDialogue()
{
    if (CurrentDialogueState != ENarr_DialogueState::Inactive)
    {
        CurrentDialogueState = ENarr_DialogueState::Inactive;
        CurrentDialogue.Empty();
        CurrentDialogueIndex = 0;

        // Clear timer
        if (GetWorld() && DialogueTimerHandle.IsValid())
        {
            GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
        }

        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue stopped"));
    }
}

bool UNarrativeManager::IsDialoguePlaying() const
{
    return CurrentDialogueState == ENarr_DialogueState::Playing;
}

void UNarrativeManager::AdvanceDialogue()
{
    if (CurrentDialogueIndex >= CurrentDialogue.Num())
    {
        // Dialogue finished
        CurrentDialogueState = ENarr_DialogueState::Completed;
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue completed"));
        StopCurrentDialogue();
        return;
    }

    const FNarr_DialogueLine& CurrentLine = CurrentDialogue[CurrentDialogueIndex];
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Playing line %d: %s - %s"), 
           CurrentDialogueIndex, *CurrentLine.SpeakerName, *CurrentLine.DialogueText);

    // Set timer for next line
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            DialogueTimerHandle,
            [this]() { 
                CurrentDialogueIndex++;
                AdvanceDialogue();
            },
            CurrentLine.Duration,
            false
        );
    }
}

void UNarrativeManager::TriggerNarrativeEvent(ENarr_NarrativeEvent EventType)
{
    FNarr_StoryEvent* Event = FindStoryEvent(EventType);
    if (!Event)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Event type %d not found"), (int32)EventType);
        return;
    }

    if (Event->bHasTriggered)
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Event %d already triggered"), (int32)EventType);
        return;
    }

    Event->bHasTriggered = true;
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Triggering event: %s"), *Event->EventDescription);

    // Play associated dialogue
    if (Event->TriggerDialogue.Num() > 0)
    {
        PlayDialogue(Event->TriggerDialogue);
    }
}

bool UNarrativeManager::HasEventTriggered(ENarr_NarrativeEvent EventType) const
{
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventType == EventType)
        {
            return Event.bHasTriggered;
        }
    }
    return false;
}

void UNarrativeManager::PlaySurvivalTip(const FString& TipText, const FString& AudioPath)
{
    TArray<FNarr_DialogueLine> TipDialogue;
    
    FNarr_DialogueLine Tip;
    Tip.SpeakerName = TEXT("Survival Guide");
    Tip.DialogueText = TipText;
    Tip.AudioFilePath = AudioPath;
    Tip.Duration = 8.0f;
    Tip.bIsPlayerThought = false;
    
    TipDialogue.Add(Tip);
    PlayDialogue(TipDialogue);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Playing survival tip: %s"), *TipText);
}

void UNarrativeManager::PlayPlayerThought(const FString& ThoughtText, const FString& AudioPath)
{
    TArray<FNarr_DialogueLine> ThoughtDialogue;
    
    FNarr_DialogueLine Thought;
    Thought.SpeakerName = TEXT("Player");
    Thought.DialogueText = ThoughtText;
    Thought.AudioFilePath = AudioPath;
    Thought.Duration = 6.0f;
    Thought.bIsPlayerThought = true;
    
    ThoughtDialogue.Add(Thought);
    PlayDialogue(ThoughtDialogue);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Playing player thought: %s"), *ThoughtText);
}

void UNarrativeManager::PlayEnvironmentalNarration(const FString& LocationName, const FString& NarrationText)
{
    TArray<FNarr_DialogueLine> EnvironmentDialogue;
    
    FNarr_DialogueLine Narration;
    Narration.SpeakerName = TEXT("Environment");
    Narration.DialogueText = FString::Printf(TEXT("%s: %s"), *LocationName, *NarrationText);
    Narration.Duration = 7.0f;
    Narration.bIsPlayerThought = false;
    
    EnvironmentDialogue.Add(Narration);
    PlayDialogue(EnvironmentDialogue);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Playing environmental narration for %s"), *LocationName);
}

FNarr_StoryEvent* UNarrativeManager::FindStoryEvent(ENarr_NarrativeEvent EventType)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventType == EventType)
        {
            return &Event;
        }
    }
    return nullptr;
}