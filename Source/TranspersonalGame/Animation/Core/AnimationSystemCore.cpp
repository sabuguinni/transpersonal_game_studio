#include "AnimationSystemCore.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UAnimationSystemCore::UAnimationSystemCore()
{
    // Initialize default animation state
    CurrentAnimationState.EmotionalState = ECharacterEmotionalState::Calm;
    CurrentAnimationState.MovementIntention = EMovementIntention::Idle;
    CurrentAnimationState.FearLevel = 0.0f;
    CurrentAnimationState.StaminaLevel = 1.0f;
    CurrentAnimationState.GroundSlope = 0.0f;
    CurrentAnimationState.MovementDirection = FVector::ZeroVector;
    CurrentAnimationState.MovementSpeed = 0.0f;
    CurrentAnimationState.bIsCarryingObject = false;
    CurrentAnimationState.ObjectWeight = 0.0f;

    TargetAnimationState = CurrentAnimationState;
}

void UAnimationSystemCore::UpdateCharacterState(const FCharacterAnimationState& NewState)
{
    TargetAnimationState = NewState;
    
    // Immediate updates for critical states
    if (NewState.FearLevel > CurrentAnimationState.FearLevel + 0.3f)
    {
        CurrentAnimationState.FearLevel = NewState.FearLevel;
        CurrentAnimationState.EmotionalState = NewState.EmotionalState;
    }
    
    // Gradual blending for other properties
    CurrentAnimationState.MovementIntention = NewState.MovementIntention;
    CurrentAnimationState.MovementDirection = NewState.MovementDirection;
    CurrentAnimationState.MovementSpeed = NewState.MovementSpeed;
    CurrentAnimationState.bIsCarryingObject = NewState.bIsCarryingObject;
    CurrentAnimationState.ObjectWeight = NewState.ObjectWeight;
}

void UAnimationSystemCore::SetFearLevel(float NewFearLevel)
{
    NewFearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
    
    // Update emotional state based on fear level
    ECharacterEmotionalState NewEmotionalState = ECharacterEmotionalState::Calm;
    
    if (NewFearLevel > 0.8f)
    {
        NewEmotionalState = ECharacterEmotionalState::Terrified;
    }
    else if (NewFearLevel > 0.5f)
    {
        NewEmotionalState = ECharacterEmotionalState::Fearful;
    }
    else if (NewFearLevel > 0.2f)
    {
        NewEmotionalState = ECharacterEmotionalState::Cautious;
    }
    
    CurrentAnimationState.FearLevel = NewFearLevel;
    CurrentAnimationState.EmotionalState = NewEmotionalState;
    TargetAnimationState.FearLevel = NewFearLevel;
    TargetAnimationState.EmotionalState = NewEmotionalState;
}

void UAnimationSystemCore::TriggerEmotionalResponse(ECharacterEmotionalState NewEmotion, float Duration)
{
    CurrentAnimationState.EmotionalState = NewEmotion;
    EmotionalTransitionTimer = 0.0f;
    EmotionalTransitionDuration = Duration;
    
    // Set appropriate fear level for the emotion
    switch (NewEmotion)
    {
        case ECharacterEmotionalState::Terrified:
            CurrentAnimationState.FearLevel = 1.0f;
            break;
        case ECharacterEmotionalState::Fearful:
            CurrentAnimationState.FearLevel = 0.7f;
            break;
        case ECharacterEmotionalState::Cautious:
            CurrentAnimationState.FearLevel = 0.4f;
            break;
        case ECharacterEmotionalState::Curious:
            CurrentAnimationState.FearLevel = 0.1f;
            break;
        case ECharacterEmotionalState::Focused:
            CurrentAnimationState.FearLevel = 0.2f;
            break;
        default:
            CurrentAnimationState.FearLevel = 0.0f;
            break;
    }
}