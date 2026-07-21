#include "Anim_MovementController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_MovementController::UAnim_MovementController()
{
    OwnerCharacter = nullptr;
    CharacterMovement = nullptr;
    
    // Set default animation thresholds
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    IdleThreshold = 10.0f;
    
    // Initialize blend parameters
    IdleToWalkBlend = 0.0f;
    WalkToRunBlend = 0.0f;
    DirectionalBlend = 0.0f;
}

void UAnim_MovementController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwnerCharacter = Cast<ATranspersonalCharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        CharacterMovement = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Animation Controller initialized for character: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Controller failed to get character reference"));
    }
}

void UAnim_MovementController::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    UpdateMovementData(DeltaTime);
    UpdateMovementState();
    UpdateBlendParameters(DeltaTime);
}

void UAnim_MovementController::UpdateMovementData(float DeltaTime)
{
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get velocity and speed
    MovementData.Velocity = CharacterMovement->Velocity;
    MovementData.Speed = MovementData.Velocity.Size();
    MovementData.GroundSpeed = MovementData.Velocity.Size2D();
    
    // Calculate movement direction relative to character forward
    if (MovementData.GroundSpeed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = MovementData.Velocity.GetSafeNormal2D();
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityDirection)));
        
        // Determine if moving left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityDirection);
        if (RightDot < 0.0f)
        {
            MovementData.Direction *= -1.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update movement flags
    MovementData.bIsInAir = CharacterMovement->IsFalling();
    MovementData.bIsCrouching = CharacterMovement->IsCrouching();
    MovementData.bIsAccelerating = CharacterMovement->GetCurrentAcceleration().SizeSquared() > 0.0f;
    
    // Calculate jump height if in air
    if (MovementData.bIsInAir)
    {
        MovementData.JumpHeight = MovementData.Velocity.Z;
    }
    else
    {
        MovementData.JumpHeight = 0.0f;
    }
}

void UAnim_MovementController::UpdateMovementState()
{
    MovementData.MovementState = DetermineMovementState();
}

EAnim_MovementState UAnim_MovementController::DetermineMovementState() const
{
    if (!OwnerCharacter || !CharacterMovement)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check if in air first
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check if crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check if swimming
    if (CharacterMovement->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Check ground movement
    if (MovementData.GroundSpeed <= IdleThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (MovementData.GroundSpeed <= RunSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

void UAnim_MovementController::UpdateBlendParameters(float DeltaTime)
{
    // Smooth interpolation speed
    const float InterpSpeed = 5.0f;
    
    // Calculate idle to walk blend
    float TargetIdleToWalk = 0.0f;
    if (MovementData.GroundSpeed > IdleThreshold)
    {
        TargetIdleToWalk = FMath::Clamp((MovementData.GroundSpeed - IdleThreshold) / (WalkSpeedThreshold - IdleThreshold), 0.0f, 1.0f);
    }
    IdleToWalkBlend = FMath::FInterpTo(IdleToWalkBlend, TargetIdleToWalk, DeltaTime, InterpSpeed);
    
    // Calculate walk to run blend
    float TargetWalkToRun = 0.0f;
    if (MovementData.GroundSpeed > WalkSpeedThreshold)
    {
        TargetWalkToRun = FMath::Clamp((MovementData.GroundSpeed - WalkSpeedThreshold) / (RunSpeedThreshold - WalkSpeedThreshold), 0.0f, 1.0f);
    }
    WalkToRunBlend = FMath::FInterpTo(WalkToRunBlend, TargetWalkToRun, DeltaTime, InterpSpeed);
    
    // Calculate directional blend (-1 to 1, where -1 is left, 1 is right)
    float TargetDirectional = MovementData.Direction / 180.0f; // Normalize to -1 to 1 range
    TargetDirectional = FMath::Clamp(TargetDirectional, -1.0f, 1.0f);
    DirectionalBlend = FMath::FInterpTo(DirectionalBlend, TargetDirectional, DeltaTime, InterpSpeed * 2.0f);
}

void UAnim_MovementController::ForceUpdateAnimation()
{
    if (OwnerCharacter && CharacterMovement)
    {
        UpdateMovementData(0.016f); // Assume 60fps for immediate update
        UpdateMovementState();
        UpdateBlendParameters(0.016f);
        
        UE_LOG(LogTemp, Log, TEXT("Animation forced update - State: %d, Speed: %f"), 
               (int32)MovementData.MovementState, MovementData.Speed);
    }
}