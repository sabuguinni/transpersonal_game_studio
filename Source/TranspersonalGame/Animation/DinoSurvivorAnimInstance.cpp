// DinoSurvivorAnimInstance.cpp
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260622_013
// Animation instance for the dinosaur survivor player character

#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsAiming = false;
    bIsDead = false;
    LeanAngle = 0.0f;
    AimPitch = 0.0f;
    AimYaw = 0.0f;
    FearLevel = 0.0f;
    StaminaLevel = 1.0f;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
        return;
    }

    // Velocity-based speed
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();

    // Direction relative to character facing
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector VelNorm = Velocity.GetSafeNormal2D();
    if (!VelNorm.IsNearlyZero())
    {
        FRotator VelRot = VelNorm.Rotation();
        FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot);
        Direction = Delta.Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // Air state
    bIsInAir = MovementComponent->IsFalling();

    // Crouch state
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Sprint: speed > walk threshold (300 = walk, 600 = sprint)
    bIsSprinting = (Speed > 350.0f) && !bIsInAir;

    // Lean angle — smoothed lateral lean based on direction
    float TargetLean = FMath::Clamp(Direction * 0.15f, -15.0f, 15.0f);
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 5.0f);

    // Aim pitch/yaw from controller rotation
    AController* Controller = OwnerCharacter->GetController();
    if (Controller)
    {
        FRotator ControlRot = Controller->GetControlRotation();
        FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        FRotator AimDelta = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRotation);
        AimPitch = FMath::ClampAngle(AimDelta.Pitch, -90.0f, 90.0f);
        AimYaw = FMath::ClampAngle(AimDelta.Yaw, -180.0f, 180.0f);
    }

    // Fear level affects animation blend (trembling, hunched posture)
    // Interpolate toward current fear — actual value set externally by SurvivalComponent
    FearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);

    // Stamina affects movement blend
    StaminaLevel = FMath::Clamp(StaminaLevel, 0.0f, 1.0f);
    if (StaminaLevel < 0.2f && bIsSprinting)
    {
        // Force walk blend when exhausted
        bIsSprinting = false;
    }
}

void UDinoSurvivorAnimInstance::SetFearLevel(float NewFear)
{
    FearLevel = FMath::Clamp(NewFear, 0.0f, 1.0f);
}

void UDinoSurvivorAnimInstance::SetStaminaLevel(float NewStamina)
{
    StaminaLevel = FMath::Clamp(NewStamina, 0.0f, 1.0f);
}

void UDinoSurvivorAnimInstance::SetIsDead(bool bDead)
{
    bIsDead = bDead;
}

void UDinoSurvivorAnimInstance::SetIsAiming(bool bAiming)
{
    bIsAiming = bAiming;
}
