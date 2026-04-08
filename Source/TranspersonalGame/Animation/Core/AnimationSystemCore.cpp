#include "AnimationSystemCore.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

UAnimationSystemCore::UAnimationSystemCore()
{
    // Initialize with default values
}

UPoseSearchDatabase* UAnimationSystemCore::GetMotionMatchingDatabase(ECreatureArchetype Archetype, EEmotionalState State)
{
    // Select appropriate database based on creature type and emotional state
    switch (State)
    {
        case EEmotionalState::Calm:
        case EEmotionalState::Curious:
        case EEmotionalState::Resting:
            if (BaseLocomotionDatabases.Contains(Archetype))
            {
                return BaseLocomotionDatabases[Archetype];
            }
            break;
            
        case EEmotionalState::Aggressive:
        case EEmotionalState::Hunting:
        case EEmotionalState::Territorial:
            if (CombatDatabases.Contains(Archetype))
            {
                return CombatDatabases[Archetype];
            }
            break;
            
        case EEmotionalState::Feeding:
            if (FeedingDatabases.Contains(Archetype))
            {
                return FeedingDatabases[Archetype];
            }
            break;
            
        default:
            break;
    }
    
    // Fallback to base locomotion
    if (BaseLocomotionDatabases.Contains(Archetype))
    {
        return BaseLocomotionDatabases[Archetype];
    }
    
    return nullptr;
}

FCreaturePersonality UAnimationSystemCore::GenerateUniquePersonality(ECreatureArchetype Archetype)
{
    FCreaturePersonality Personality;
    
    // Archetype-specific personality tendencies
    switch (Archetype)
    {
        case ECreatureArchetype::Paleontologist:
            // Human character - more controlled, analytical movements
            Personality.Nervousness = FMath::RandRange(0.4f, 0.7f);
            Personality.Aggression = FMath::RandRange(0.1f, 0.3f);
            Personality.Curiosity = FMath::RandRange(0.7f, 0.9f);
            break;
            
        case ECreatureArchetype::SmallHerbivore:
            // High nervousness, low aggression, moderate curiosity
            Personality.Nervousness = FMath::RandRange(0.7f, 0.9f);
            Personality.Aggression = FMath::RandRange(0.1f, 0.3f);
            Personality.Curiosity = FMath::RandRange(0.5f, 0.8f);
            break;
            
        case ECreatureArchetype::ApexPredator:
            // Low nervousness, high aggression, moderate curiosity
            Personality.Nervousness = FMath::RandRange(0.1f, 0.3f);
            Personality.Aggression = FMath::RandRange(0.7f, 0.9f);
            Personality.Curiosity = FMath::RandRange(0.4f, 0.6f);
            break;
            
        case ECreatureArchetype::SmallCarnivore:
            // Pack hunters - moderate nervousness, high aggression when in groups
            Personality.Nervousness = FMath::RandRange(0.4f, 0.6f);
            Personality.Aggression = FMath::RandRange(0.6f, 0.8f);
            Personality.Curiosity = FMath::RandRange(0.6f, 0.8f);
            break;
            
        default:
            // Use default random values
            break;
    }
    
    return Personality;
}

bool UAnimationSystemCore::ShouldUseStealthMovement(const AActor* Character, const AActor* NearbyThreat)
{
    if (!Character || !NearbyThreat)
    {
        return false;
    }
    
    // Calculate distance and threat level
    float Distance = FVector::Dist(Character->GetActorLocation(), NearbyThreat->GetActorLocation());
    
    // If within 1000 units and threat is larger, use stealth movement
    const float StealthDistance = 1000.0f;
    
    if (Distance < StealthDistance)
    {
        // Additional logic could check threat size, aggression level, etc.
        return true;
    }
    
    return false;
}

float UAnimationSystemCore::CalculateFearIntensity(const AActor* Character, TArray<AActor*> NearbyThreats)
{
    if (!Character || NearbyThreats.Num() == 0)
    {
        return 0.0f;
    }
    
    float FearIntensity = 0.0f;
    FVector CharacterLocation = Character->GetActorLocation();
    
    for (const AActor* Threat : NearbyThreats)
    {
        if (!Threat)
        {
            continue;
        }
        
        float Distance = FVector::Dist(CharacterLocation, Threat->GetActorLocation());
        
        // Fear decreases with distance, increases with proximity
        float DistanceFactor = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f);
        
        // Multiple threats compound fear
        FearIntensity += DistanceFactor * 0.3f;
    }
    
    return FMath::Clamp(FearIntensity, 0.0f, 1.0f);
}