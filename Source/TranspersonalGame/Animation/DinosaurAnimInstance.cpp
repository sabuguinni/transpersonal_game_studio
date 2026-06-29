// DinosaurAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Dinosaur AnimInstance — drives locomotion, attack, and death states for all dinosaur species

#include "DinosaurAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    // Locomotion defaults
    GroundSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsMoving = false;
    bIsRunning = false;
    bIsInAir = false;
    VerticalVelocity = 0.0f;

    // Combat defaults
    bIsAttacking = false;
    bIsRoaring = false;
    bIsEating = false;
    bIsDead = false;
    bIsStunned = false;

    // Behavioral defaults
    bIsAlerted = false;
    bIsSearching = false;
    bIsSleeping = false;
    bIsIdle = true;

    // Blend weights
    AttackBlendWeight = 0.0f;
    RoarBlendWeight = 0.0f;
    InjuryBlendWeight = 0.0f;
    AlertBlendWeight = 0.0f;

    // Species defaults
    DinosaurSpecies = EAnim_DinosaurSpecies::Raptor;
    HealthPercent = 1.0f;
    StaminaPercent = 1.0f;

    // IK defaults
    bEnableFootIK = true;
    LeftFootIKAlpha = 1.0f;
    RightFootIKAlpha = 1.0f;
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;

    // Tail/head procedural
    TailSwayAngle = 0.0f;
    HeadLookAtAlpha = 0.0f;
    HeadLookAtTarget = FVector::ZeroVector;

    // Cached refs
    OwnerPawn = nullptr;
    OwnerMovement = nullptr;

    // Internal
    TimeSinceLastAttack = 0.0f;
    TimeSinceLastRoar = 0.0f;
    IdleVariantIndex = 0;
    IdleVariantTimer = 0.0f;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerPawn);
        if (OwnerCharacter)
        {
            OwnerMovement = OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        if (!OwnerPawn) return;

        ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerPawn);
        if (OwnerCharacter)
        {
            OwnerMovement = OwnerCharacter->GetCharacterMovement();
        }
    }

    UpdateLocomotion(DeltaSeconds);
    UpdateCombatState(DeltaSeconds);
    UpdateBehaviorState(DeltaSeconds);
    UpdateProceduralElements(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
}

void UDinosaurAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    FVector Velocity = OwnerPawn->GetVelocity();
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    GroundSpeed = HorizontalVelocity.Size();
    VerticalVelocity = Velocity.Z;

    bIsMoving = GroundSpeed > 10.0f;
    bIsInAir = (OwnerMovement && OwnerMovement->IsFalling());

    // Determine run threshold by species
    float RunThreshold = GetRunThresholdForSpecies();
    bIsRunning = GroundSpeed > RunThreshold;

    // Movement direction relative to actor forward
    if (bIsMoving)
    {
        FRotator ActorRotation = OwnerPawn->GetActorRotation();
        FVector ForwardVector = ActorRotation.Vector();
        FVector NormalizedVelocity = HorizontalVelocity.GetSafeNormal();

        float DotProduct = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        FVector CrossProduct = FVector::CrossProduct(ForwardVector, NormalizedVelocity);
        float AngleRad = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
        float AngleDeg = FMath::RadiansToDegrees(AngleRad);

        MovementDirection = (CrossProduct.Z < 0.0f) ? -AngleDeg : AngleDeg;
    }
    else
    {
        MovementDirection = FMath::FInterpTo(MovementDirection, 0.0f, DeltaSeconds, 5.0f);
    }
}

void UDinosaurAnimInstance::UpdateCombatState(float DeltaSeconds)
{
    // Decay timers
    TimeSinceLastAttack += DeltaSeconds;
    TimeSinceLastRoar += DeltaSeconds;

    // Blend weight decay
    if (!bIsAttacking)
    {
        AttackBlendWeight = FMath::FInterpTo(AttackBlendWeight, 0.0f, DeltaSeconds, 4.0f);
    }
    if (!bIsRoaring)
    {
        RoarBlendWeight = FMath::FInterpTo(RoarBlendWeight, 0.0f, DeltaSeconds, 3.0f);
    }

    // Injury blend based on health
    float TargetInjuryBlend = 0.0f;
    if (HealthPercent < 0.3f)
    {
        TargetInjuryBlend = 1.0f - (HealthPercent / 0.3f);
    }
    InjuryBlendWeight = FMath::FInterpTo(InjuryBlendWeight, TargetInjuryBlend, DeltaSeconds, 2.0f);

    // Alert blend
    float TargetAlertBlend = bIsAlerted ? 1.0f : 0.0f;
    AlertBlendWeight = FMath::FInterpTo(AlertBlendWeight, TargetAlertBlend, DeltaSeconds, 3.0f);
}

