#include "NarrativeBible.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

DEFINE_LOG_CATEGORY(LogNarrativeBible);

UNarrativeBible::UNarrativeBible()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize core narrative elements
    InitializeCoreNarrative();
    InitializeCharacterArcs();
    InitializeWorldLore();
}

void UNarrativeBible::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogNarrativeBible, Log, TEXT("Narrative Bible initialized - Core story elements loaded"));
    
    // Register with narrative manager
    if (UWorld* World = GetWorld())
    {
        if (ANarrativeManager* NarrativeManager = Cast<ANarrativeManager>(
            UGameplayStatics::GetActorOfClass(World, ANarrativeManager::StaticClass())))
        {
            NarrativeManager->RegisterNarrativeBible(this);
        }
    }
    
    // Initialize emotional baseline
    CurrentEmotionalState = ENarrativeEmotion::Wonder;
    EmotionalIntensity = 0.5f;
}

void UNarrativeBible::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update narrative context based on world state
    UpdateNarrativeContext(DeltaTime);
    
    // Process emotional state transitions
    ProcessEmotionalTransitions(DeltaTime);
    
    // Check for narrative triggers
    CheckNarrativeTriggers();
}

void UNarrativeBible::InitializeCoreNarrative()
{
    // Core premise: Paleontologist transported to Cretaceous period
    CoreNarrative.Protagonist = FNarrativeCharacter{
        TEXT("Dr. Sarah Chen"),
        TEXT("Paleontologist"),
        TEXT("A brilliant scientist specializing in Cretaceous period ecosystems"),
        ENarrativeRole::Protagonist
    };
    
    CoreNarrative.CentralConflict = TEXT("Survival in prehistoric world while searching for return gem");
    CoreNarrative.ThematicCore = TEXT("Adaptation, scientific curiosity vs primal fear, temporal displacement");
    
    // Major story beats
    CoreNarrative.ActStructure.Add(FNarrativeAct{
        TEXT("Arrival"),
        TEXT("Discovery of temporal gem and transportation to Cretaceous period"),
        0.0f, 0.15f
    });
    
    CoreNarrative.ActStructure.Add(FNarrativeAct{
        TEXT("Survival"),
        TEXT("Learning to survive, building shelter, avoiding predators"),
        0.15f, 0.4f
    });
    
    CoreNarrative.ActStructure.Add(FNarrativeAct{
        TEXT("Understanding"),
        TEXT("Observing dinosaur behavior, forming connections, finding purpose"),
        0.4f, 0.7f
    });
    
    CoreNarrative.ActStructure.Add(FNarrativeAct{
        TEXT("Resolution"),
        TEXT("Finding the twin gem and choosing whether to return home"),
        0.7f, 1.0f
    });
}

void UNarrativeBible::InitializeCharacterArcs()
{
    // Protagonist arc: From helpless scientist to prehistoric survivor
    FCharacterArc ProtagonistArc;
    ProtagonistArc.CharacterName = TEXT("Dr. Sarah Chen");
    ProtagonistArc.StartingState = TEXT("Confident academic, dependent on modern technology");
    ProtagonistArc.EndingState = TEXT("Resourceful survivor, connected to natural world");
    ProtagonistArc.CentralConflict = TEXT("Scientific rationality vs primal instincts");
    
    // Arc progression points
    ProtagonistArc.ProgressionPoints.Add(FNarrativeProgressionPoint{
        0.0f, TEXT("Arrival"), TEXT("Shock and disorientation")
    });
    ProtagonistArc.ProgressionPoints.Add(FNarrativeProgressionPoint{
        0.2f, TEXT("First Night"), TEXT("Terror and helplessness")
    });
    ProtagonistArc.ProgressionPoints.Add(FNarrativeProgressionPoint{
        0.4f, TEXT("First Shelter"), TEXT("Growing confidence and resourcefulness")
    });
    ProtagonistArc.ProgressionPoints.Add(FNarrativeProgressionPoint{
        0.6f, TEXT("Dinosaur Bond"), TEXT("Connection with prehistoric life")
    });
    ProtagonistArc.ProgressionPoints.Add(FNarrativeProgressionPoint{
        0.8f, TEXT("Mastery"), TEXT("Thriving in prehistoric environment")
    });
    ProtagonistArc.ProgressionPoints.Add(FNarrativeProgressionPoint{
        1.0f, TEXT("Choice"), TEXT("Decision between old life and new understanding")
    });
    
    CharacterArcs.Add(ProtagonistArc.CharacterName, ProtagonistArc);
}

