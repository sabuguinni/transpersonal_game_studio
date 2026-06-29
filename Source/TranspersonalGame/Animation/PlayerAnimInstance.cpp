// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Implements locomotion blend logic for the prehistoric human player character

#include "PlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    Speed = 0.0f;
    SmoothedSpeed = 0.0f;
    Direction = 0.0f;
    bIsMoving = false;
    bIsSprinting = false;
    bIsCrouching = false;
    bIsInAir = false;
    bIsSneaking = false;
    bIsClimbing = false;
    FearLevel = 0.0f;
    StaminaLevel = 1.0f;
    bIsAttacking = false;
    bIsBlocking = false;
    bIsHurt = false;
    bIsDead = false;
    AimPitch = 0.0f;
    AimYaw = 0.0f;
    LeanAmount = 0.0f;
    LeftFootIKAlpha = 1.0f;
    RightFootIKAlpha = 1.0f;
    LeftFootEffectorLocation = FVector::ZeroVector;
    RightFootEffectorLocation = FVector::ZeroVector;
    MovementState = EAnim_PlayerMovementState::Idle;
    CombatState = EAnim_PlayerCombatState::Unarmed;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
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

    UpdateLocomotionData(DeltaSeconds);
    UpdateAimData();
    UpdateFootIK();
    UpdateMovementState();
    UpdateCombatState();
}

void UPlayerAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    if (!MovementComponent) return;

    // Raw speed from velocity magnitude
    const FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size();

    // Smooth speed for blend space transitions
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaSeconds, 8.0f);

    // Direction relative to character facing
    if (OwnerCharacter && Speed > 1.0f)
    {
        const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        const FVector VelocityNorm = Velocity.GetSafeNormal();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(
            VelocityNorm.Rotation(), ActorRotation).Yaw;
    }
    else
    {
        Direction = FMath::FInterpTo(Direction, 0.0f, DeltaSeconds, 5.0f);
    }

    // Boolean states
    bIsMoving = Speed > 3.0f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Lean: lateral lean based on angular velocity
    const float AngularVelocityYaw = MovementComponent->GetLastUpdateRotation().Yaw;
    LeanAmount = FMath::Clamp(AngularVelocityYaw * 0.05f, -1.0f, 1.0f);
    LeanAmount = FMath::FInterpTo(LeanAmount, 0.0f, DeltaSeconds, 4.0f);
}

void UPlayerAnimInstance::UpdateAimData()
{
    if (!OwnerCharacter) return;

    const AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);

    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -180.0f, 180.0f);
}

void UPlayerAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter) return;

    // Only apply IK when on ground
    if (bIsInAir)
    {
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, GetWorld()->GetDeltaSeconds(), 5.0f);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, GetWorld()->GetDeltaSeconds(), 5.0f);
        return;
    }

    LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 1.0f, GetWorld()->GetDeltaSeconds(), 5.0f);
    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 1.0f, GetWorld()->GetDeltaSeconds(), 5.0f);

    // Trace for left foot
    LeftFootEffectorLocation = CalculateFootIKOffset(TEXT("foot_l"));

    // Trace for right foot
    RightFootEffectorLocation = CalculateFootIKOffset(TEXT("foot_r"));
}

FVector UPlayerAnimInstance::CalculateFootIKOffset(const FName& FootSocketName)
{
    if (!OwnerCharacter) return FVector::ZeroVector;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return FVector::ZeroVector;

    const FVector FootWorldLocation = Mesh->GetSocketLocation(FootSocketName);

    FHitResult HitResult;
    const FVector TraceStart = FootWorldLocation + FVector(0.0f, 0.0f, 50.0f);
    const FVector TraceEnd = FootWorldLocation - FVector(0.0f, 0.0f, 75.0f);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

    if (bHit)
    {
        // Return offset in component space
        const FVector HitOffset = HitResult.Location - FootWorldLocation;
        return FVector(0.0f, 0.0f, HitOffset.Z);
    }

    return FVector::ZeroVector;
}

void UPlayerAnimInstance::UpdateMovementState()
{
    if (bIsDead)
    {
        MovementState = EAnim_PlayerMovementState::Dead;
        return;
    }

    if (bIsClimbing)
    {
        MovementState = EAnim_PlayerMovementState::Climbing;
        return;
    }

    if (bIsInAir)
    {
        MovementState = EAnim_PlayerMovementState::Jumping;
        return;
    }

    if (bIsCrouching || bIsSneaking)
    {
        MovementState = EAnim_PlayerMovementState::Crouching;
        return;
    }

    if (bIsSprinting && bIsMoving)
    {
        MovementState = EAnim_PlayerMovementState::Sprinting;
        return;
    }

    if (bIsMoving)
    {
        MovementState = EAnim_PlayerMovementState::Walking;
        return;
    }

    MovementState = EAnim_PlayerMovementState::Idle;
}

void UPlayerAnimInstance::UpdateCombatState()
{
    if (bIsAttacking)
    {
        // Keep current combat state but flag attacking
        return;
    }
    // Combat state is set externally via SetCombatState
}

void UPlayerAnimInstance::SetSprinting(bool bSprinting)
{
    bIsSprinting = bSprinting;
}

void UPlayerAnimInstance::SetSneaking(bool bSneaking)
{
    bIsSneaking = bSneaking;
}

void UPlayerAnimInstance::SetClimbing(bool bClimbing)
{
    bIsClimbing = bClimbing;
}

void UPlayerAnimInstance::SetFearLevel(float Fear)
{
    FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);
}

void UPlayerAnimInstance::SetStaminaLevel(float Stamina)
{
    StaminaLevel = FMath::Clamp(Stamina, 0.0f, 1.0f);
}

void UPlayerAnimInstance::SetCombatState(EAnim_PlayerCombatState NewState)
{
    CombatState = NewState;
}

void UPlayerAnimInstance::TriggerAttack()
{
    bIsAttacking = true;
}

void UPlayerAnimInstance::TriggerHurt()
{
    bIsHurt = true;
}

void UPlayerAnimInstance::TriggerDeath()
{
    bIsDead = true;
    MovementState = EAnim_PlayerMovementState::Dead;
}
