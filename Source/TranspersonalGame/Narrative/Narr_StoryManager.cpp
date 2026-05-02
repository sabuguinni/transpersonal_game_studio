#include "Narr_StoryManager.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ANarr_StoryManager::ANarr_StoryManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;
    
    // Initialize narrative context
    NarrativeContext = FNarr_NarrativeContext();
    
    // Initialize timer
    StoryUpdateTimer = 0.0f;
}

void ANarr_StoryManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize story events
    InitializeStoryEvents();
    
    // Find narrative triggers in the level
    TArray<AActor*> FoundTriggers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATriggerBox::StaticClass(), FoundTriggers);
    
    for (AActor* Actor : FoundTriggers)
    {
        ATriggerBox* TriggerBox = Cast<ATriggerBox>(Actor);
        if (TriggerBox && TriggerBox->GetName().Contains(TEXT("Narrative")))
        {
            NarrativeTriggers.Add(TriggerBox);
            UE_LOG(LogTemp, Log, TEXT("Found narrative trigger: %s"), *TriggerBox->GetName());
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("StoryManager initialized with %d triggers"), NarrativeTriggers.Num());
}

void ANarr_StoryManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    StoryUpdateTimer += DeltaTime;
    
    if (StoryUpdateTimer >= StoryUpdateInterval)
    {
        CheckTriggerConditions();
        ProcessPendingEvents();
        EvaluateStoryProgression();
        
        StoryUpdateTimer = 0.0f;
    }
}

void ANarr_StoryManager::InitializeStoryEvents()
{
    StoryEvents.Empty();
    
    // Introduction events
    FNarr_StoryEvent IntroEvent;
    IntroEvent.EventID = TEXT("INTRO_AWAKENING");
    IntroEvent.DialogueText = TEXT("You awaken in an unfamiliar world. The air is thick and humid. Strange sounds echo from the dense vegetation around you.");
    IntroEvent.TriggerType = ENarr_TriggerType::Discovery;
    IntroEvent.Priority = 10.0f;
    IntroEvent.bHasBeenTriggered = false;
    StoryEvents.Add(IntroEvent);
    
    // First dinosaur encounter
    FNarr_StoryEvent FirstDinoEvent;
    FirstDinoEvent.EventID = TEXT("FIRST_DINOSAUR");
    FirstDinoEvent.DialogueText = TEXT("Movement in the undergrowth. Something massive. Your heart pounds as you realize you're not alone in this ancient world.");
    FirstDinoEvent.TriggerType = ENarr_TriggerType::Discovery;
    FirstDinoEvent.Priority = 9.0f;
    FirstDinoEvent.bHasBeenTriggered = false;
    StoryEvents.Add(FirstDinoEvent);
    
    // Danger zone warning
    FNarr_StoryEvent DangerEvent;
    DangerEvent.EventID = TEXT("DANGER_ZONE");
    DangerEvent.DialogueText = TEXT("The scent of predators hangs heavy in the air. Every instinct screams danger. This is not a place for the unwary.");
    DangerEvent.TriggerType = ENarr_TriggerType::Danger;
    DangerEvent.Priority = 8.0f;
    DangerEvent.bHasBeenTriggered = false;
    StoryEvents.Add(DangerEvent);
    
    // Safe zone discovery
    FNarr_StoryEvent SafeEvent;
    SafeEvent.EventID = TEXT("SAFE_HAVEN");
    SafeEvent.DialogueText = TEXT("Higher ground offers respite. From here, you can observe the valley below and plan your next move carefully.");
    SafeEvent.TriggerType = ENarr_TriggerType::Safety;
    SafeEvent.Priority = 7.0f;
    SafeEvent.bHasBeenTriggered = false;
    StoryEvents.Add(SafeEvent);
    
    // Survival milestone
    FNarr_StoryEvent SurvivalEvent;
    SurvivalEvent.EventID = TEXT("FIRST_DAY");
    SurvivalEvent.DialogueText = TEXT("One day survived. Each moment in this primordial world is a victory against impossible odds.");
    SurvivalEvent.TriggerType = ENarr_TriggerType::Achievement;
    SurvivalEvent.Priority = 6.0f;
    SurvivalEvent.bHasBeenTriggered = false;
    StoryEvents.Add(SurvivalEvent);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d story events"), StoryEvents.Num());
}

void ANarr_StoryManager::TriggerStoryEvent(const FString& EventID, ENarr_TriggerType TriggerType)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bHasBeenTriggered)
        {
            Event.bHasBeenTriggered = true;
            NarrativeContext.CompletedEvents.Add(EventID);
            
            // Play narrative audio if available
            PlayNarrativeAudio(EventID);
            
            // Log the event
            UE_LOG(LogTemp, Log, TEXT("Story Event Triggered: %s - %s"), *EventID, *Event.DialogueText);
            
            // Display on screen for debugging
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                    FString::Printf(TEXT("NARRATIVE: %s"), *Event.DialogueText));
            }
            
            break;
        }
    }
}

void ANarr_StoryManager::AdvanceStoryState(ENarr_StoryState NewState)
{
    if (NewState != NarrativeContext.CurrentState)
    {
        ENarr_StoryState PreviousState = NarrativeContext.CurrentState;
        NarrativeContext.CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Story state advanced from %d to %d"), 
            (int32)PreviousState, (int32)NewState);
        
        // Trigger state-specific events
        switch (NewState)
        {
            case ENarr_StoryState::FirstContact:
                TriggerStoryEvent(TEXT("FIRST_DINOSAUR"), ENarr_TriggerType::Discovery);
                break;
            case ENarr_StoryState::Survival:
                TriggerStoryEvent(TEXT("FIRST_DAY"), ENarr_TriggerType::Achievement);
                break;
            default:
                break;
        }
    }
}

