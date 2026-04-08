#include "AnimationSystemCore.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/PoseSearch/PoseSearchLibrary.h"
#include "DrawDebugHelpers.h"

UAnimationSystemCore::UAnimationSystemCore()
{
    // Initialize default character profile
    CurrentProfile.Confidence = 0.5f;
    CurrentProfile.Nervousness = 0.3f;
    CurrentProfile.Fatigue = 0.0f;
    CurrentProfile.InjuryLevel = 0.0f;
    CurrentProfile.BaseMovementSpeed = 1.0f;
    CurrentProfile.StepVariation = 0.1f;
    CurrentProfile.PosturalTension = 0.2f;

    CurrentTerrainType = ETerrainAdaptation::Flat;
    FootIKTargets.SetNum(2); // Left and right foot
}

void UAnimationSystemCore::InitializeMotionMatchingDatabases()
{
    // This will be populated with actual database assets
    // For now, we establish the structure
    
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemCore: Initializing Motion Matching databases"));
    
    // Movement state databases would be loaded here
    // Each database contains animations specific to that movement state
    // organized by character archetype and emotional state
}

UPoseSearchDatabase* UAnimationSystemCore::GetDatabaseForState(ECharacterMovementState State, ETerrainAdaptation Terrain)
{
    // Primary database selection based on movement state
    UPoseSearchDatabase** PrimaryDatabase = MovementDatabases.Find(State);
    if (PrimaryDatabase && *PrimaryDatabase)
    {
        return *PrimaryDatabase;
    }

    // Fallback to terrain-specific database
    UPoseSearchDatabase** TerrainDatabase = TerrainDatabases.Find(Terrain);
    if (TerrainDatabase && *TerrainDatabase)
    {
        return *TerrainDatabase;
    }

    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemCore: No database found for state %d, terrain %d"), 
           (int32)State, (int32)Terrain);
    
    return nullptr;
}

void UAnimationSystemCore::SetCharacterAnimationProfile(const FCharacterAnimationProfile& Profile)
{
    CurrentProfile = Profile;
    
    // Recalculate movement parameters based on new profile
    // This affects how Motion Matching selects animations
    
    UE_LOG(LogTemp, Log, TEXT("AnimationSystemCore: Updated character profile - Confidence: %f, Nervousness: %f"), 
           Profile.Confidence, Profile.Nervousness);
}

void UAnimationSystemCore::UpdateTerrainAdaptation(const FVector& FootLocation, float TerrainHeight)
{
    // Analyze terrain at foot location
    AnalyzeTerrainType(FootLocation);
    
    // Update IK targets based on terrain
    // This ensures feet properly contact uneven surfaces
    for (int32 i = 0; i < FootIKTargets.Num(); i++)
    {
        FootIKTargets[i].Z = TerrainHeight;
    }
}

void UAnimationSystemCore::SetIKTargets(const TArray<FVector>& FootTargets)
{
    FootIKTargets = FootTargets;
    
    // Validate IK targets are within reasonable range
    for (const FVector& Target : FootIKTargets)
    {
        float Distance = FVector::Dist(Target, FVector::ZeroVector);
        if (Distance > MaxIKReach)
        {
            UE_LOG(LogTemp, Warning, TEXT("AnimationSystemCore: IK target exceeds max reach: %f"), Distance);
        }
    }
}

void UAnimationSystemCore::AnalyzeTerrainType(const FVector& Location)
{
    // This would integrate with the Procedural World Generator's terrain data
    // For now, we establish the interface
    
    // Terrain analysis would consider:
    // - Surface angle (flat, uphill, downhill)
    // - Material type (rock, mud, vegetation)
    // - Stability (solid, loose, slippery)
    
    CurrentTerrainType = ETerrainAdaptation::Flat; // Placeholder
}

float UAnimationSystemCore::CalculateMovementWeight(ECharacterMovementState State) const
{
    float BaseWeight = 1.0f;
    
    // Modify weight based on character profile
    switch (State)
    {
        case ECharacterMovementState::Walking:
            BaseWeight *= (1.0f - CurrentProfile.Nervousness * 0.3f);
            BaseWeight *= (1.0f - CurrentProfile.Fatigue * 0.5f);
            break;
            
        case ECharacterMovementState::Running:
            BaseWeight *= CurrentProfile.Confidence;
            BaseWeight *= (1.0f - CurrentProfile.InjuryLevel * 0.8f);
            break;
            
        case ECharacterMovementState::Crouching:
            BaseWeight *= (1.0f + CurrentProfile.Nervousness * 0.5f);
            break;
            
        case ECharacterMovementState::Fearful:
            BaseWeight *= CurrentProfile.Nervousness;
            BaseWeight *= (1.0f + CurrentProfile.PosturalTension);
            break;
            
        default:
            break;
    }
    
    return FMath::Clamp(BaseWeight, 0.1f, 2.0f);
}