void UNarrativeBible::InitializeWorldLore()
{
    // Temporal mechanics lore
    WorldLore.Add(TEXT("TemporalGems"), FLoreEntry{
        TEXT("Temporal Displacement Crystals"),
        TEXT("Mysterious crystalline formations that exist outside normal spacetime"),
        TEXT("These gems appear to be naturally occurring temporal anomalies, possibly formed during cosmic events that created rifts in the fabric of time itself.")
    });
    
    // Prehistoric world lore
    WorldLore.Add(TEXT("CretaceousEcosystem"), FLoreEntry{
        TEXT("The Living Cretaceous"),
        TEXT("A fully functional prehistoric ecosystem in perfect balance"),
        TEXT("This world represents Earth as it existed 100 million years ago, with complex predator-prey relationships, seasonal migrations, and intricate social structures among dinosaur species.")
    });
    
    // Consciousness lore (transpersonal elements)
    WorldLore.Add(TEXT("PrimalConsciousness"), FLoreEntry{
        TEXT("The Ancient Awareness"),
        TEXT("A form of consciousness that predates human thought"),
        TEXT("Some dinosaur species, particularly the larger herbivores, seem to possess a form of collective awareness that transcends individual minds - a primal consciousness that connects all life in this ancient world.")
    });
}

void UNarrativeBible::UpdateNarrativeContext(float DeltaTime)
{
    // Update time of day context
    if (UWorld* World = GetWorld())
    {
        float TimeOfDay = World->GetTimeSeconds() / 3600.0f; // Convert to hours
        TimeOfDay = FMath::Fmod(TimeOfDay, 24.0f);
        
        if (TimeOfDay >= 6.0f && TimeOfDay < 18.0f)
        {
            CurrentTimeContext = ENarrativeTimeContext::Day;
        }
        else if (TimeOfDay >= 18.0f && TimeOfDay < 22.0f)
        {
            CurrentTimeContext = ENarrativeTimeContext::Dusk;
        }
        else
        {
            CurrentTimeContext = ENarrativeTimeContext::Night;
        }
    }
    
    // Update survival context based on player state
    UpdateSurvivalContext();
    
    // Update discovery context
    UpdateDiscoveryContext();
}

void UNarrativeBible::ProcessEmotionalTransitions(float DeltaTime)
{
    // Emotional states naturally decay toward neutral over time
    float DecayRate = 0.1f * DeltaTime;
    
    if (EmotionalIntensity > 0.5f)
    {
        EmotionalIntensity = FMath::Max(0.5f, EmotionalIntensity - DecayRate);
    }
    else if (EmotionalIntensity < 0.5f)
    {
        EmotionalIntensity = FMath::Min(0.5f, EmotionalIntensity + DecayRate);
    }
    
    // Check for emotional state changes based on context
    ENarrativeEmotion NewEmotion = DetermineEmotionalState();
    
    if (NewEmotion != CurrentEmotionalState)
    {
        TriggerEmotionalTransition(CurrentEmotionalState, NewEmotion);
        CurrentEmotionalState = NewEmotion;
    }
}

ENarrativeEmotion UNarrativeBible::DetermineEmotionalState() const
{
    // Determine emotional state based on current context
    switch (CurrentTimeContext)
    {
        case ENarrativeTimeContext::Night:
            return ENarrativeEmotion::Fear;
        case ENarrativeTimeContext::Dusk:
            return ENarrativeEmotion::Melancholy;
        case ENarrativeTimeContext::Day:
        default:
            return ENarrativeEmotion::Wonder;
    }
}

void UNarrativeBible::TriggerEmotionalTransition(ENarrativeEmotion FromEmotion, ENarrativeEmotion ToEmotion)
{
    UE_LOG(LogNarrativeBible, Log, TEXT("Emotional transition: %s -> %s"), 
        *UEnum::GetValueAsString(FromEmotion), 
        *UEnum::GetValueAsString(ToEmotion));
    
    // Broadcast emotional transition event
    OnEmotionalStateChanged.Broadcast(FromEmotion, ToEmotion, EmotionalIntensity);
    
    // Trigger appropriate narrative response
    switch (ToEmotion)
    {
        case ENarrativeEmotion::Fear:
            TriggerFearResponse();
            break;
        case ENarrativeEmotion::Wonder:
            TriggerWonderResponse();
            break;
        case ENarrativeEmotion::Melancholy:
            TriggerMelancholyResponse();
            break;
        case ENarrativeEmotion::Relief:
            TriggerReliefResponse();
            break;
    }
}

void UNarrativeBible::CheckNarrativeTriggers()
{
    // Check for major narrative milestone triggers
    for (const auto& Trigger : NarrativeTriggers)
    {
        if (!Trigger.bTriggered && EvaluateTriggerCondition(Trigger))
        {
            ExecuteNarrativeTrigger(Trigger);
        }
    }
}

