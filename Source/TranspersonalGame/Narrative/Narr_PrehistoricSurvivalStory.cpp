#include "Narr_PrehistoricSurvivalStory.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "UObject/ConstructorHelpers.h"

UNarr_PrehistoricSurvivalStory::UNarr_PrehistoricSurvivalStory()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize story progression
    CurrentChapter = ENarr_StoryChapter::Arrival;
    StoryProgress = 0.0f;
    bIsStoryActive = true;
    
    // Initialize survival context
    CurrentSurvivalState = ENarr_SurvivalState::Exploring;
    ThreatLevel = ENarr_ThreatLevel::Low;
    
    // Initialize character states
    PlayerFearLevel = 0.0f;
    PlayerConfidenceLevel = 100.0f;
    PlayerKnowledgeLevel = 0.0f;
    
    // Initialize environmental factors
    TimeOfDay = 12.0f; // Noon
    WeatherCondition = ENarr_WeatherCondition::Clear;
    SeasonalPhase = ENarr_SeasonalPhase::DrySeasonStart;
    
    // Initialize story triggers
    bHasMetFirstDinosaur = false;
    bHasBuiltFirstShelter = false;
    bHasDiscoveredFire = false;
    bHasFormedAlliance = false;
    bHasSurvivedFirstNight = false;
    bHasEncounteredPredator = false;
    bHasLearnedHunting = false;
    bHasExploredCaves = false;
    
    // Initialize biome discovery
    BiomesDiscovered.Empty();
    
    // Initialize character relationships
    NPCRelationships.Empty();
    
    // Initialize story events
    CompletedStoryEvents.Empty();
    ActiveStoryEvents.Empty();
    
    // Initialize narrative voice lines
    CurrentNarrativeVoiceLine = TEXT("");
    bIsNarrativeVoiceActive = false;
    NarrativeVoiceTimer = 0.0f;
    
    // Initialize chapter progression
    ChapterCompletionPercentage.Add(ENarr_StoryChapter::Arrival, 0.0f);
    ChapterCompletionPercentage.Add(ENarr_StoryChapter::FirstContact, 0.0f);
    ChapterCompletionPercentage.Add(ENarr_StoryChapter::Adaptation, 0.0f);
    ChapterCompletionPercentage.Add(ENarr_StoryChapter::Discovery, 0.0f);
    ChapterCompletionPercentage.Add(ENarr_StoryChapter::Mastery, 0.0f);
    ChapterCompletionPercentage.Add(ENarr_StoryChapter::Legacy, 0.0f);
}

void UNarr_PrehistoricSurvivalStory::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize story system
    InitializeStorySystem();
    
    // Start opening narrative
    TriggerOpeningNarrative();
    
    UE_LOG(LogTemp, Log, TEXT("Prehistoric Survival Story system initialized"));
}

void UNarr_PrehistoricSurvivalStory::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsStoryActive)
        return;
        
    // Update story progression
    UpdateStoryProgression(DeltaTime);
    
    // Update narrative voice timing
    UpdateNarrativeVoice(DeltaTime);
    
    // Check for story triggers
    CheckStoryTriggers();
    
    // Update environmental context
    UpdateEnvironmentalContext(DeltaTime);
}

void UNarr_PrehistoricSurvivalStory::InitializeStorySystem()
{
    // Set initial story state
    CurrentChapter = ENarr_StoryChapter::Arrival;
    StoryProgress = 0.0f;
    
    // Initialize player state
    PlayerFearLevel = 25.0f; // Start with some fear
    PlayerConfidenceLevel = 50.0f; // Moderate confidence
    PlayerKnowledgeLevel = 10.0f; // Minimal knowledge
    
    // Set initial environment
    ThreatLevel = ENarr_ThreatLevel::Low;
    CurrentSurvivalState = ENarr_SurvivalState::Exploring;
    
    // Clear previous story data
    CompletedStoryEvents.Empty();
    ActiveStoryEvents.Empty();
    BiomesDiscovered.Empty();
    NPCRelationships.Empty();
    
    // Add initial story event
    FNarr_StoryEvent InitialEvent;
    InitialEvent.EventID = TEXT("story_arrival");
    InitialEvent.EventType = ENarr_StoryEventType::Narrative;
    InitialEvent.EventTitle = TEXT("Arrival in the Cretaceous");
    InitialEvent.EventDescription = TEXT("You have arrived in a dangerous prehistoric world. Survival is your only priority.");
    InitialEvent.TriggerConditions.Add(TEXT("game_start"));
    InitialEvent.bIsActive = true;
    InitialEvent.Priority = 10;
    
    ActiveStoryEvents.Add(InitialEvent);
}

