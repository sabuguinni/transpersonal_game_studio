#include "Anim_BlendSpaceManager.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

UAnim_BlendSpaceManager::UAnim_BlendSpaceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentLocomotionState = EAnim_LocomotionState::Idle;
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 375.0f;
    SprintSpeedThreshold = 600.0f;
    
    // Initialize blend space data
    BlendSpaceData = FAnim_BlendSpaceData();
}

void UAnim_BlendSpaceManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Validate that we have the necessary animation assets
    if (!ValidateAnimationAssets())
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation assets not fully configured for BlendSpaceManager"));
    }
    
    // Create runtime blend space if needed
    CreateRuntimeBlendSpace();
}

void UAnim_BlendSpaceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Get owner character
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Get movement component
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    // Calculate current speed and direction
    FVector Velocity = MovementComp->Velocity;
    float CurrentSpeed = Velocity.Size();
    FVector2D Direction = FVector2D(Velocity.X, Velocity.Y).GetSafeNormal();
    
    // Determine locomotion state
    bool bIsCrouching = MovementComp->IsCrouching();
    bool bIsInAir = MovementComp->IsFalling();
    
    EAnim_LocomotionState NewState = GetLocomotionStateFromSpeed(CurrentSpeed, bIsCrouching, bIsInAir);
    
    // Update state if changed
    if (NewState != CurrentLocomotionState)
    {
        if (CanTransitionToState(CurrentLocomotionState, NewState))
        {
            SetLocomotionState(NewState);
        }
    }
    
    // Update blend space input
    UpdateBlendSpaceInput(CurrentSpeed, Direction);
}

void UAnim_BlendSpaceManager::UpdateBlendSpaceInput(float Speed, FVector2D Direction)
{
    // Normalize speed for blend space (0.0 = idle, 1.0 = sprint)
    float NormalizedSpeed = 0.0f;
    
    if (Speed > WalkSpeedThreshold)
    {
        if (Speed <= RunSpeedThreshold)
        {
            // Walking range (0.0 to 0.5)
            NormalizedSpeed = FMath::GetMappedRangeValueClamped(
                FVector2D(WalkSpeedThreshold, RunSpeedThreshold),
                FVector2D(0.0f, 0.5f),
                Speed
            );
        }
        else if (Speed <= SprintSpeedThreshold)
        {
            // Running range (0.5 to 1.0)
            NormalizedSpeed = FMath::GetMappedRangeValueClamped(
                FVector2D(RunSpeedThreshold, SprintSpeedThreshold),
                FVector2D(0.5f, 1.0f),
                Speed
            );
        }
        else
        {
            // Sprinting (1.0)
            NormalizedSpeed = 1.0f;
        }
    }
    
    // Update blend space data
    BlendSpaceData.BlendSpaceInput = NormalizedSpeed;
    BlendSpaceData.DirectionalInput = Direction;
    
    // Calculate internal blend space values
    CalculateBlendSpaceValues(Speed, Direction);
}

void UAnim_BlendSpaceManager::SetLocomotionState(EAnim_LocomotionState NewState)
{
    if (CurrentLocomotionState != NewState)
    {
        EAnim_LocomotionState PreviousState = CurrentLocomotionState;
        CurrentLocomotionState = NewState;
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("Locomotion state changed from %d to %d"), 
               (int32)PreviousState, (int32)CurrentLocomotionState);
    }
}