bool UNarrativeBible::EvaluateTriggerCondition(const FNarrativeTrigger& Trigger) const
{
    // Evaluate trigger conditions based on type
    switch (Trigger.TriggerType)
    {
        case ENarrativeTriggerType::TimeBasedEvent:
            return GetWorld()->GetTimeSeconds() >= Trigger.TimeThreshold;
        case ENarrativeTriggerType::LocationDiscovery:
            return CheckLocationDiscovered(Trigger.LocationName);
        case ENarrativeTriggerType::EmotionalThreshold:
            return EmotionalIntensity >= Trigger.EmotionalThreshold;
        case ENarrativeTriggerType::CharacterInteraction:
            return CheckCharacterInteraction(Trigger.CharacterName);
        default:
            return false;
    }
}

void UNarrativeBible::ExecuteNarrativeTrigger(const FNarrativeTrigger& Trigger)
{
    UE_LOG(LogNarrativeBible, Log, TEXT("Executing narrative trigger: %s"), *Trigger.TriggerName);
    
    // Mark trigger as executed
    const_cast<FNarrativeTrigger&>(Trigger).bTriggered = true;
    
    // Execute trigger effects
    OnNarrativeTriggerExecuted.Broadcast(Trigger.TriggerName, Trigger.NarrativeText);
    
    // Update story progression
    UpdateStoryProgression(Trigger.ProgressionValue);
}

void UNarrativeBible::UpdateStoryProgression(float ProgressionValue)
{
    StoryProgression = FMath::Clamp(StoryProgression + ProgressionValue, 0.0f, 1.0f);
    
    UE_LOG(LogNarrativeBible, Log, TEXT("Story progression updated: %f"), StoryProgression);
    
    // Check if we've entered a new narrative act
    for (const auto& Act : CoreNarrative.ActStructure)
    {
        if (StoryProgression >= Act.StartProgress && StoryProgression <= Act.EndProgress)
        {
            if (CurrentAct != Act.ActName)
            {
                CurrentAct = Act.ActName;
                OnActChanged.Broadcast(CurrentAct, Act.Description);
                UE_LOG(LogNarrativeBible, Log, TEXT("Entered new narrative act: %s"), *CurrentAct);
            }
            break;
        }
    }
}

FString UNarrativeBible::GetContextualNarrative(const FString& Context) const
{
    // Return contextual narrative based on current state
    FString ContextKey = FString::Printf(TEXT("%s_%s_%s"), 
        *UEnum::GetValueAsString(CurrentEmotionalState),
        *UEnum::GetValueAsString(CurrentTimeContext),
        *Context);
    
    if (const FString* ContextualText = ContextualNarratives.Find(ContextKey))
    {
        return *ContextualText;
    }
    
    // Return default narrative if specific context not found
    return GetDefaultNarrativeForContext(Context);
}

FString UNarrativeBible::GetDefaultNarrativeForContext(const FString& Context) const
{
    // Default narratives for common contexts
    if (Context == TEXT("Discovery"))
    {
        return TEXT("The prehistoric world reveals another of its ancient secrets.");
    }
    else if (Context == TEXT("Danger"))
    {
        return TEXT("Survival instincts sharpen as primal threats emerge from the wilderness.");
    }
    else if (Context == TEXT("Wonder"))
    {
        return TEXT("The majesty of the ancient world fills the mind with awe and possibility.");
    }
    
    return TEXT("The journey through time continues, each moment a step deeper into the unknown.");
}

void UNarrativeBible::TriggerFearResponse()
{
    UE_LOG(LogNarrativeBible, Log, TEXT("Triggering fear response"));
    // Implementation for fear-based narrative responses
}

void UNarrativeBible::TriggerWonderResponse()
{
    UE_LOG(LogNarrativeBible, Log, TEXT("Triggering wonder response"));
    // Implementation for wonder-based narrative responses
}

void UNarrativeBible::TriggerMelancholyResponse()
{
    UE_LOG(LogNarrativeBible, Log, TEXT("Triggering melancholy response"));
    // Implementation for melancholy-based narrative responses
}

void UNarrativeBible::TriggerReliefResponse()
{
    UE_LOG(LogNarrativeBible, Log, TEXT("Triggering relief response"));
    // Implementation for relief-based narrative responses
}

void UNarrativeBible::UpdateSurvivalContext()
{
    // Update survival context based on player needs and environment
    // This would integrate with survival systems from other modules
}

void UNarrativeBible::UpdateDiscoveryContext()
{
    // Update discovery context based on exploration progress
    // This would integrate with world generation and quest systems
}

bool UNarrativeBible::CheckLocationDiscovered(const FString& LocationName) const
{
    // Check if specific location has been discovered
    // This would integrate with exploration tracking systems
    return false;
}

bool UNarrativeBible::CheckCharacterInteraction(const FString& CharacterName) const
{
    // Check if player has interacted with specific character
    // This would integrate with dialogue and character systems
    return false;
}