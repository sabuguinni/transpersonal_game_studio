#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsPlayingDialogue = false;
    InitializeStoryEvents();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager initialized with %d story events"), StoryEvents.Num());
}

void UNarrativeManager::Deinitialize()
{
    StoryEvents.Empty();
    CompletedEvents.Empty();
    
    Super::Deinitialize();
}

void UNarrativeManager::InitializeStoryEvents()
{
    // Initialize core survival story events
    FNarr_StoryEvent IntroEvent;
    IntroEvent.EventID = TEXT("game_intro");
    IntroEvent.EventDescription = FText::FromString(TEXT("Player awakens in the prehistoric valley"));
    
    FNarr_DialogueLine IntroLine;
    IntroLine.SpeakerName = TEXT("Tribal Elder");
    IntroLine.DialogueText = FText::FromString(TEXT("The ancient valley holds many secrets, survivor. Stay alert and trust your instincts."));
    IntroLine.Duration = 6.0f;
    IntroEvent.DialogueLines.Add(IntroLine);
    
    StoryEvents.Add(IntroEvent);

    // First dinosaur encounter
    FNarr_StoryEvent DinosaurEvent;
    DinosaurEvent.EventID = TEXT("first_dinosaur");
    DinosaurEvent.EventDescription = FText::FromString(TEXT("Player encounters their first dinosaur"));
    DinosaurEvent.RequiredEvents.Add(TEXT("game_intro"));
    
    FNarr_DialogueLine DinosaurLine;
    DinosaurLine.SpeakerName = TEXT("Scout");
    DinosaurLine.DialogueText = FText::FromString(TEXT("Warning! Thunder Lizard territory ahead. Avoid open ground when the earth trembles."));
    DinosaurLine.Duration = 5.0f;
    DinosaurEvent.DialogueLines.Add(DinosaurLine);
    
    StoryEvents.Add(DinosaurEvent);

    // Survival milestone
    FNarr_StoryEvent SurvivalEvent;
    SurvivalEvent.EventID = TEXT("survival_milestone");
    SurvivalEvent.EventDescription = FText::FromString(TEXT("Player survives first day"));
    
    FNarr_DialogueLine SurvivalLine;
    SurvivalLine.SpeakerName = TEXT("Veteran Hunter");
    SurvivalLine.DialogueText = FText::FromString(TEXT("You have learned to read the signs of danger. The valley respects those who respect it."));
    SurvivalLine.Duration = 5.5f;
    SurvivalEvent.DialogueLines.Add(SurvivalLine);
    
    StoryEvents.Add(SurvivalEvent);
}

void UNarrativeManager::TriggerStoryEvent(const FString& EventID)
{
    FNarr_StoryEvent* Event = FindStoryEvent(EventID);
    if (!Event)
    {
        UE_LOG(LogTemp, Warning, TEXT("Story event not found: %s"), *EventID);
        return;
    }

    // Check if prerequisites are met
    for (const FString& RequiredEvent : Event->RequiredEvents)
    {
        if (!IsEventCompleted(RequiredEvent))
        {
            UE_LOG(LogTemp, Warning, TEXT("Prerequisites not met for event: %s"), *EventID);
            return;
        }
    }

    // Play dialogue sequence if available
    if (Event->DialogueLines.Num() > 0)
    {
        PlayDialogueSequence(Event->DialogueLines);
    }

    // Mark event as completed
    CompleteEvent(EventID);
    
    UE_LOG(LogTemp, Log, TEXT("Story event triggered: %s"), *EventID);
}

bool UNarrativeManager::IsEventCompleted(const FString& EventID) const
{
    return CompletedEvents.Contains(EventID);
}

void UNarrativeManager::CompleteEvent(const FString& EventID)
{
    if (!CompletedEvents.Contains(EventID))
    {
        CompletedEvents.Add(EventID);
        UE_LOG(LogTemp, Log, TEXT("Story event completed: %s"), *EventID);
    }
}

void UNarrativeManager::PlayDialogue(const FNarr_DialogueLine& DialogueLine)
{
    if (bIsPlayingDialogue)
    {
        return;
    }

    CurrentDialogue = DialogueLine;
    bIsPlayingDialogue = true;

    // Log dialogue for debugging
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue: [%s] %s"), 
           *DialogueLine.SpeakerName, 
           *DialogueLine.DialogueText.ToString());

    // TODO: Integrate with UI system to display dialogue
    // TODO: Play audio if AudioURL is provided
    
    // Simulate dialogue duration
    FTimerHandle DialogueTimer;
    GetWorld()->GetTimerManager().SetTimer(DialogueTimer, [this]()
    {
        bIsPlayingDialogue = false;
    }, CurrentDialogue.Duration, false);
}