void UNarr_PrehistoricSurvivalStory::TriggerOpeningNarrative()
{
    // Trigger opening narrative sequence
    FString OpeningNarrative = TEXT("You find yourself in an alien world. The air is thick and humid. Strange sounds echo through the dense vegetation. This is not your time. This is the age of giants. Your survival depends on understanding the rules of this prehistoric world.");
    
    TriggerNarrativeVoiceLine(OpeningNarrative, 8.0f);
    
    // Log story start
    UE_LOG(LogTemp, Log, TEXT("Opening narrative triggered: %s"), *OpeningNarrative);
}

void UNarr_PrehistoricSurvivalStory::UpdateStoryProgression(float DeltaTime)
{
    // Update overall story progress
    float ProgressIncrement = DeltaTime * 0.1f; // Slow progression
    StoryProgress = FMath::Clamp(StoryProgress + ProgressIncrement, 0.0f, 100.0f);
    
    // Update chapter completion
    if (ChapterCompletionPercentage.Contains(CurrentChapter))
    {
        float CurrentCompletion = ChapterCompletionPercentage[CurrentChapter];
        CurrentCompletion += DeltaTime * 0.5f; // Chapter progression
        ChapterCompletionPercentage[CurrentChapter] = FMath::Clamp(CurrentCompletion, 0.0f, 100.0f);
        
        // Check for chapter completion
        if (CurrentCompletion >= 100.0f)
        {
            AdvanceToNextChapter();
        }
    }
}

void UNarr_PrehistoricSurvivalStory::UpdateNarrativeVoice(float DeltaTime)
{
    if (bIsNarrativeVoiceActive)
    {
        NarrativeVoiceTimer -= DeltaTime;
        if (NarrativeVoiceTimer <= 0.0f)
        {
            bIsNarrativeVoiceActive = false;
            CurrentNarrativeVoiceLine = TEXT("");
        }
    }
}

void UNarr_PrehistoricSurvivalStory::CheckStoryTriggers()
{
    // Check for story progression triggers
    CheckDinosaurEncounter();
    CheckShelterBuilding();
    CheckFireDiscovery();
    CheckNightSurvival();
    CheckBiomeExploration();
    CheckPredatorEncounter();
}

void UNarr_PrehistoricSurvivalStory::UpdateEnvironmentalContext(float DeltaTime)
{
    // Update time of day (simplified)
    TimeOfDay += DeltaTime * 0.1f; // Slow time progression
    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay = 0.0f;
    }
    
    // Update threat level based on time and environment
    if (TimeOfDay >= 18.0f || TimeOfDay <= 6.0f) // Night time
    {
        ThreatLevel = ENarr_ThreatLevel::High;
    }
    else if (TimeOfDay >= 16.0f || TimeOfDay <= 8.0f) // Dawn/Dusk
    {
        ThreatLevel = ENarr_ThreatLevel::Medium;
    }
    else
    {
        ThreatLevel = ENarr_ThreatLevel::Low;
    }
}

void UNarr_PrehistoricSurvivalStory::CheckDinosaurEncounter()
{
    if (!bHasMetFirstDinosaur)
    {
        // This would be triggered by actual dinosaur proximity in real implementation
        // For now, simulate based on story progress
        if (StoryProgress >= 5.0f)
        {
            TriggerFirstDinosaurEncounter();
        }
    }
}

void UNarr_PrehistoricSurvivalStory::CheckShelterBuilding()
{
    if (!bHasBuiltFirstShelter)
    {
        // This would be triggered by actual shelter construction
        if (StoryProgress >= 15.0f)
        {
            TriggerShelterBuildingEvent();
        }
    }
}

void UNarr_PrehistoricSurvivalStory::CheckFireDiscovery()
{
    if (!bHasDiscoveredFire)
    {
        // This would be triggered by actual fire creation
        if (StoryProgress >= 25.0f)
        {
            TriggerFireDiscoveryEvent();
        }
    }
}

void UNarr_PrehistoricSurvivalStory::CheckNightSurvival()
{
    if (!bHasSurvivedFirstNight)
    {
        // Check if player has survived a full night cycle
        if (TimeOfDay >= 6.0f && TimeOfDay <= 8.0f && StoryProgress >= 10.0f)
        {
            TriggerNightSurvivalEvent();
        }
    }
}

void UNarr_PrehistoricSurvivalStory::CheckBiomeExploration()
{
    // This would check actual player location vs biome boundaries
    // For now, simulate based on story progress
    if (BiomesDiscovered.Num() == 0 && StoryProgress >= 20.0f)
    {
        BiomesDiscovered.Add(TEXT("Forest"));
        TriggerBiomeDiscoveryEvent(TEXT("Forest"));
    }
}

