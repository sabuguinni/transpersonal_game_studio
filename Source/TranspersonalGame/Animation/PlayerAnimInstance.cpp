// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full implementation of the player character animation instance
// Handles locomotion blending, survival states, combat, and foot IK

#include "PlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    SmoothedSpeed = 0.0f;
    Direction = 0.0f;
    bIsMoving = false;
    bIsSprinting = false;
    bIsCrouching = false;
    bIsInAir = false;
    LeanAmount = 0.0f;

    // Survival state defaults
    bIsSneaking = false;
    bIsClimbing = false;
    FearLevel = 0.0f;
    StaminaLevel = 1.0f;

    // Combat defaults
    bIsInCombat = false;
    bIsAttacking = false;
    bIsBlocking = false;
    bIsHurt = false;
    bIsDead = false;
    UpperBodyLayerWeight = 0.0f;

    // Foot IK defaults
    bEnableFootIK = true;
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    LeftFootRotation = FRotator::ZeroRotator;
    RightFootRotation = FRotator::ZeroRotator;
    IKTraceDistance = 60.0f;

    // Internal
    OwnerCharacter = nullptr;
    OwnerMovement = nullptr;
    SpeedSmoothingRate = 8.0f;
    LeanSmoothingRate = 5.0f;
    PreviousVelocity = FVector::ZeroVector;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        OwnerCharacter = Cast<ACharacter>(OwnerPawn);
        if (OwnerCharacter)
        {
            OwnerMovement = OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !OwnerMovement)
    {
        // Try to re-acquire owner references
        APawn* OwnerPawn = TryGetPawnOwner();
        if (OwnerPawn)
        {
            OwnerCharacter = Cast<ACharacter>(OwnerPawn);
            if (OwnerCharacter)
            {
                OwnerMovement = OwnerCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    // --- LOCOMOTION UPDATE ---
    UpdateLocomotionState(DeltaSeconds);

    // --- SURVIVAL STATE UPDATE ---
    UpdateSurvivalState(DeltaSeconds);

    // --- FOOT IK UPDATE ---
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaSeconds);
    }

    // --- LEAN UPDATE ---
    UpdateLean(DeltaSeconds);
}

void UPlayerAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    if (!OwnerCharacter || !OwnerMovement)
        return;

    FVector Velocity = OwnerMovement->Velocity;
    float RawSpeed = Velocity.Size2D(); // horizontal speed only

    // Smooth the speed for blend space transitions
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, RawSpeed, DeltaSeconds, SpeedSmoothingRate);
    Speed = SmoothedSpeed;

    // Moving threshold: 10 cm/s
    bIsMoving = RawSpeed > 10.0f;

    // In air
    bIsInAir = OwnerMovement->IsFalling();

    // Crouching
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Direction (for strafing blend spaces)
    if (bIsMoving)
    {
        FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        FVector VelocityNorm = Velocity.GetSafeNormal2D();
        FVector ForwardVector = ActorRotation.Vector();

        // Calculate direction angle (-180 to 180)
        Direction = UKismetMathLibrary::DegAtan2(
            FVector::DotProduct(VelocityNorm, FVector::CrossProduct(FVector::UpVector, ForwardVector)),
            FVector::DotProduct(VelocityNorm, ForwardVector)
        );
    }
    else
    {
        Direction = FMath::FInterpTo(Direction, 0.0f, DeltaSeconds, 5.0f);
    }

    // Sprint detection: sprinting if speed > 400 cm/s (walk ~200, run ~400, sprint ~600+)
    bIsSprinting = RawSpeed > 420.0f && !bIsCrouching && !bIsInAir;

    // Sneak: crouching while moving slowly
    bIsSneaking = bIsCrouching && bIsMoving && RawSpeed < 180.0f;
}

