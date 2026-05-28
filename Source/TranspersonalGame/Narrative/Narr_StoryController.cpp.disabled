#include "Narr_StoryController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

ANarr_StoryController::ANarr_StoryController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize timing
    PhaseTransitionCooldown = 60.0f;
    LastEventTime = 0.0f;

    // Initialize player progress
    PlayerProgress.CurrentPhase = ENarr_StoryPhase::Awakening;
    PlayerProgress.EmotionalState = ENarr_EmotionalState::Terror;
    PlayerProgress.DinosaurEncounters = 0;
    PlayerProgress.SuccessfulObservations = 0;
    PlayerProgress.NearDeathExperiences = 0;
    PlayerProgress.SurvivalTime = 0.0f;
}

void ANarr_StoryController::BeginPlay()
{
    Super::BeginPlay();

    // Find player character
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerCharacter = Cast<ATranspersonalCharacter>(PC->GetPawn());
        }
    }

    // Initialize story events
    InitializeStoryEvents();

    // Register audio cues (URLs from previous TTS generation)
    RegisterAudioCue(TEXT("QuestComplete"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777737474863_QuestNarrator.mp3"));
    RegisterAudioCue(TEXT("PredatorWarning"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777737476980_FieldResearcher.mp3"));
    RegisterAudioCue(TEXT("TerritoryDiscovery"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777737479203_SeniorScientist.mp3"));
    RegisterAudioCue(TEXT("MissionUpdate"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777737481434_MissionController.mp3"));

    UE_LOG(LogTemp, Warning, TEXT("Narrative Story Controller initialized"));
}

void ANarr_StoryController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update survival time
    UpdateSurvivalTime(DeltaTime);

    // Update active events
    UpdateActiveEvents();

    // Check location-based events
    if (PlayerCharacter)
    {
        CheckLocationBasedEvents(PlayerCharacter->GetActorLocation());
    }
}

void ANarr_StoryController::InitializeStoryEvents()
{
    StoryEvents.Empty();

    // Awakening phase events
    FNarr_StoryEvent AwakeningEvent;
    AwakeningEvent.EventName = TEXT("First Awakening");
    AwakeningEvent.EventDescription = TEXT("You wake up in a strange, ancient world filled with sounds you've never heard before.");
    AwakeningEvent.RequiredPhase = ENarr_StoryPhase::Awakening;
    AwakeningEvent.TriggerLocation = FVector(0, 0, 100);
    AwakeningEvent.TriggerRadius = 200.0f;
    AwakeningEvent.AudioCueName = TEXT("QuestComplete");
    StoryEvents.Add(AwakeningEvent);

    // First contact events
    FNarr_StoryEvent FirstContactEvent;
    FirstContactEvent.EventName = TEXT("First Dinosaur Sighting");
    FirstContactEvent.EventDescription = TEXT("A massive creature moves through the trees ahead. Your heart races as you realize you're not alone.");
    FirstContactEvent.RequiredPhase = ENarr_StoryPhase::FirstContact;
    FirstContactEvent.TriggerLocation = FVector(1000, 500, 150);
    FirstContactEvent.TriggerRadius = 800.0f;
    FirstContactEvent.AudioCueName = TEXT("PredatorWarning");
    StoryEvents.Add(FirstContactEvent);

    // Territory mapping events
    FNarr_StoryEvent TerritoryEvent;
    TerritoryEvent.EventName = TEXT("Predator Territory Discovery");
    TerritoryEvent.EventDescription = TEXT("Massive claw marks on the trees and scattered bones mark this as dangerous territory.");
    TerritoryEvent.RequiredPhase = ENarr_StoryPhase::TerritoryMapping;
    TerritoryEvent.TriggerLocation = FVector(-1500, 2000, 200);
    TerritoryEvent.TriggerRadius = 600.0f;
    TerritoryEvent.AudioCueName = TEXT("TerritoryDiscovery");
    StoryEvents.Add(TerritoryEvent);

    // Herd observation events
    FNarr_StoryEvent HerdEvent;
    HerdEvent.EventName = TEXT("Herbivore Herd Gathering");
    HerdEvent.EventDescription = TEXT("A peaceful herd gathers at the water source. Their coordinated movements suggest complex social behavior.");
    HerdEvent.RequiredPhase = ENarr_StoryPhase::HerdObservation;
    HerdEvent.TriggerLocation = FVector(0, -2000, 100);
    HerdEvent.TriggerRadius = 1000.0f;
    HerdEvent.AudioCueName = TEXT("MissionUpdate");
    StoryEvents.Add(HerdEvent);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d story events"), StoryEvents.Num());
}

void ANarr_StoryController::AdvanceStoryPhase(ENarr_StoryPhase NewPhase)
{
    if (PlayerProgress.CurrentPhase != NewPhase)
    {
        ENarr_StoryPhase OldPhase = PlayerProgress.CurrentPhase;
        PlayerProgress.CurrentPhase = NewPhase;

        // Trigger blueprint event
        OnPhaseTransition(OldPhase, NewPhase);

        // Update emotional state based on phase
        switch (NewPhase)
        {
            case ENarr_StoryPhase::Awakening:
                UpdateEmotionalState(ENarr_EmotionalState::Terror);
                break;
            case ENarr_StoryPhase::FirstContact:
                UpdateEmotionalState(ENarr_EmotionalState::Anxiety);
                break;
            case ENarr_StoryPhase::LearningToSurvive:
                UpdateEmotionalState(ENarr_EmotionalState::Caution);
                break;
            case ENarr_StoryPhase::TerritoryMapping:
                UpdateEmotionalState(ENarr_EmotionalState::Curiosity);
                break;
            case ENarr_StoryPhase::PredatorAwareness:
                UpdateEmotionalState(ENarr_EmotionalState::Caution);
                break;
            case ENarr_StoryPhase::HerdObservation:
                UpdateEmotionalState(ENarr_EmotionalState::Curiosity);
                break;
            case ENarr_StoryPhase::SurvivalMastery:
                UpdateEmotionalState(ENarr_EmotionalState::Confidence);
                break;
            case ENarr_StoryPhase::TerritorialConflict:
                UpdateEmotionalState(ENarr_EmotionalState::Determination);
                break;
        }

        UE_LOG(LogTemp, Warning, TEXT("Story phase advanced from %d to %d"), (int32)OldPhase, (int32)NewPhase);
    }
}

void ANarr_StoryController::UpdateEmotionalState(ENarr_EmotionalState NewState)
{
    if (PlayerProgress.EmotionalState != NewState)
    {
        ENarr_EmotionalState OldState = PlayerProgress.EmotionalState;
        PlayerProgress.EmotionalState = NewState;

        // Trigger blueprint event
        OnEmotionalStateChange(OldState, NewState);

        UE_LOG(LogTemp, Log, TEXT("Emotional state changed from %d to %d"), (int32)OldState, (int32)NewState);
    }
}

void ANarr_StoryController::CheckLocationBasedEvents(FVector PlayerLocation)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bTriggered && CanTriggerEvent(Event))
        {
            float DistanceToEvent = FVector::Dist(PlayerLocation, Event.TriggerLocation);
            
            if (DistanceToEvent <= Event.TriggerRadius)
            {
                if (CurrentTime - LastEventTime >= Event.EventCooldown)
                {
                    ProcessEventTrigger(Event);
                    LastEventTime = CurrentTime;
                }
            }
        }
    }
}

bool ANarr_StoryController::CanTriggerEvent(const FNarr_StoryEvent& Event) const
{
    // Check if player is in the required story phase or beyond
    return (int32)PlayerProgress.CurrentPhase >= (int32)Event.RequiredPhase;
}

void ANarr_StoryController::ProcessEventTrigger(FNarr_StoryEvent& Event)
{
    Event.bTriggered = true;
    ActiveEvents.Add(Event);

    // Play audio if available
    if (!Event.AudioCueName.IsEmpty())
    {
        PlayNarrativeAudio(Event.AudioCueName);
    }

    // Trigger blueprint event
    OnStoryEventTriggered(Event);

    UE_LOG(LogTemp, Warning, TEXT("Story event triggered: %s"), *Event.EventName);
}

void ANarr_StoryController::TriggerStoryEvent(const FString& EventName)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventName == EventName && !Event.bTriggered)
        {
            ProcessEventTrigger(Event);
            break;
        }
    }
}