void UNarr_PrehistoricSurvivalStory::CheckPredatorEncounter()
{
    if (!bHasEncounteredPredator)
    {
        // This would be triggered by actual predator proximity
        if (StoryProgress >= 30.0f && ThreatLevel >= ENarr_ThreatLevel::Medium)
        {
            TriggerPredatorEncounterEvent();
        }
    }
}

void UNarr_PrehistoricSurvivalStory::TriggerFirstDinosaurEncounter()
{
    bHasMetFirstDinosaur = true;
    
    FString NarrativeText = TEXT("Your first encounter with the giants of this world. A massive herbivore grazes nearby, oblivious to your presence. You realize the scale of the creatures that rule this land. Respect and caution will be your allies.");
    
    TriggerNarrativeVoiceLine(NarrativeText, 10.0f);
    
    // Update player psychology
    PlayerFearLevel += 15.0f;
    PlayerKnowledgeLevel += 10.0f;
    
    // Add story event
    FNarr_StoryEvent Event;
    Event.EventID = TEXT("first_dinosaur_encounter");
    Event.EventType = ENarr_StoryEventType::Discovery;
    Event.EventTitle = TEXT("First Contact");
    Event.EventDescription = TEXT("Encountered your first dinosaur");
    CompletedStoryEvents.Add(Event);
    
    UE_LOG(LogTemp, Log, TEXT("First dinosaur encounter triggered"));
}

void UNarr_PrehistoricSurvivalStory::TriggerShelterBuildingEvent()
{
    bHasBuiltFirstShelter = true;
    
    FString NarrativeText = TEXT("Your first shelter takes shape. Crude but functional, it represents your first step toward mastering this environment. Protection from the elements and predators is now within reach.");
    
    TriggerNarrativeVoiceLine(NarrativeText, 8.0f);
    
    // Update player psychology
    PlayerConfidenceLevel += 20.0f;
    PlayerKnowledgeLevel += 15.0f;
    PlayerFearLevel -= 10.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Shelter building event triggered"));
}

void UNarr_PrehistoricSurvivalStory::TriggerFireDiscoveryEvent()
{
    bHasDiscoveredFire = true;
    
    FString NarrativeText = TEXT("Fire. The great equalizer. With flame comes warmth, protection, and the ability to cook food. You have taken a crucial step toward not just surviving, but thriving in this prehistoric world.");
    
    TriggerNarrativeVoiceLine(NarrativeText, 9.0f);
    
    // Update player psychology
    PlayerConfidenceLevel += 25.0f;
    PlayerKnowledgeLevel += 20.0f;
    PlayerFearLevel -= 15.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Fire discovery event triggered"));
}

void UNarr_PrehistoricSurvivalStory::TriggerNightSurvivalEvent()
{
    bHasSurvivedFirstNight = true;
    
    FString NarrativeText = TEXT("You have survived your first night in the Cretaceous. The darkness brought new sounds, new fears, but you endured. Each night survived is a victory against the odds.");
    
    TriggerNarrativeVoiceLine(NarrativeText, 8.0f);
    
    // Update player psychology
    PlayerConfidenceLevel += 15.0f;
    PlayerKnowledgeLevel += 10.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Night survival event triggered"));
}

void UNarr_PrehistoricSurvivalStory::TriggerBiomeDiscoveryEvent(const FString& BiomeName)
{
    FString NarrativeText = FString::Printf(TEXT("You have discovered a new biome: %s. Each environment brings new challenges, new resources, and new opportunities. Adaptation is the key to survival."), *BiomeName);
    
    TriggerNarrativeVoiceLine(NarrativeText, 7.0f);
    
    // Update player knowledge
    PlayerKnowledgeLevel += 12.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Biome discovery event triggered: %s"), *BiomeName);
}

void UNarr_PrehistoricSurvivalStory::TriggerPredatorEncounterEvent()
{
    bHasEncounteredPredator = true;
    
    FString NarrativeText = TEXT("A predator has noticed you. Your heart pounds as ancient instincts awaken. This is the moment that separates survivors from victims. Stay calm, stay smart, stay alive.");
    
    TriggerNarrativeVoiceLine(NarrativeText, 9.0f);
    
    // Update player psychology
    PlayerFearLevel += 25.0f;
    PlayerKnowledgeLevel += 15.0f;
    
    // Change survival state
    CurrentSurvivalState = ENarr_SurvivalState::Fleeing;
    
    UE_LOG(LogTemp, Log, TEXT("Predator encounter event triggered"));
}