void ANarr_StoryManager::RegisterDinosaurEncounter(const FString& DinosaurType)
{
    NarrativeContext.DinosaurEncounters++;
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur encounter registered: %s (Total: %d)"), 
        *DinosaurType, NarrativeContext.DinosaurEncounters);
    
    // Trigger first contact if this is the first encounter
    if (NarrativeContext.DinosaurEncounters == 1 && 
        NarrativeContext.CurrentState == ENarr_StoryState::Exploration)
    {
        AdvanceStoryState(ENarr_StoryState::FirstContact);
    }
}

void ANarr_StoryManager::UpdateSurvivalMetrics(float HealthPercent, float HungerPercent, float ThirstPercent)
{
    // Calculate survival score based on player condition
    float ConditionScore = (HealthPercent + (100.0f - HungerPercent) + (100.0f - ThirstPercent)) / 3.0f;
    NarrativeContext.SurvivalScore = FMath::Max(NarrativeContext.SurvivalScore, ConditionScore);
    
    // Check for critical conditions
    if (HealthPercent < 25.0f || HungerPercent > 80.0f || ThirstPercent > 80.0f)
    {
        if (!IsEventCompleted(TEXT("CRITICAL_CONDITION")))
        {
            TriggerStoryEvent(TEXT("CRITICAL_CONDITION"), ENarr_TriggerType::Danger);
        }
    }
}

bool ANarr_StoryManager::IsEventCompleted(const FString& EventID) const
{
    return NarrativeContext.CompletedEvents.Contains(EventID);
}

FNarr_StoryEvent ANarr_StoryManager::GetNextPendingEvent() const
{
    FNarr_StoryEvent BestEvent;
    float HighestPriority = -1.0f;
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bHasBeenTriggered && Event.Priority > HighestPriority)
        {
            BestEvent = Event;
            HighestPriority = Event.Priority;
        }
    }
    
    return BestEvent;
}

void ANarr_StoryManager::PlayNarrativeAudio(const FString& EventID)
{
    // Find the event and play its audio
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && Event.VoiceClip.IsValid())
        {
            if (AudioComponent && Event.VoiceClip.LoadSynchronous())
            {
                AudioComponent->SetSound(Event.VoiceClip.Get());
                AudioComponent->Play();
                UE_LOG(LogTemp, Log, TEXT("Playing narrative audio for: %s"), *EventID);
            }
            break;
        }
    }
}

void ANarr_StoryManager::CheckTriggerConditions()
{
    // Check if player is in any narrative trigger zones
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;
    
    for (ATriggerBox* Trigger : NarrativeTriggers)
    {
        if (!Trigger) continue;
        
        // Check if player is overlapping with trigger
        TArray<AActor*> OverlappingActors;
        Trigger->GetOverlappingActors(OverlappingActors, APawn::StaticClass());
        
        for (AActor* Actor : OverlappingActors)
        {
            if (Actor == PlayerPawn)
            {
                FString TriggerName = Trigger->GetName();
                
                // Trigger appropriate events based on trigger name
                if (TriggerName.Contains(TEXT("Discovery")) && !IsEventCompleted(TEXT("FIRST_DINOSAUR")))
                {
                    TriggerStoryEvent(TEXT("FIRST_DINOSAUR"), ENarr_TriggerType::Discovery);
                }
                else if (TriggerName.Contains(TEXT("Danger")) && !IsEventCompleted(TEXT("DANGER_ZONE")))
                {
                    TriggerStoryEvent(TEXT("DANGER_ZONE"), ENarr_TriggerType::Danger);
                }
                else if (TriggerName.Contains(TEXT("Safe")) && !IsEventCompleted(TEXT("SAFE_HAVEN")))
                {
                    TriggerStoryEvent(TEXT("SAFE_HAVEN"), ENarr_TriggerType::Safety);
                }
                break;
            }
        }
    }
}

void ANarr_StoryManager::ProcessPendingEvents()
{
    // Process any time-based or condition-based events
    if (NarrativeContext.CurrentState == ENarr_StoryState::Introduction && 
        !IsEventCompleted(TEXT("INTRO_AWAKENING")))
    {
        TriggerStoryEvent(TEXT("INTRO_AWAKENING"), ENarr_TriggerType::Discovery);
    }
}

void ANarr_StoryManager::EvaluateStoryProgression()
{
    // Check if we should advance to the next story state
    if (ShouldAdvanceToNextState())
    {
        switch (NarrativeContext.CurrentState)
        {
            case ENarr_StoryState::Introduction:
                if (IsEventCompleted(TEXT("INTRO_AWAKENING")))
                {
                    AdvanceStoryState(ENarr_StoryState::Exploration);
                }
                break;
            case ENarr_StoryState::Exploration:
                if (NarrativeContext.DinosaurEncounters > 0)
                {
                    AdvanceStoryState(ENarr_StoryState::FirstContact);
                }
                break;
            case ENarr_StoryState::FirstContact:
                if (NarrativeContext.SurvivalScore > 50.0f)
                {
                    AdvanceStoryState(ENarr_StoryState::Survival);
                }
                break;
            default:
                break;
        }
    }
}

bool ANarr_StoryManager::ShouldAdvanceToNextState() const
{
    // Simple progression logic based on completed events and metrics
    switch (NarrativeContext.CurrentState)
    {
        case ENarr_StoryState::Introduction:
            return IsEventCompleted(TEXT("INTRO_AWAKENING"));
        case ENarr_StoryState::Exploration:
            return NarrativeContext.DinosaurEncounters > 0;
        case ENarr_StoryState::FirstContact:
            return NarrativeContext.SurvivalScore > 50.0f;
        default:
            return false;
    }
}