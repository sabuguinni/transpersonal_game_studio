#include "NPC_DinosaurPersonalitySystem.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UNPC_DinosaurPersonalitySystem::UNPC_DinosaurPersonalitySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize personality traits with random values
    Aggressiveness = FMath::RandRange(0.1f, 1.0f);
    Curiosity = FMath::RandRange(0.1f, 1.0f);
    Fearfulness = FMath::RandRange(0.1f, 1.0f);
    Sociability = FMath::RandRange(0.1f, 1.0f);
    Territoriality = FMath::RandRange(0.1f, 1.0f);
    
    // Set default personality type
    PersonalityType = ENPC_PersonalityType::Balanced;
    
    // Initialize mood
    CurrentMood = ENPC_MoodState::Neutral;
    MoodStability = 0.7f;
    StressLevel = 0.0f;
    
    // Initialize experience tracking
    PlayerInteractionCount = 0;
    LastPlayerInteractionTime = 0.0f;
    ThreatMemoryDuration = 300.0f; // 5 minutes
}

void UNPC_DinosaurPersonalitySystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Determine personality type based on traits
    DeterminePersonalityType();
    
    // Initialize mood based on personality
    InitializeMoodFromPersonality();
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurPersonalitySystem initialized for %s with personality %s"), 
           *GetOwner()->GetName(), 
           *UEnum::GetValueAsString(PersonalityType));
}

void UNPC_DinosaurPersonalitySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update mood over time
    UpdateMoodOverTime(DeltaTime);
    
    // Decay stress over time
    StressLevel = FMath::Max(0.0f, StressLevel - (DeltaTime * 0.1f));
    
    // Update threat memory
    UpdateThreatMemory(DeltaTime);
}

void UNPC_DinosaurPersonalitySystem::DeterminePersonalityType()
{
    // Determine personality type based on trait combinations
    if (Aggressiveness > 0.7f && Territoriality > 0.6f)
    {
        PersonalityType = ENPC_PersonalityType::Aggressive;
    }
    else if (Fearfulness > 0.7f && Sociability < 0.3f)
    {
        PersonalityType = ENPC_PersonalityType::Timid;
    }
    else if (Curiosity > 0.7f && Fearfulness < 0.4f)
    {
        PersonalityType = ENPC_PersonalityType::Curious;
    }
    else if (Sociability > 0.7f && Aggressiveness < 0.4f)
    {
        PersonalityType = ENPC_PersonalityType::Social;
    }
    else if (Territoriality > 0.8f && Sociability < 0.3f)
    {
        PersonalityType = ENPC_PersonalityType::Territorial;
    }
    else
    {
        PersonalityType = ENPC_PersonalityType::Balanced;
    }
}

void UNPC_DinosaurPersonalitySystem::InitializeMoodFromPersonality()
{
    switch (PersonalityType)
    {
        case ENPC_PersonalityType::Aggressive:
            CurrentMood = ENPC_MoodState::Agitated;
            break;
        case ENPC_PersonalityType::Timid:
            CurrentMood = ENPC_MoodState::Fearful;
            break;
        case ENPC_PersonalityType::Curious:
            CurrentMood = ENPC_MoodState::Alert;
            break;
        case ENPC_PersonalityType::Social:
            CurrentMood = ENPC_MoodState::Content;
            break;
        default:
            CurrentMood = ENPC_MoodState::Neutral;
            break;
    }
}

void UNPC_DinosaurPersonalitySystem::UpdateMoodOverTime(float DeltaTime)
{
    // Mood naturally drifts toward neutral over time
    float MoodDecayRate = 0.05f * DeltaTime;
    
    // Adjust mood based on stress level
    if (StressLevel > 0.7f)
    {
        if (CurrentMood != ENPC_MoodState::Fearful && CurrentMood != ENPC_MoodState::Agitated)
        {
            TransitionToMood(Fearfulness > Aggressiveness ? ENPC_MoodState::Fearful : ENPC_MoodState::Agitated);
        }
    }
    else if (StressLevel < 0.2f)
    {
        // Low stress allows for positive moods
        if (FMath::RandRange(0.0f, 1.0f) < 0.01f) // Small chance per tick
        {
            TransitionToMood(ENPC_MoodState::Content);
        }
    }
}

void UNPC_DinosaurPersonalitySystem::TransitionToMood(ENPC_MoodState NewMood)
{
    if (CurrentMood != NewMood)
    {
        ENPC_MoodState PreviousMood = CurrentMood;
        CurrentMood = NewMood;
        
        // Trigger mood change event
        OnMoodChanged.Broadcast(PreviousMood, CurrentMood);
        
        UE_LOG(LogTemp, Log, TEXT("%s mood changed from %s to %s"), 
               *GetOwner()->GetName(),
               *UEnum::GetValueAsString(PreviousMood),
               *UEnum::GetValueAsString(CurrentMood));
    }
}

