#include "AnimationSystemCore.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnimationSystemCore::UAnimationSystemCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    AnimationData.MovementState = ECharacterMovementState::Idle;
    AnimationData.ThreatLevel = EThreatLevel::None;
    AnimationData.CurrentTerrain = ETerrainType::Flat;
    AnimationData.StaminaLevel = 1.0f;
    AnimationData.HealthLevel = 1.0f;
    AnimationData.FearLevel = 0.0f;
    AnimationData.ConfidenceLevel = 0.5f;
    
    PreviousMovementState = ECharacterMovementState::Idle;
    PreviousThreatLevel = EThreatLevel::None;
}

void UAnimationSystemCore::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize animation system
    UE_LOG(LogTemp, Log, TEXT("Animation System Core initialized for %s"), 
           *GetOwner()->GetName());
}

void UAnimationSystemCore::TickComponent(float DeltaTime, ELevelTick TickType, 
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAnimationBlends(DeltaTime);
    ProcessThreatResponse(DeltaTime);
    UpdatePhysicalState(DeltaTime);
}

void UAnimationSystemCore::UpdateMovementState(ECharacterMovementState NewState)
{
    if (AnimationData.MovementState != NewState)
    {
        PreviousMovementState = AnimationData.MovementState;
        AnimationData.MovementState = NewState;
        StateTransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Movement state changed to: %d"), (int32)NewState);
    }
}

void UAnimationSystemCore::UpdateThreatLevel(EThreatLevel NewThreatLevel)
{
    if (AnimationData.ThreatLevel != NewThreatLevel)
    {
        PreviousThreatLevel = AnimationData.ThreatLevel;
        AnimationData.ThreatLevel = NewThreatLevel;
        ThreatTransitionTimer = 0.0f;
        
        // Immediate fear response based on threat level
        switch (NewThreatLevel)
        {
            case EThreatLevel::None:
                AnimationData.FearLevel = FMath::Max(0.0f, AnimationData.FearLevel - 0.1f);
                break;
            case EThreatLevel::Low:
                AnimationData.FearLevel = FMath::Max(0.2f, AnimationData.FearLevel);
                break;
            case EThreatLevel::Medium:
                AnimationData.FearLevel = FMath::Max(0.5f, AnimationData.FearLevel);
                break;
            case EThreatLevel::High:
                AnimationData.FearLevel = FMath::Max(0.8f, AnimationData.FearLevel);
                break;
            case EThreatLevel::Extreme:
                AnimationData.FearLevel = 1.0f;
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Threat level changed to: %d, Fear level: %f"), 
               (int32)NewThreatLevel, AnimationData.FearLevel);
    }
}

void UAnimationSystemCore::UpdateTerrainType(ETerrainType NewTerrain)
{
    AnimationData.CurrentTerrain = NewTerrain;
}

UPoseSearchDatabase* UAnimationSystemCore::GetCurrentDatabase() const
{
    // Select appropriate database based on current state
    switch (AnimationData.MovementState)
    {
        case ECharacterMovementState::Idle:
        case ECharacterMovementState::Observing:
            return IdleDatabase;
            
        case ECharacterMovementState::Panicked:
            return PanicDatabase;
            
        case ECharacterMovementState::Crouched:
        case ECharacterMovementState::Hiding:
            return StealthDatabase;
            
        case ECharacterMovementState::Cautious:
        case ECharacterMovementState::Normal:
        case ECharacterMovementState::Injured:
        case ECharacterMovementState::Exhausted:
        default:
            return LocomotionDatabase;
    }
}

float UAnimationSystemCore::GetBlendWeight(ECharacterMovementState State) const
{
    if (AnimationData.MovementState == State)
    {
        return FMath::Clamp(StateTransitionTimer / (1.0f / StateTransitionSpeed), 0.0f, 1.0f);
    }
    else if (PreviousMovementState == State)
    {
        return 1.0f - FMath::Clamp(StateTransitionTimer / (1.0f / StateTransitionSpeed), 0.0f, 1.0f);
    }
    
    return 0.0f;
}

void UAnimationSystemCore::UpdateAnimationBlends(float DeltaTime)
{
    StateTransitionTimer += DeltaTime;
    ThreatTransitionTimer += DeltaTime;
    
    // Update movement speed based on state and threat
    float BaseSpeed = 1.0f;
    
    switch (AnimationData.MovementState)
    {
        case ECharacterMovementState::Cautious:
            BaseSpeed = 0.6f;
            break;
        case ECharacterMovementState::Panicked:
            BaseSpeed = 1.8f;
            break;
        case ECharacterMovementState::Crouched:
            BaseSpeed = 0.3f;
            break;
        case ECharacterMovementState::Injured:
            BaseSpeed = 0.4f;
            break;
        case ECharacterMovementState::Exhausted:
            BaseSpeed = 0.5f;
            break;
        case ECharacterMovementState::Hiding:
            BaseSpeed = 0.1f;
            break;
        default:
            BaseSpeed = 1.0f;
            break;
    }
    
    // Apply threat level modifier
    float ThreatModifier = 1.0f + (AnimationData.FearLevel * 0.5f);
    AnimationData.MovementSpeed = BaseSpeed * ThreatModifier;
}

void UAnimationSystemCore::ProcessThreatResponse(float DeltaTime)
{
    // Gradually reduce fear when no threat is present
    if (AnimationData.ThreatLevel == EThreatLevel::None)
    {
        AnimationData.FearLevel = FMath::FInterpTo(AnimationData.FearLevel, 0.0f, DeltaTime, 2.0f);
    }
    
    // Update confidence based on recent experiences
    if (AnimationData.ThreatLevel == EThreatLevel::None && AnimationData.FearLevel < 0.3f)
    {
        AnimationData.ConfidenceLevel = FMath::FInterpTo(AnimationData.ConfidenceLevel, 0.7f, DeltaTime, 1.0f);
    }
    else if (AnimationData.ThreatLevel >= EThreatLevel::High)
    {
        AnimationData.ConfidenceLevel = FMath::FInterpTo(AnimationData.ConfidenceLevel, 0.1f, DeltaTime, 3.0f);
    }
}

void UAnimationSystemCore::UpdatePhysicalState(float DeltaTime)
{
    // Stamina consumption based on movement state
    float StaminaDrain = 0.0f;
    
    switch (AnimationData.MovementState)
    {
        case ECharacterMovementState::Panicked:
            StaminaDrain = 0.3f;
            break;
        case ECharacterMovementState::Normal:
            StaminaDrain = 0.1f;
            break;
        case ECharacterMovementState::Cautious:
            StaminaDrain = 0.05f;
            break;
        default:
            StaminaDrain = 0.0f;
            break;
    }
    
    // Apply stamina changes
    if (StaminaDrain > 0.0f)
    {
        AnimationData.StaminaLevel = FMath::Max(0.0f, AnimationData.StaminaLevel - (StaminaDrain * DeltaTime));
    }
    else
    {
        // Regenerate stamina when resting
        AnimationData.StaminaLevel = FMath::Min(1.0f, AnimationData.StaminaLevel + (0.2f * DeltaTime));
    }
    
    // Force exhausted state when stamina is very low
    if (AnimationData.StaminaLevel < 0.2f && AnimationData.MovementState != ECharacterMovementState::Exhausted)
    {
        UpdateMovementState(ECharacterMovementState::Exhausted);
    }
}