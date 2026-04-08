#include "AnimationSystemCore.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"

UAnimationSystemCore::UAnimationSystemCore()
{
    // Initialize default animation state
    CurrentAnimationState.EmotionalState = ECharacterEmotionalState::Cautious;
    CurrentAnimationState.MovementContext = EMovementContext::OpenArea;
    CurrentAnimationState.ThreatLevel = EThreatLevel::None;
    CurrentAnimationState.FatigueLevel = 0.0f;
    CurrentAnimationState.ExperienceLevel = 0.0f;
    CurrentAnimationState.TimeInCurrentArea = 0.0f;
    CurrentAnimationState.bIsCarryingWeight = false;
    CurrentAnimationState.bIsInjured = false;
    CurrentAnimationState.InjurySeverity = 0.0f;

    PreviousAnimationState = CurrentAnimationState;
}

void UAnimationSystemCore::UpdateAnimationState(const FCharacterAnimationState& NewState)
{
    PreviousAnimationState = CurrentAnimationState;
    CurrentAnimationState = NewState;
    StateTransitionTime = 0.0f;
    
    // Log significant state changes for debugging
    if (PreviousAnimationState.EmotionalState != CurrentAnimationState.EmotionalState)
    {
        UE_LOG(LogTemp, Log, TEXT("Emotional State Changed: %d -> %d"), 
            (int32)PreviousAnimationState.EmotionalState, 
            (int32)CurrentAnimationState.EmotionalState);
    }
    
    if (PreviousAnimationState.ThreatLevel != CurrentAnimationState.ThreatLevel)
    {
        UE_LOG(LogTemp, Log, TEXT("Threat Level Changed: %d -> %d"), 
            (int32)PreviousAnimationState.ThreatLevel, 
            (int32)CurrentAnimationState.ThreatLevel);
    }
}

void UAnimationSystemCore::SetMotionMatchingDatabase(UPoseSearchDatabase* Database)
{
    CurrentMotionDatabase = Database;
    UE_LOG(LogTemp, Log, TEXT("Motion Matching Database Updated"));
}

void UAnimationSystemCore::UpdateFootIK(float DeltaTime)
{
    // This will be called from the Animation Blueprint
    // Foot IK logic will be implemented in the Animation Blueprint nodes
    StateTransitionTime += DeltaTime;
    LastUpdateTime = DeltaTime;
}

void UAnimationSystemCore::SetIKFootTarget(bool bLeftFoot, FVector TargetLocation, FRotator TargetRotation)
{
    if (bLeftFoot)
    {
        LeftFootIKLocation = TargetLocation;
        LeftFootIKRotation = TargetRotation;
    }
    else
    {
        RightFootIKLocation = TargetLocation;
        RightFootIKRotation = TargetRotation;
    }
}

float UAnimationSystemCore::CalculateMovementBlendWeight(ECharacterEmotionalState State, EThreatLevel Threat) const
{
    float BaseWeight = 1.0f;
    
    // Emotional state modifiers
    switch (State)
    {
        case ECharacterEmotionalState::Terrified:
            BaseWeight *= 1.5f; // More erratic, faster movements
            break;
        case ECharacterEmotionalState::Anxious:
            BaseWeight *= 1.2f; // Slightly heightened movements
            break;
        case ECharacterEmotionalState::Cautious:
            BaseWeight *= 1.0f; // Normal baseline
            break;
        case ECharacterEmotionalState::Focused:
            BaseWeight *= 0.9f; // More controlled movements
            break;
        case ECharacterEmotionalState::Confident:
            BaseWeight *= 0.8f; // Smoother, more efficient
            break;
        case ECharacterEmotionalState::Exhausted:
            BaseWeight *= 0.6f; // Slower, more labored
            break;
    }
    
    // Threat level modifiers
    switch (Threat)
    {
        case EThreatLevel::None:
            BaseWeight *= 1.0f;
            break;
        case EThreatLevel::Distant:
            BaseWeight *= 1.1f; // Slight tension
            break;
        case EThreatLevel::Nearby:
            BaseWeight *= 1.3f; // Heightened alertness
            break;
        case EThreatLevel::Immediate:
            BaseWeight *= 1.6f; // High stress movements
            break;
        case EThreatLevel::Combat:
            BaseWeight *= 2.0f; // Maximum intensity
            break;
    }
    
    // Factor in fatigue - tired characters move differently
    float FatigueModifier = FMath::Lerp(1.0f, 0.7f, CurrentAnimationState.FatigueLevel);
    BaseWeight *= FatigueModifier;
    
    // Factor in experience - experienced characters are more efficient
    float ExperienceModifier = FMath::Lerp(1.0f, 0.85f, CurrentAnimationState.ExperienceLevel);
    BaseWeight *= ExperienceModifier;
    
    return FMath::Clamp(BaseWeight, 0.1f, 3.0f);
}

float UAnimationSystemCore::GetContextualSpeedModifier(EMovementContext Context) const
{
    switch (Context)
    {
        case EMovementContext::OpenArea:
            return 1.0f; // Normal speed
        case EMovementContext::DenseVegetation:
            return 0.7f; // Slower, more careful
        case EMovementContext::NearWater:
            return 0.8f; // Cautious near water
        case EMovementContext::HighGround:
            return 1.1f; // Confident on high ground
        case EMovementContext::Hiding:
            return 0.3f; // Very slow, stealthy
        case EMovementContext::Stalking:
            return 0.5f; // Slow, deliberate
        default:
            return 1.0f;
    }
}