void UNarr_PrehistoricSurvivalStory::AdvanceToNextChapter()
{
    ENarr_StoryChapter NextChapter = CurrentChapter;
    
    switch (CurrentChapter)
    {
        case ENarr_StoryChapter::Arrival:
            NextChapter = ENarr_StoryChapter::FirstContact;
            break;
        case ENarr_StoryChapter::FirstContact:
            NextChapter = ENarr_StoryChapter::Adaptation;
            break;
        case ENarr_StoryChapter::Adaptation:
            NextChapter = ENarr_StoryChapter::Discovery;
            break;
        case ENarr_StoryChapter::Discovery:
            NextChapter = ENarr_StoryChapter::Mastery;
            break;
        case ENarr_StoryChapter::Mastery:
            NextChapter = ENarr_StoryChapter::Legacy;
            break;
        case ENarr_StoryChapter::Legacy:
            // Story complete
            break;
    }
    
    if (NextChapter != CurrentChapter)
    {
        CurrentChapter = NextChapter;
        TriggerChapterTransition(NextChapter);
    }
}

void UNarr_PrehistoricSurvivalStory::TriggerChapterTransition(ENarr_StoryChapter NewChapter)
{
    FString ChapterName = GetChapterName(NewChapter);
    FString NarrativeText = FString::Printf(TEXT("Chapter: %s. Your journey continues to evolve. New challenges await, but you are no longer the same person who first arrived in this world."), *ChapterName);
    
    TriggerNarrativeVoiceLine(NarrativeText, 10.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Chapter transition to: %s"), *ChapterName);
}

void UNarr_PrehistoricSurvivalStory::TriggerNarrativeVoiceLine(const FString& NarrativeText, float Duration)
{
    CurrentNarrativeVoiceLine = NarrativeText;
    bIsNarrativeVoiceActive = true;
    NarrativeVoiceTimer = Duration;
    
    // Broadcast narrative event
    OnNarrativeTriggered.Broadcast(NarrativeText, Duration);
    
    UE_LOG(LogTemp, Log, TEXT("Narrative triggered: %s"), *NarrativeText);
}

FString UNarr_PrehistoricSurvivalStory::GetChapterName(ENarr_StoryChapter Chapter) const
{
    switch (Chapter)
    {
        case ENarr_StoryChapter::Arrival: return TEXT("Arrival");
        case ENarr_StoryChapter::FirstContact: return TEXT("First Contact");
        case ENarr_StoryChapter::Adaptation: return TEXT("Adaptation");
        case ENarr_StoryChapter::Discovery: return TEXT("Discovery");
        case ENarr_StoryChapter::Mastery: return TEXT("Mastery");
        case ENarr_StoryChapter::Legacy: return TEXT("Legacy");
        default: return TEXT("Unknown");
    }
}

void UNarr_PrehistoricSurvivalStory::UpdatePlayerPsychology(float FearDelta, float ConfidenceDelta, float KnowledgeDelta)
{
    PlayerFearLevel = FMath::Clamp(PlayerFearLevel + FearDelta, 0.0f, 100.0f);
    PlayerConfidenceLevel = FMath::Clamp(PlayerConfidenceLevel + ConfidenceDelta, 0.0f, 100.0f);
    PlayerKnowledgeLevel = FMath::Clamp(PlayerKnowledgeLevel + KnowledgeDelta, 0.0f, 100.0f);
}

void UNarr_PrehistoricSurvivalStory::SetSurvivalState(ENarr_SurvivalState NewState)
{
    if (CurrentSurvivalState != NewState)
    {
        CurrentSurvivalState = NewState;
        OnSurvivalStateChanged.Broadcast(NewState);
    }
}

void UNarr_PrehistoricSurvivalStory::SetThreatLevel(ENarr_ThreatLevel NewThreatLevel)
{
    if (ThreatLevel != NewThreatLevel)
    {
        ThreatLevel = NewThreatLevel;
        OnThreatLevelChanged.Broadcast(NewThreatLevel);
    }
}

bool UNarr_PrehistoricSurvivalStory::HasCompletedStoryEvent(const FString& EventID) const
{
    for (const FNarr_StoryEvent& Event : CompletedStoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return true;
        }
    }
    return false;
}

float UNarr_PrehistoricSurvivalStory::GetChapterProgress(ENarr_StoryChapter Chapter) const
{
    if (ChapterCompletionPercentage.Contains(Chapter))
    {
        return ChapterCompletionPercentage[Chapter];
    }
    return 0.0f;
}

TArray<FString> UNarr_PrehistoricSurvivalStory::GetDiscoveredBiomes() const
{
    return BiomesDiscovered;
}

FString UNarr_PrehistoricSurvivalStory::GetCurrentNarrativeVoiceLine() const
{
    return bIsNarrativeVoiceActive ? CurrentNarrativeVoiceLine : TEXT("");
}