void UNPC_DinosaurPersonalitySystem::ReactToPlayerPresence(APawn* Player, float Distance)
{
    if (!Player) return;
    
    PlayerInteractionCount++;
    LastPlayerInteractionTime = GetWorld()->GetTimeSeconds();
    
    // Calculate reaction based on personality and distance
    float ReactionIntensity = CalculateReactionIntensity(Distance);
    
    // Apply stress based on personality
    float StressIncrease = 0.0f;
    
    switch (PersonalityType)
    {
        case ENPC_PersonalityType::Aggressive:
            StressIncrease = ReactionIntensity * 0.3f;
            if (Distance < 1000.0f) // Close proximity triggers aggression
            {
                TransitionToMood(ENPC_MoodState::Agitated);
            }
            break;
            
        case ENPC_PersonalityType::Timid:
            StressIncrease = ReactionIntensity * 0.8f;
            TransitionToMood(ENPC_MoodState::Fearful);
            break;
            
        case ENPC_PersonalityType::Curious:
            StressIncrease = ReactionIntensity * 0.2f;
            if (Distance > 500.0f && Distance < 2000.0f) // Safe observation distance
            {
                TransitionToMood(ENPC_MoodState::Alert);
            }
            break;
            
        case ENPC_PersonalityType::Social:
            StressIncrease = ReactionIntensity * 0.4f;
            if (PlayerInteractionCount > 5) // Familiar with player
            {
                TransitionToMood(ENPC_MoodState::Content);
            }
            break;
            
        case ENPC_PersonalityType::Territorial:
            if (Distance < 1500.0f) // Within territory
            {
                StressIncrease = ReactionIntensity * 0.9f;
                TransitionToMood(ENPC_MoodState::Agitated);
            }
            break;
            
        default:
            StressIncrease = ReactionIntensity * 0.5f;
            break;
    }
    
    StressLevel = FMath::Clamp(StressLevel + StressIncrease, 0.0f, 1.0f);
}

void UNPC_DinosaurPersonalitySystem::ReactToThreat(AActor* ThreatSource, float ThreatLevel)
{
    if (!ThreatSource) return;
    
    // Record threat in memory
    FNPC_ThreatMemory ThreatMemory;
    ThreatMemory.ThreatSource = ThreatSource;
    ThreatMemory.ThreatLevel = ThreatLevel;
    ThreatMemory.TimeRecorded = GetWorld()->GetTimeSeconds();
    ThreatMemory.Location = ThreatSource->GetActorLocation();
    
    ThreatMemories.Add(ThreatMemory);
    
    // React based on personality
    float StressIncrease = ThreatLevel * GetThreatSensitivity();
    StressLevel = FMath::Clamp(StressLevel + StressIncrease, 0.0f, 1.0f);
    
    // Mood response to threat
    if (ThreatLevel > 0.7f)
    {
        TransitionToMood(Aggressiveness > Fearfulness ? ENPC_MoodState::Agitated : ENPC_MoodState::Fearful);
    }
    else if (ThreatLevel > 0.3f)
    {
        TransitionToMood(ENPC_MoodState::Alert);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s detected threat from %s (Level: %.2f)"), 
           *GetOwner()->GetName(), 
           *ThreatSource->GetName(), 
           ThreatLevel);
}

float UNPC_DinosaurPersonalitySystem::CalculateReactionIntensity(float Distance) const
{
    // Closer distance = higher intensity reaction
    float MaxReactionDistance = 3000.0f;
    float NormalizedDistance = FMath::Clamp(Distance / MaxReactionDistance, 0.0f, 1.0f);
    
    // Inverse relationship: closer = more intense
    return 1.0f - NormalizedDistance;
}

float UNPC_DinosaurPersonalitySystem::GetThreatSensitivity() const
{
    float BaseSensitivity = 0.5f;
    
    // Adjust based on personality traits
    BaseSensitivity += (Fearfulness - 0.5f) * 0.4f;
    BaseSensitivity += (Aggressiveness - 0.5f) * 0.2f;
    BaseSensitivity -= (Curiosity - 0.5f) * 0.1f;
    
    return FMath::Clamp(BaseSensitivity, 0.1f, 1.0f);
}

void UNPC_DinosaurPersonalitySystem::UpdateThreatMemory(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old threat memories
    ThreatMemories.RemoveAll([CurrentTime, this](const FNPC_ThreatMemory& Memory)
    {
        return (CurrentTime - Memory.TimeRecorded) > ThreatMemoryDuration;
    });
}

bool UNPC_DinosaurPersonalitySystem::HasRecentThreatMemory(AActor* PotentialThreat) const
{
    if (!PotentialThreat) return false;
    
    return ThreatMemories.ContainsByPredicate([PotentialThreat](const FNPC_ThreatMemory& Memory)
    {
        return Memory.ThreatSource == PotentialThreat;
    });
}

float UNPC_DinosaurPersonalitySystem::GetPersonalityInfluencedValue(float BaseValue, ENPC_PersonalityTrait TraitType) const
{
    float TraitValue = 0.5f; // Default neutral
    
    switch (TraitType)
    {
        case ENPC_PersonalityTrait::Aggressiveness:
            TraitValue = Aggressiveness;
            break;
        case ENPC_PersonalityTrait::Curiosity:
            TraitValue = Curiosity;
            break;
        case ENPC_PersonalityTrait::Fearfulness:
            TraitValue = Fearfulness;
            break;
        case ENPC_PersonalityTrait::Sociability:
            TraitValue = Sociability;
            break;
        case ENPC_PersonalityTrait::Territoriality:
            TraitValue = Territoriality;
            break;
    }
    
    // Apply personality influence to base value
    float Influence = (TraitValue - 0.5f) * 2.0f; // Convert 0-1 to -1 to 1
    return FMath::Clamp(BaseValue + (BaseValue * Influence * 0.5f), 0.0f, 2.0f);
}