EAnim_LocomotionState UAnim_BlendSpaceManager::GetLocomotionStateFromSpeed(float Speed, bool bIsCrouching, bool bIsInAir)
{
    // Handle airborne states first
    if (bIsInAir)
    {
        // Determine if jumping or falling based on velocity
        ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
        if (OwnerCharacter)
        {
            FVector Velocity = OwnerCharacter->GetVelocity();
            if (Velocity.Z > 0.0f)
            {
                return EAnim_LocomotionState::Jumping;
            }
            else
            {
                return EAnim_LocomotionState::Falling;
            }
        }
        return EAnim_LocomotionState::Falling;
    }
    
    // Handle crouching states
    if (bIsCrouching)
    {
        return EAnim_LocomotionState::Crouching;
    }
    
    // Handle ground locomotion based on speed
    if (Speed < WalkSpeedThreshold)
    {
        return EAnim_LocomotionState::Idle;
    }
    else if (Speed < RunSpeedThreshold)
    {
        return EAnim_LocomotionState::Walking;
    }
    else if (Speed < SprintSpeedThreshold)
    {
        return EAnim_LocomotionState::Running;
    }
    else
    {
        return EAnim_LocomotionState::Sprinting;
    }
}

void UAnim_BlendSpaceManager::CreateRuntimeBlendSpace()
{
    // This function would create blend spaces at runtime if needed
    // For now, we'll rely on pre-created blend space assets
    
    if (!BlendSpaceData.LocomotionBlendSpace)
    {
        UE_LOG(LogTemp, Warning, TEXT("No locomotion blend space assigned to BlendSpaceManager"));
    }
    
    if (!BlendSpaceData.DirectionalBlendSpace)
    {
        UE_LOG(LogTemp, Warning, TEXT("No directional blend space assigned to BlendSpaceManager"));
    }
}

bool UAnim_BlendSpaceManager::ValidateAnimationAssets()
{
    bool bAllAssetsValid = true;
    
    // Check essential animations
    if (!AnimationSequences.IdleAnimation)
    {
        UE_LOG(LogTemp, Warning, TEXT("Missing idle animation"));
        bAllAssetsValid = false;
    }
    
    if (!AnimationSequences.WalkAnimation)
    {
        UE_LOG(LogTemp, Warning, TEXT("Missing walk animation"));
        bAllAssetsValid = false;
    }
    
    if (!AnimationSequences.RunAnimation)
    {
        UE_LOG(LogTemp, Warning, TEXT("Missing run animation"));
        bAllAssetsValid = false;
    }
    
    return bAllAssetsValid;
}

void UAnim_BlendSpaceManager::CalculateBlendSpaceValues(float Speed, FVector2D Direction)
{
    // Internal calculation for blend space interpolation
    // This handles the smooth transitions between animation states
    
    // Apply smoothing to direction input to avoid jittery animations
    static FVector2D SmoothedDirection = FVector2D::ZeroVector;
    float SmoothingFactor = 0.1f;
    
    SmoothedDirection = FMath::Lerp(SmoothedDirection, Direction, SmoothingFactor);
    BlendSpaceData.DirectionalInput = SmoothedDirection;
}

bool UAnim_BlendSpaceManager::CanTransitionToState(EAnim_LocomotionState FromState, EAnim_LocomotionState ToState)
{
    // Define valid state transitions
    // This prevents jarring animation changes
    
    switch (FromState)
    {
        case EAnim_LocomotionState::Idle:
            return true; // Can transition to any state from idle
            
        case EAnim_LocomotionState::Walking:
            return (ToState != EAnim_LocomotionState::Sprinting); // Can't go directly from walk to sprint
            
        case EAnim_LocomotionState::Running:
            return true; // Running can transition to any state
            
        case EAnim_LocomotionState::Sprinting:
            return (ToState != EAnim_LocomotionState::Walking); // Can't go directly from sprint to walk
            
        case EAnim_LocomotionState::Jumping:
            return (ToState == EAnim_LocomotionState::Falling || ToState == EAnim_LocomotionState::Landing);
            
        case EAnim_LocomotionState::Falling:
            return (ToState == EAnim_LocomotionState::Landing || ToState == EAnim_LocomotionState::Idle);
            
        case EAnim_LocomotionState::Landing:
            return (ToState == EAnim_LocomotionState::Idle || ToState == EAnim_LocomotionState::Walking);
            
        case EAnim_LocomotionState::Crouching:
            return true; // Can transition to any state from crouching
            
        default:
            return true;
    }
}