void ANarr_StoryController::RegisterDinosaurEncounter(const FString& DinosaurType, bool bSurvived)
{
    PlayerProgress.DinosaurEncounters++;
    
    if (!bSurvived)
    {
        RecordNearDeathExperience();
    }

    // Unlock knowledge about the dinosaur
    FString KnowledgeItem = FString::Printf(TEXT("Encountered %s"), *DinosaurType);
    UnlockKnowledge(KnowledgeItem);

    UE_LOG(LogTemp, Log, TEXT("Dinosaur encounter registered: %s (Survived: %s)"), *DinosaurType, bSurvived ? TEXT("Yes") : TEXT("No"));
}

void ANarr_StoryController::RegisterSuccessfulObservation(const FString& ObservationType)
{
    PlayerProgress.SuccessfulObservations++;
    
    // Unlock knowledge about the observation
    FString KnowledgeItem = FString::Printf(TEXT("Observed %s"), *ObservationType);
    UnlockKnowledge(KnowledgeItem);

    UE_LOG(LogTemp, Log, TEXT("Successful observation registered: %s"), *ObservationType);
}

void ANarr_StoryController::PlayNarrativeAudio(const FString& AudioCueName)
{
    if (AudioCueURLs.Contains(AudioCueName))
    {
        FString AudioURL = AudioCueURLs[AudioCueName];
        UE_LOG(LogTemp, Warning, TEXT("Playing narrative audio: %s -> %s"), *AudioCueName, *AudioURL);
        
        // In a full implementation, this would trigger audio playback
        // For now, we log the audio cue
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio cue not found: %s"), *AudioCueName);
    }
}

