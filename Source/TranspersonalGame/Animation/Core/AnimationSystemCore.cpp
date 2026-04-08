#include "AnimationSystemCore.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

UAnimationSystemCore::UAnimationSystemCore()
{
    // Initialize default animation state
    CurrentAnimationState = FAnimationStateData();
    StateTransitionTimer = 0.0f;
    PreviousState = ECharacterAnimationState::Idle;
}

void UAnimationSystemCore::UpdateAnimationState(const FAnimationStateData& NewState)
{
    // Store previous state for blend calculations
    PreviousState = CurrentAnimationState.CurrentState;
    
    // Update current state
    CurrentAnimationState = NewState;
    
    // Reset transition timer
    StateTransitionTimer = 0.0f;
    
    // Log state change for debugging
    UE_LOG(LogTemp, Log, TEXT("Animation State Changed: %s -> %s"), 
           *UEnum::GetValueAsString(PreviousState),
           *UEnum::GetValueAsString(CurrentAnimationState.CurrentState));
}

UPoseSearchDatabase* UAnimationSystemCore::GetActiveDatabase() const
{
    // Select database based on current state and fear level
    switch (CurrentAnimationState.CurrentState)
    {
        case ECharacterAnimationState::Idle:
        case ECharacterAnimationState::Walking:
        case ECharacterAnimationState::Jogging:
            if (CurrentAnimationState.FearLevel > 0.7f)
            {
                return PanicMovementDatabase;
            }
            else if (CurrentAnimationState.CautionLevel > 0.6f)
            {
                return CautiousMovementDatabase;
            }
            return LocomotionDatabase;
            
        case ECharacterAnimationState::Cautious:
        case ECharacterAnimationState::Sneaking:
        case ECharacterAnimationState::Hiding:
        case ECharacterAnimationState::Observing:
            return CautiousMovementDatabase;
            
        case ECharacterAnimationState::Running:
            return PanicMovementDatabase;
            
        case ECharacterAnimationState::Gathering:
        case ECharacterAnimationState::Crafting:
        case ECharacterAnimationState::Climbing:
            return InteractionDatabase;
            
        default:
            return LocomotionDatabase;
    }
}

float UAnimationSystemCore::CalculateBlendTime(ECharacterAnimationState FromState, ECharacterAnimationState ToState) const
{
    // Fast transitions for panic states
    if (ToState == ECharacterAnimationState::Running || 
        FromState == ECharacterAnimationState::Running)
    {
        return FAST_BLEND_TIME;
    }
    
    // Slow transitions for careful movements
    if (ToState == ECharacterAnimationState::Cautious ||
        ToState == ECharacterAnimationState::Sneaking ||
        ToState == ECharacterAnimationState::Hiding)
    {
        return SLOW_BLEND_TIME;
    }
    
    // Normal transitions for most cases
    return NORMAL_BLEND_TIME;
}

void UAnimationSystemCore::SetFearLevel(float NewFearLevel)
{
    CurrentAnimationState.FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
    
    // Automatically adjust caution level based on fear
    CurrentAnimationState.CautionLevel = FMath::Max(CurrentAnimationState.CautionLevel, 
                                                   CurrentAnimationState.FearLevel * 0.8f);
}

void UAnimationSystemCore::SetTerrainType(ETerrainType NewTerrain)
{
    CurrentAnimationState.CurrentTerrain = NewTerrain;
    
    // Adjust caution level based on terrain difficulty
    switch (NewTerrain)
    {
        case ETerrainType::Rocky:
        case ETerrainType::Muddy:
            CurrentAnimationState.CautionLevel = FMath::Max(CurrentAnimationState.CautionLevel, 0.7f);
            break;
        case ETerrainType::Uphill:
        case ETerrainType::Downhill:
            CurrentAnimationState.CautionLevel = FMath::Max(CurrentAnimationState.CautionLevel, 0.6f);
            break;
        case ETerrainType::Vegetation:
        case ETerrainType::Water:
            CurrentAnimationState.CautionLevel = FMath::Max(CurrentAnimationState.CautionLevel, 0.5f);
            break;
        default:
            // Flat terrain - no additional caution
            break;
    }
}