void UPlayerAnimInstance::UpdateSurvivalState(float DeltaSeconds)
{
    // FearLevel and StaminaLevel are driven externally by the survival system
    // Here we just clamp them to valid ranges
    FearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    StaminaLevel = FMath::Clamp(StaminaLevel, 0.0f, 1.0f);

    // Upper body layer weight: blend in combat layer when in combat
    float TargetUpperBodyWeight = (bIsInCombat || bIsAttacking) ? 1.0f : 0.0f;
    UpperBodyLayerWeight = FMath::FInterpTo(UpperBodyLayerWeight, TargetUpperBodyWeight, DeltaSeconds, 6.0f);
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter)
        return;

    // Don't do foot IK in air
    if (bIsInAir)
    {
        LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, FVector::ZeroVector, DeltaSeconds, 10.0f);
        RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, FVector::ZeroVector, DeltaSeconds, 10.0f);
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
        return;

    // Bone socket names for foot traces
    const FName LeftFootSocket = TEXT("foot_l");
    const FName RightFootSocket = TEXT("foot_r");

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
        return;

    // Trace for left foot
    FVector LeftFootLocation = MeshComp->GetSocketLocation(LeftFootSocket);
    FVector LeftTraceStart = LeftFootLocation + FVector(0, 0, IKTraceDistance);
    FVector LeftTraceEnd = LeftFootLocation - FVector(0, 0, IKTraceDistance);

    FHitResult LeftHit;
    FCollisionQueryParams LeftParams;
    LeftParams.AddIgnoredActor(OwnerCharacter);

    if (World->LineTraceSingleByChannel(LeftHit, LeftTraceStart, LeftTraceEnd, ECC_Visibility, LeftParams))
    {
        FVector TargetLeftOffset = FVector(0, 0, LeftHit.Location.Z - LeftFootLocation.Z);
        LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, TargetLeftOffset, DeltaSeconds, 12.0f);

        // Foot rotation from surface normal
        FVector Normal = LeftHit.Normal;
        LeftFootRotation = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
            0.0f,
            FMath::RadiansToDegrees(FMath::Atan2(-Normal.Y, Normal.Z))
        );
    }
    else
    {
        LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, FVector::ZeroVector, DeltaSeconds, 12.0f);
        LeftFootRotation = FMath::RInterpTo(LeftFootRotation, FRotator::ZeroRotator, DeltaSeconds, 12.0f);
    }

    // Trace for right foot
    FVector RightFootLocation = MeshComp->GetSocketLocation(RightFootSocket);
    FVector RightTraceStart = RightFootLocation + FVector(0, 0, IKTraceDistance);
    FVector RightTraceEnd = RightFootLocation - FVector(0, 0, IKTraceDistance);

    FHitResult RightHit;
    FCollisionQueryParams RightParams;
    RightParams.AddIgnoredActor(OwnerCharacter);

    if (World->LineTraceSingleByChannel(RightHit, RightTraceStart, RightTraceEnd, ECC_Visibility, RightParams))
    {
        FVector TargetRightOffset = FVector(0, 0, RightHit.Location.Z - RightFootLocation.Z);
        RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, TargetRightOffset, DeltaSeconds, 12.0f);

        FVector Normal = RightHit.Normal;
        RightFootRotation = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
            0.0f,
            FMath::RadiansToDegrees(FMath::Atan2(-Normal.Y, Normal.Z))
        );
    }
    else
    {
        RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, FVector::ZeroVector, DeltaSeconds, 12.0f);
        RightFootRotation = FMath::RInterpTo(RightFootRotation, FRotator::ZeroRotator, DeltaSeconds, 12.0f);
    }
}

void UPlayerAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter || !OwnerMovement)
        return;

    FVector CurrentVelocity = OwnerMovement->Velocity;
    FVector Acceleration = (CurrentVelocity - PreviousVelocity) / FMath::Max(DeltaSeconds, 0.001f);
    PreviousVelocity = CurrentVelocity;

    // Project lateral acceleration onto character right vector for lean
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    float LateralAccel = FVector::DotProduct(Acceleration, RightVector);

    // Normalize to -1..1 range (max lean at ~600 cm/s² lateral accel)
    float TargetLean = FMath::Clamp(LateralAccel / 600.0f, -1.0f, 1.0f);
    LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, LeanSmoothingRate);
}

void UPlayerAnimInstance::SetFearLevel(float NewFearLevel)
{
    FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
}

void UPlayerAnimInstance::SetStaminaLevel(float NewStaminaLevel)
{
    StaminaLevel = FMath::Clamp(NewStaminaLevel, 0.0f, 1.0f);
}

void UPlayerAnimInstance::SetCombatState(bool bInCombat, bool bAttacking, bool bBlocking)
{
    bIsInCombat = bInCombat;
    bIsAttacking = bAttacking;
    bIsBlocking = bBlocking;
}

void UPlayerAnimInstance::TriggerHurt()
{
    bIsHurt = true;
    // The AnimBP montage will reset this via notify
    // We set a timer-based reset as fallback
    GetWorld()->GetTimerManager().SetTimer(
        HurtResetTimer,
        [this]() { bIsHurt = false; },
        0.5f,
        false
    );
}

void UPlayerAnimInstance::TriggerDeath()
{
    bIsDead = true;
    bIsInCombat = false;
    bIsAttacking = false;
    bIsBlocking = false;
}