void ANarr_StoryController::RegisterAudioCue(const FString& CueName, const FString& AudioURL)
{
    AudioCueURLs.Add(CueName, AudioURL);
    UE_LOG(LogTemp, Log, TEXT("Registered audio cue: %s"), *CueName);
}

void ANarr_StoryController::UnlockKnowledge(const FString& KnowledgeItem)
{
    if (!PlayerProgress.UnlockedKnowledge.Contains(KnowledgeItem))
    {
        PlayerProgress.UnlockedKnowledge.Add(KnowledgeItem);
        UE_LOG(LogTemp, Log, TEXT("Knowledge unlocked: %s"), *KnowledgeItem);
    }
}

bool ANarr_StoryController::HasKnowledge(const FString& KnowledgeItem) const
{
    return PlayerProgress.UnlockedKnowledge.Contains(KnowledgeItem);
}

TArray<FString> ANarr_StoryController::GetAvailableKnowledge() const
{
    return PlayerProgress.UnlockedKnowledge;
}

void ANarr_StoryController::UpdateSurvivalTime(float DeltaTime)
{
    PlayerProgress.SurvivalTime += DeltaTime;
}

void ANarr_StoryController::RecordNearDeathExperience()
{
    PlayerProgress.NearDeathExperiences++;
    UE_LOG(LogTemp, Warning, TEXT("Near death experience recorded. Total: %d"), PlayerProgress.NearDeathExperiences);
}

void ANarr_StoryController::UpdateActiveEvents()
{
    // Remove expired events from active list
    ActiveEvents.RemoveAll([](const FNarr_StoryEvent& Event) {
        return Event.bTriggered;
    });
}

FString ANarr_StoryController::GetCurrentPhaseDescription() const
{
    switch (PlayerProgress.CurrentPhase)
    {
        case ENarr_StoryPhase::Awakening:
            return TEXT("Awakening to a prehistoric world");
        case ENarr_StoryPhase::FirstContact:
            return TEXT("First encounters with ancient creatures");
        case ENarr_StoryPhase::LearningToSurvive:
            return TEXT("Learning basic survival skills");
        case ENarr_StoryPhase::TerritoryMapping:
            return TEXT("Mapping dangerous territories");
        case ENarr_StoryPhase::PredatorAwareness:
            return TEXT("Understanding predator behavior");
        case ENarr_StoryPhase::HerdObservation:
            return TEXT("Studying herbivore social patterns");
        case ENarr_StoryPhase::SurvivalMastery:
            return TEXT("Mastering survival techniques");
        case ENarr_StoryPhase::TerritorialConflict:
            return TEXT("Navigating territorial conflicts");
        default:
            return TEXT("Unknown phase");
    }
}

FString ANarr_StoryController::GetEmotionalStateDescription() const
{
    switch (PlayerProgress.EmotionalState)
    {
        case ENarr_EmotionalState::Terror:
            return TEXT("Overwhelming terror");
        case ENarr_EmotionalState::Anxiety:
            return TEXT("Constant anxiety");
        case ENarr_EmotionalState::Caution:
            return TEXT("Careful caution");
        case ENarr_EmotionalState::Curiosity:
            return TEXT("Scientific curiosity");
        case ENarr_EmotionalState::Confidence:
            return TEXT("Growing confidence");
        case ENarr_EmotionalState::Determination:
            return TEXT("Focused determination");
        case ENarr_EmotionalState::Respect:
            return TEXT("Respectful understanding");
        default:
            return TEXT("Unknown emotional state");
    }
}