void UNarrativeManager::PlayDialogueSequence(const TArray<FNarr_DialogueLine>& DialogueSequence)
{
    if (DialogueSequence.Num() == 0 || bIsPlayingDialogue)
    {
        return;
    }

    // Play first dialogue line
    PlayDialogue(DialogueSequence[0]);

    // Schedule remaining lines
    float TotalDelay = DialogueSequence[0].Duration;
    for (int32 i = 1; i < DialogueSequence.Num(); i++)
    {
        FTimerHandle SequenceTimer;
        const FNarr_DialogueLine LineCopy = DialogueSequence[i];
        
        GetWorld()->GetTimerManager().SetTimer(SequenceTimer, [this, LineCopy]()
        {
            PlayDialogue(LineCopy);
        }, TotalDelay, false);
        
        TotalDelay += DialogueSequence[i].Duration;
    }
}

void UNarrativeManager::OnPlayerSurvivalStateChanged(float Health, float Hunger, float Thirst, float Fear)
{
    // Trigger narrative events based on survival state
    if (Health < 0.3f && !IsEventCompleted(TEXT("low_health_warning")))
    {
        FNarr_DialogueLine WarningLine;
        WarningLine.SpeakerName = TEXT("Inner Voice");
        WarningLine.DialogueText = FText::FromString(TEXT("Your body grows weak. Find shelter and tend to your wounds."));
        WarningLine.Duration = 4.0f;
        PlayDialogue(WarningLine);
        
        CompleteEvent(TEXT("low_health_warning"));
    }

    if (Fear > 0.8f && !IsEventCompleted(TEXT("high_fear_state")))
    {
        FNarr_DialogueLine FearLine;
        FearLine.SpeakerName = TEXT("Inner Voice");
        FearLine.DialogueText = FText::FromString(TEXT("Terror grips your heart. Breathe deeply and find your courage."));
        FearLine.Duration = 4.5f;
        PlayDialogue(FearLine);
        
        CompleteEvent(TEXT("high_fear_state"));
    }
}

void UNarrativeManager::OnDinosaurEncounter(const FString& DinosaurType, float Distance, bool bIsHostile)
{
    if (DinosaurType.Contains(TEXT("TRex")) && Distance < 2000.0f)
    {
        if (!IsEventCompleted(TEXT("trex_encounter")))
        {
            TriggerStoryEvent(TEXT("first_dinosaur"));
            CompleteEvent(TEXT("trex_encounter"));
        }
    }
    else if (DinosaurType.Contains(TEXT("Raptor")) && bIsHostile)
    {
        FNarr_DialogueLine RaptorLine;
        RaptorLine.SpeakerName = TEXT("Veteran Hunter");
        RaptorLine.DialogueText = FText::FromString(TEXT("Pack hunters! They coordinate their attacks. Seek high ground immediately."));
        RaptorLine.Duration = 4.0f;
        PlayDialogue(RaptorLine);
    }
}

void UNarrativeManager::OnBiomeEntered(const FString& BiomeName)
{
    FString EventID = FString::Printf(TEXT("entered_%s"), *BiomeName.ToLower());
    
    if (!IsEventCompleted(EventID))
    {
        FNarr_DialogueLine BiomeLine;
        BiomeLine.SpeakerName = TEXT("Survival Guide");
        
        if (BiomeName.Contains(TEXT("Swamp")))
        {
            BiomeLine.DialogueText = FText::FromString(TEXT("The marshlands hide both danger and resources. Watch for quicksand and predators."));
        }
        else if (BiomeName.Contains(TEXT("Forest")))
        {
            BiomeLine.DialogueText = FText::FromString(TEXT("Dense foliage provides cover but limits visibility. Listen for the sounds of the hunt."));
        }
        else if (BiomeName.Contains(TEXT("Desert")))
        {
            BiomeLine.DialogueText = FText::FromString(TEXT("The harsh sun and scarce water test even the strongest survivors."));
        }
        else if (BiomeName.Contains(TEXT("Mountain")))
        {
            BiomeLine.DialogueText = FText::FromString(TEXT("High peaks offer safety and vantage points, but the climb is treacherous."));
        }
        else
        {
            BiomeLine.DialogueText = FText::FromString(TEXT("You have entered new territory. Stay alert for signs of danger."));
        }
        
        BiomeLine.Duration = 5.0f;
        PlayDialogue(BiomeLine);
        CompleteEvent(EventID);
    }
}

void UNarrativeManager::LoadDialogueAudio(const FString& AudioURL)
{
    // TODO: Implement audio loading from URL
    // This would integrate with the ElevenLabs TTS system
    UE_LOG(LogTemp, Log, TEXT("Loading dialogue audio from: %s"), *AudioURL);
}

FNarr_StoryEvent* UNarrativeManager::FindStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return &Event;
        }
    }
    return nullptr;
}