void UDinosaurAnimInstance::UpdateBehaviorState(float DeltaSeconds)
{
    // Idle variant cycling — prevents static idle loop
    if (bIsIdle && !bIsMoving && !bIsAttacking && !bIsDead)
    {
        IdleVariantTimer += DeltaSeconds;
        float VariantDuration = GetIdleVariantDuration();
        if (IdleVariantTimer >= VariantDuration)
        {
            IdleVariantTimer = 0.0f;
            IdleVariantIndex = (IdleVariantIndex + 1) % 3; // 3 idle variants
        }
    }
    else
    {
        IdleVariantTimer = 0.0f;
    }

    // Auto-set idle when not doing anything
    bIsIdle = !bIsMoving && !bIsAttacking && !bIsRoaring && !bIsEating && !bIsDead && !bIsStunned && !bIsSleeping;
}

void UDinosaurAnimInstance::UpdateProceduralElements(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    // Tail sway — oscillates based on movement speed
    float SwayFrequency = bIsMoving ? (GroundSpeed / 100.0f) * 2.0f : 0.5f;
    float SwayAmplitude = bIsMoving ? 15.0f : 5.0f;
    float GameTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    TailSwayAngle = FMath::Sin(GameTime * SwayFrequency) * SwayAmplitude;

    // Head look-at — smooth interpolation toward target
    if (HeadLookAtAlpha > 0.01f && !HeadLookAtTarget.IsZero())
    {
        // Alpha is set externally by AI/behavior system
        // Here we just ensure smooth transitions
    }
}

void UDinosaurAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!bEnableFootIK || !OwnerPawn) return;

    // Foot IK trace — cast rays downward from foot bone positions
    // This adapts foot placement to uneven terrain
    UWorld* World = GetWorld();
    if (!World) return;

    FVector ActorLocation = OwnerPawn->GetActorLocation();
    FVector ActorForward = OwnerPawn->GetActorForwardVector();
    FVector ActorRight = OwnerPawn->GetActorRightVector();

    // Approximate foot positions based on species scale
    float FootSpread = GetFootSpreadForSpecies();
    float FootForwardOffset = GetFootForwardOffsetForSpecies();

    FVector LeftFootWorld = ActorLocation + ActorRight * (-FootSpread) + ActorForward * FootForwardOffset;
    FVector RightFootWorld = ActorLocation + ActorRight * FootSpread + ActorForward * FootForwardOffset;

    // Trace downward
    FHitResult LeftHit, RightHit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);

    float TraceDistance = 100.0f;
    bool bLeftHit = World->LineTraceSingleByChannel(
        LeftHit,
        LeftFootWorld + FVector(0, 0, 50),
        LeftFootWorld - FVector(0, 0, TraceDistance),
        ECC_WorldStatic,
        QueryParams
    );

    bool bRightHit = World->LineTraceSingleByChannel(
        RightHit,
        RightFootWorld + FVector(0, 0, 50),
        RightFootWorld - FVector(0, 0, TraceDistance),
        ECC_WorldStatic,
        QueryParams
    );

    // Smooth IK offsets
    FVector TargetLeftOffset = bLeftHit ? FVector(0, 0, LeftHit.ImpactPoint.Z - ActorLocation.Z) : FVector::ZeroVector;
    FVector TargetRightOffset = bRightHit ? FVector(0, 0, RightHit.ImpactPoint.Z - ActorLocation.Z) : FVector::ZeroVector;

    LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, TargetLeftOffset, DeltaSeconds, 10.0f);
    RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, TargetRightOffset, DeltaSeconds, 10.0f);

    // Alpha — reduce IK when in air
    float TargetAlpha = bIsInAir ? 0.0f : 1.0f;
    LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, TargetAlpha, DeltaSeconds, 5.0f);
    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, TargetAlpha, DeltaSeconds, 5.0f);
}

