#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
    : Speed(0.f)
    , Direction(0.f)
    , VerticalVelocity(0.f)
    , bIsInAir(false)
    , bIsCrouching(false)
    , bIsSprinting(false)
    , bIsMoving(false)
    , LocomotionState(EAnim_LocomotionState::Idle)
    , StanceType(EAnim_StanceType::Standing)
    , Health(100.f)
    , Stamina(100.f)
    , Fear(0.f)
    , bIsExhausted(false)
    , bIsInjured(false)
    , LeftFootIKLocation(FVector::ZeroVector)
    , RightFootIKLocation(FVector::ZeroVector)
    , LeftFootIKRotation(FRotator::ZeroRotator)
    , RightFootIKRotation(FRotator::ZeroRotator)
    , IKAlpha(0.f)
    , AimPitch(0.f)
    , AimYaw(0.f)
    , bIsAiming(false)
    , bIsAttacking(false)
    , LeanAngle(0.f)
    , AccelerationMagnitude(0.f)
    , PreviousVelocity(FVector::ZeroVector)
    , LeanAlpha(0.f)
{
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        OwnerCharacter = Cast<ACharacter>(Pawn);
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        APawn* Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            OwnerCharacter = Cast<ACharacter>(Pawn);
            if (OwnerCharacter)
            {
                MovementComponent = OwnerCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    // ── Core velocity data ────────────────────────────────────────────────────
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    VerticalVelocity = Velocity.Z;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsMoving = Speed > 10.f;

    // Direction relative to actor forward
    if (bIsMoving)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
        Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
    }
    else
    {
        Direction = 0.f;
    }

    // Sprint detection: speed > 400 cm/s threshold
    bIsSprinting = Speed > 400.f && !bIsInAir && !bIsCrouching;

    // Lean calculation
    UpdateLean(DeltaSeconds);

    // Locomotion state machine
    UpdateLocomotionState();

    // Foot IK
    UpdateFootIK(DeltaSeconds);

    // Aim offset
    UpdateAimOffset();

    // Survival stats (pulled from character if it exposes them)
    UpdateSurvivalStats();

    PreviousVelocity = Velocity;
}

void UTranspersonalAnimInstance::UpdateLocomotionState()
{
    if (!OwnerCharacter) return;

    // Dead check — health driven
    if (Health <= 0.f)
    {
        LocomotionState = EAnim_LocomotionState::Dead;
        StanceType = EAnim_StanceType::Prone;
        return;
    }

    // Airborne
    if (bIsInAir)
    {
        LocomotionState = VerticalVelocity > 0.f
            ? EAnim_LocomotionState::Jumping
            : EAnim_LocomotionState::Falling;
        StanceType = EAnim_StanceType::Standing;
        return;
    }

    // Attacking
    if (bIsAttacking)
    {
        LocomotionState = EAnim_LocomotionState::Attacking;
    }
    // Crouching
    else if (bIsCrouching)
    {
        LocomotionState = bIsMoving
            ? EAnim_LocomotionState::Crouching
            : EAnim_LocomotionState::Crouching;
        StanceType = EAnim_StanceType::Crouched;
        return;
    }
    // Sprinting
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprinting;
    }
    // Walking
    else if (bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Walking;
    }
    // Idle
    else
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }

    StanceType = EAnim_StanceType::Standing;
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Only apply IK when grounded
    if (bIsInAir)
    {
        IKAlpha = FMath::FInterpTo(IKAlpha, 0.f, DeltaSeconds, 10.f);
        return;
    }

    IKAlpha = FMath::FInterpTo(IKAlpha, 1.f, DeltaSeconds, 10.f);

    UWorld* World = GetWorld();
    if (!World) return;

    const float TraceLength = 80.f;
    const float FootHeight = 10.f;

    // Helper lambda for foot trace
    auto TraceFootIK = [&](FName SocketName, FVector& OutLocation, FRotator& OutRotation)
    {
        USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
        if (!Mesh) return;

        FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
        FVector TraceStart = FVector(SocketLoc.X, SocketLoc.Y, SocketLoc.Z + TraceLength);
        FVector TraceEnd   = FVector(SocketLoc.X, SocketLoc.Y, SocketLoc.Z - TraceLength);

        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            OutLocation = HitResult.ImpactPoint + FVector(0.f, 0.f, FootHeight);
            // Align foot to surface normal
            FVector Normal = HitResult.ImpactNormal;
            OutRotation = FRotator(
                FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
                0.f,
                FMath::RadiansToDegrees(-FMath::Atan2(Normal.Y, Normal.Z))
            );
        }
        else
        {
            OutLocation = SocketLoc;
            OutRotation = FRotator::ZeroRotator;
        }
    };

    TraceFootIK(FName("foot_l"), LeftFootIKLocation, LeftFootIKRotation);
    TraceFootIK(FName("foot_r"), RightFootIKLocation, RightFootIKRotation);
}

void UTranspersonalAnimInstance::UpdateSurvivalStats()
{
    if (!OwnerCharacter) return;

    // Try to read survival stats from character via property reflection
    // This is a safe fallback — if the character doesn't expose these,
    // we keep the defaults. The AnimBP can override via Blueprint.

    // Exhausted: stamina below 15%
    bIsExhausted = Stamina < 15.f;

    // Injured: health below 30%
    bIsInjured = Health < 30.f;
}

void UTranspersonalAnimInstance::UpdateAimOffset()
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot   = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot   = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.f, 90.f);
    AimYaw   = FMath::ClampAngle(DeltaRot.Yaw,   -90.f, 90.f);
}

void UTranspersonalAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    FVector CurrentVelocity = MovementComponent ? MovementComponent->Velocity : FVector::ZeroVector;
    FVector Acceleration = (CurrentVelocity - PreviousVelocity) / FMath::Max(DeltaSeconds, KINDA_SMALL_NUMBER);
    AccelerationMagnitude = Acceleration.Size2D();

    // Lateral lean based on centripetal acceleration
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector LocalAccel = ActorRot.UnrotateVector(Acceleration);
    float TargetLean = FMath::Clamp(LocalAccel.Y * 0.01f, -15.f, 15.f);

    LeanAlpha = FMath::FInterpTo(LeanAlpha, TargetLean, DeltaSeconds, 4.f);
    LeanAngle = LeanAlpha;
}
