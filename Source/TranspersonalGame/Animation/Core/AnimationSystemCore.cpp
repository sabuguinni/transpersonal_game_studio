#include "AnimationSystemCore.h"
#include "Animation/PoseSearch/PoseSearchDatabase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

UAnimationSystemCore::UAnimationSystemCore()
{
    CurrentAnimationState = ECharacterAnimationState::Idle_Calm;
    StateTransitionTime = 0.0f;
    EmotionalTransitionProgress = 0.0f;
}

void UAnimationSystemCore::InitializeForCharacter(USkeletalMeshComponent* SkeletalMesh, const FCharacterAnimationProfile& Profile)
{
    if (!SkeletalMesh)
    {
        UE_LOG(LogAnimation, Warning, TEXT("AnimationSystemCore: Cannot initialize with null SkeletalMesh"));
        return;
    }
    
    CurrentProfile = Profile;
    CurrentAnimationState = ECharacterAnimationState::Idle_Calm;
    StateTransitionTime = 0.0f;
    EmotionalTransitionProgress = 0.0f;
    
    UE_LOG(LogAnimation, Log, TEXT("AnimationSystemCore: Initialized for character with emotional state: %d"), 
           (int32)CurrentProfile.CurrentEmotionalState);
}

void UAnimationSystemCore::UpdateAnimationState(ECharacterAnimationState NewState, float DeltaTime)
{
    if (CurrentAnimationState != NewState)
    {
        // Start transition to new state
        CurrentAnimationState = NewState;
        StateTransitionTime = 0.0f;
        
        UE_LOG(LogAnimation, Verbose, TEXT("AnimationSystemCore: Transitioning to state: %d"), (int32)NewState);
    }
    
    StateTransitionTime += DeltaTime;
    UpdateEmotionalTransition(DeltaTime);
}

void UAnimationSystemCore::UpdateEmotionalState(EEmotionalState NewState, float TransitionTime)
{
    if (CurrentProfile.CurrentEmotionalState != NewState)
    {
        CurrentProfile.CurrentEmotionalState = NewState;
        EmotionalTransitionProgress = 0.0f;
        
        UE_LOG(LogAnimation, Log, TEXT("AnimationSystemCore: Emotional state changed to: %d"), (int32)NewState);
    }
}

UPoseSearchDatabase* UAnimationSystemCore::GetCurrentMotionDatabase() const
{
    return CurrentProfile.MotionDatabase;
}

float UAnimationSystemCore::CalculateEmotionalBlendWeight(EEmotionalState TargetState) const
{
    if (CurrentProfile.CurrentEmotionalState == TargetState)
    {
        return FMath::Clamp(EmotionalTransitionProgress, 0.0f, 1.0f);
    }
    
    // Calculate cross-emotional influences
    switch (CurrentProfile.CurrentEmotionalState)
    {
        case EEmotionalState::Terrified:
            if (TargetState == EEmotionalState::Anxious) return 0.7f;
            break;
            
        case EEmotionalState::Anxious:
            if (TargetState == EEmotionalState::Calm) return 0.3f;
            if (TargetState == EEmotionalState::Terrified) return 0.4f;
            break;
            
        case EEmotionalState::Exhausted:
            // Exhaustion affects all movements
            return FMath::Lerp(0.2f, 0.8f, CurrentProfile.FatigueLevel);
            
        default:
            break;
    }
    
    return 0.0f;
}

void UAnimationSystemCore::UpdateEmotionalTransition(float DeltaTime)
{
    // Emotional transitions are influenced by fear and fatigue levels
    float TransitionSpeed = 1.0f;
    
    // High fear makes emotional changes faster
    TransitionSpeed *= (1.0f + CurrentProfile.FearLevel * 2.0f);
    
    // High fatigue makes emotional changes slower
    TransitionSpeed *= (1.0f - CurrentProfile.FatigueLevel * 0.5f);
    
    EmotionalTransitionProgress = FMath::Clamp(
        EmotionalTransitionProgress + (DeltaTime * TransitionSpeed), 
        0.0f, 
        1.0f
    );
}

ECharacterAnimationState UAnimationSystemCore::SelectOptimalAnimationState() const
{
    // This would contain logic to select the best animation state
    // based on current emotional state, fatigue, fear, etc.
    // For now, return current state
    return CurrentAnimationState;
}