void UDinosaurAnimInstance::TriggerAttack(EAnim_DinosaurAttackType AttackType)
{
    if (bIsDead || bIsStunned) return;

    bIsAttacking = true;
    AttackBlendWeight = 1.0f;
    TimeSinceLastAttack = 0.0f;
    CurrentAttackType = AttackType;
}

void UDinosaurAnimInstance::TriggerRoar()
{
    if (bIsDead || bIsStunned) return;

    bIsRoaring = true;
    RoarBlendWeight = 1.0f;
    TimeSinceLastRoar = 0.0f;
}

void UDinosaurAnimInstance::SetAlertState(bool bAlerted, FVector ThreatLocation)
{
    bIsAlerted = bAlerted;
    if (bAlerted)
    {
        HeadLookAtTarget = ThreatLocation;
        HeadLookAtAlpha = 1.0f;
    }
    else
    {
        HeadLookAtAlpha = FMath::FInterpTo(HeadLookAtAlpha, 0.0f, 0.016f, 2.0f);
    }
}

void UDinosaurAnimInstance::SetHealthPercent(float NewHealthPercent)
{
    HealthPercent = FMath::Clamp(NewHealthPercent, 0.0f, 1.0f);
    if (HealthPercent <= 0.0f)
    {
        bIsDead = true;
        bIsAttacking = false;
        bIsRoaring = false;
    }
}

void UDinosaurAnimInstance::SetStaminaPercent(float NewStaminaPercent)
{
    StaminaPercent = FMath::Clamp(NewStaminaPercent, 0.0f, 1.0f);
}

float UDinosaurAnimInstance::GetRunThresholdForSpecies() const
{
    switch (DinosaurSpecies)
    {
        case EAnim_DinosaurSpecies::TRex:           return 300.0f;
        case EAnim_DinosaurSpecies::Raptor:         return 200.0f;
        case EAnim_DinosaurSpecies::Triceratops:    return 250.0f;
        case EAnim_DinosaurSpecies::Brachiosaurus:  return 150.0f;
        case EAnim_DinosaurSpecies::Pterodactyl:    return 400.0f;
        case EAnim_DinosaurSpecies::Stegosaurus:    return 180.0f;
        case EAnim_DinosaurSpecies::Ankylosaurus:   return 160.0f;
        default:                                    return 200.0f;
    }
}

float UDinosaurAnimInstance::GetFootSpreadForSpecies() const
{
    switch (DinosaurSpecies)
    {
        case EAnim_DinosaurSpecies::TRex:           return 60.0f;
        case EAnim_DinosaurSpecies::Raptor:         return 25.0f;
        case EAnim_DinosaurSpecies::Triceratops:    return 70.0f;
        case EAnim_DinosaurSpecies::Brachiosaurus:  return 100.0f;
        case EAnim_DinosaurSpecies::Stegosaurus:    return 65.0f;
        case EAnim_DinosaurSpecies::Ankylosaurus:   return 75.0f;
        default:                                    return 40.0f;
    }
}

float UDinosaurAnimInstance::GetFootForwardOffsetForSpecies() const
{
    switch (DinosaurSpecies)
    {
        case EAnim_DinosaurSpecies::TRex:           return 80.0f;
        case EAnim_DinosaurSpecies::Raptor:         return 30.0f;
        case EAnim_DinosaurSpecies::Triceratops:    return 90.0f;
        case EAnim_DinosaurSpecies::Brachiosaurus:  return 120.0f;
        default:                                    return 50.0f;
    }
}

float UDinosaurAnimInstance::GetIdleVariantDuration() const
{
    switch (DinosaurSpecies)
    {
        case EAnim_DinosaurSpecies::TRex:           return 8.0f;
        case EAnim_DinosaurSpecies::Raptor:         return 4.0f;
        case EAnim_DinosaurSpecies::Brachiosaurus:  return 12.0f;
        default:                                    return 6.0f;
    }
}
