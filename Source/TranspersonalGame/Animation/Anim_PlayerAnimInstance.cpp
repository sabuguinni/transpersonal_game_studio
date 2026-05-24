#include "Anim_PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_PlayerAnimInstance::UAnim_PlayerAnimInstance()
{
    // Set default values
    WalkThreshold = 50.0f;
    RunThreshold = 300.0f;
    JumpThreshold = 100.0f;
    
    bWasInAirLastFrame = false;
    TimeInCurrentState = 0.0f;
    PreviousState = EAnim_LocomotionState::Idle;
    
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnim_PlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character references
    UpdateCharacterReferences();
    
    // Initialize movement data
    MovementData = FAnim_MovementData();
    
    UE_LOG(LogTemp, Log, TEXT("UAnim_PlayerAnimInstance: Initialized for character: %s"), 
           OwningCharacter ? *OwningCharacter->GetName() : TEXT("None"));
}

void UAnim_PlayerAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    // Update character references if needed
    if (!OwningCharacter || !MovementComponent)
    {
        UpdateCharacterReferences();
        if (!OwningCharacter || !MovementComponent)
        {
            return; // Cannot update without valid references
        }
    }
    
    // Update movement data
    UpdateMovementData(DeltaTimeX);
    
    // Update locomotion state
    UpdateLocomotionState();
    
    // Update state timing
    TimeInCurrentState += DeltaTimeX;
}

void UAnim_PlayerAnimInstance::UpdateCharacterReferences()
{
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
    }
}

void UAnim_PlayerAnimInstance::UpdateMovementData(float DeltaTime)
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size();
    
    // Calculate movement direction relative to character forward
    if (MovementData.Speed > 1.0f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        float DotProduct = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
        
        // Determine if moving left or right
        FVector RightVector = OwningCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        if (RightDot < 0.0f)
        {
            MovementData.Direction *= -1.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update air state
    bWasInAirLastFrame = MovementData.bIsInAir;
    MovementData.bIsInAir = MovementComponent->IsFalling();
    
    // Update crouch state
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate jump height (Z velocity when in air)
    if (MovementData.bIsInAir)
    {
        MovementData.JumpHeight = Velocity.Z;
    }
    else
    {
        MovementData.JumpHeight = 0.0f;
    }
}

void UAnim_PlayerAnimInstance::UpdateLocomotionState()
{
    EAnim_LocomotionState NewState = MovementData.LocomotionState;
    
    // State transition logic
    if (ShouldTransitionToJumping())
    {
        NewState = EAnim_LocomotionState::Jumping;
    }
    else if (ShouldTransitionToFalling())
    {
        NewState = EAnim_LocomotionState::Falling;
    }
    else if (ShouldTransitionToLanding())
    {
        NewState = EAnim_LocomotionState::Landing;
    }
    else if (MovementData.bIsCrouching)
    {
        NewState = EAnim_LocomotionState::Crouching;
    }
    else if (ShouldTransitionToRunning())
    {
        NewState = EAnim_LocomotionState::Running;
    }
    else if (ShouldTransitionToWalking())
    {
        NewState = EAnim_LocomotionState::Walking;
    }
    else if (ShouldTransitionToIdle())
    {
        NewState = EAnim_LocomotionState::Idle;
    }
    
    // Update state if changed
    if (NewState != MovementData.LocomotionState)
    {
        PreviousState = MovementData.LocomotionState;
        MovementData.LocomotionState = NewState;
        TimeInCurrentState = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("UAnim_PlayerAnimInstance: State transition from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

bool UAnim_PlayerAnimInstance::ShouldTransitionToIdle() const
{
    return !MovementData.bIsInAir && 
           !MovementData.bIsCrouching && 
           MovementData.Speed < WalkThreshold;
}

bool UAnim_PlayerAnimInstance::ShouldTransitionToWalking() const
{
    return !MovementData.bIsInAir && 
           !MovementData.bIsCrouching && 
           MovementData.Speed >= WalkThreshold && 
           MovementData.Speed < RunThreshold;
}

bool UAnim_PlayerAnimInstance::ShouldTransitionToRunning() const
{
    return !MovementData.bIsInAir && 
           !MovementData.bIsCrouching && 
           MovementData.Speed >= RunThreshold;
}

bool UAnim_PlayerAnimInstance::ShouldTransitionToJumping() const
{
    return MovementData.bIsInAir && 
           !bWasInAirLastFrame && 
           MovementData.JumpHeight > JumpThreshold;
}

bool UAnim_PlayerAnimInstance::ShouldTransitionToFalling() const
{
    return MovementData.bIsInAir && 
           (MovementData.JumpHeight <= 0.0f || 
            MovementData.LocomotionState == EAnim_LocomotionState::Jumping);
}

bool UAnim_PlayerAnimInstance::ShouldTransitionToLanding() const
{
    return !MovementData.bIsInAir && 
           bWasInAirLastFrame && 
           (PreviousState == EAnim_LocomotionState::Falling || 
            PreviousState == EAnim_LocomotionState::Jumping);
}

void UAnim_PlayerAnimInstance::ForceLocomotionState(EAnim_LocomotionState NewState)
{
    if (NewState != MovementData.LocomotionState)
    {
        PreviousState = MovementData.LocomotionState;
        MovementData.LocomotionState = NewState;
        TimeInCurrentState = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("UAnim_PlayerAnimInstance: Forced state transition to %d"), (int32)NewState);
    }
}

void UAnim_PlayerAnimInstance::ResetAnimationState()
{
    MovementData = FAnim_MovementData();
    PreviousState = EAnim_LocomotionState::Idle;
    TimeInCurrentState = 0.0f;
    bWasInAirLastFrame = false;
    
    UE_LOG(LogTemp, Log, TEXT("UAnim_PlayerAnimInstance: Animation state reset"));
}