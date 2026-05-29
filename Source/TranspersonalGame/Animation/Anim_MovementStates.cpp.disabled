#include "Anim_MovementStates.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_MovementStates::UAnim_MovementStates()
{
    WalkSpeedThreshold = 200.0f;
    RunSpeedThreshold = 400.0f;
    LeanInterpSpeed = 5.0f;
    MaxLeanAngle = 15.0f;
}

FAnim_MovementData UAnim_MovementStates::CalculateMovementData(ACharacter* Character)
{
    FAnim_MovementData MovementData;
    
    if (!Character || !Character->GetCharacterMovement())
    {
        return MovementData;
    }

    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    FVector Velocity = Character->GetVelocity();
    FRotator ActorRotation = Character->GetActorRotation();
    
    // Calculate basic movement values
    MovementData.Speed = Velocity.Size();
    MovementData.Direction = CalculateDirection(Velocity, ActorRotation);
    MovementData.bIsInAir = MovementComp->IsFalling();
    MovementData.bIsCrouching = MovementComp->IsCrouching();
    
    // Determine movement state
    MovementData.MovementState = GetMovementStateFromVelocity(Velocity, MovementData.bIsInAir, MovementData.bIsCrouching);
    
    // Calculate lean for turning animations
    MovementData.Lean = CalculateLean(Velocity, ActorRotation, 0.016f); // Assume 60fps
    
    // Calculate aim pitch from control rotation
    if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
    {
        MovementData.AimPitch = PC->GetControlRotation().Pitch;
    }
    
    return MovementData;
}

EAnim_MovementState UAnim_MovementStates::GetMovementStateFromVelocity(const FVector& Velocity, bool bIsInAir, bool bIsCrouching)
{
    float Speed = Velocity.Size();
    
    if (bIsInAir)
    {
        return Velocity.Z > 0 ? EAnim_MovementState::Jumping : EAnim_MovementState::Falling;
    }
    
    if (bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    if (Speed < 10.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (Speed < 200.0f)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

float UAnim_MovementStates::CalculateDirection(const FVector& Velocity, const FRotator& ActorRotation)
{
    if (Velocity.SizeSquared() < 1.0f)
    {
        return 0.0f;
    }
    
    FVector ForwardVector = ActorRotation.Vector();
    FVector VelocityNormalized = Velocity.GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
    
    return UKismetMathLibrary::DegAtan2(CrossProduct, DotProduct);
}

float UAnim_MovementStates::CalculateLean(const FVector& Velocity, const FRotator& ActorRotation, float DeltaTime)
{
    if (Velocity.SizeSquared() < 100.0f) // Not moving fast enough to lean
    {
        return 0.0f;
    }
    
    FVector RightVector = ActorRotation.RotateVector(FVector::RightVector);
    FVector VelocityNormalized = Velocity.GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(RightVector, VelocityNormalized);
    float TargetLean = DotProduct * 15.0f; // Max 15 degrees lean
    
    // In a real implementation, this would be interpolated over time
    return FMath::Clamp(TargetLean, -15.0f, 15.0f);
}