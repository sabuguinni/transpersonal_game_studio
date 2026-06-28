// DinosaurAnimInstance.cpp
// Animation Agent #10 — Cycle 005
// Full implementation of UAnimInstance for all dinosaur species.
// Drives locomotion blend spaces, attack montages, and foot IK.

#include "DinosaurAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    // Locomotion defaults
    GroundSpeed        = 0.f;
    Direction          = 0.f;
    bIsInAir           = false;
    bIsAttacking       = false;
    bIsRoaring         = false;
    bIsEating          = false;
    bIsDead            = false;
    bIsAggressive      = false;

    // IK defaults
    LeftFootIKOffset   = FVector::ZeroVector;
    RightFootIKOffset  = FVector::ZeroVector;
    LeftFootRotation   = FRotator::ZeroRotator;
    RightFootRotation  = FRotator::ZeroRotator;
    IKTraceDistance    = 80.f;
    IKInterpSpeed      = 15.f;

    // State defaults
    LocomotionState    = EAnim_DinoLocomotionState::Idle;
    BodySize           = EAnim_DinoBodySize::Large;

    // Blend space defaults
    WalkPlayRate       = 1.f;
    RunPlayRate        = 1.f;
    TurnRate           = 0.f;

    // Internal
    CachedCharacter    = nullptr;
    CachedMovement     = nullptr;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    CachedCharacter = Cast<ACharacter>(Pawn);
    if (CachedCharacter)
    {
        CachedMovement = CachedCharacter->GetCharacterMovement();
    }
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!CachedCharacter || !CachedMovement) return;

    // ── Ground Speed ──────────────────────────────────────────────
    FVector Velocity = CachedCharacter->GetVelocity();
    Velocity.Z = 0.f;
    GroundSpeed = Velocity.Size();

    // ── Direction (for strafing blend spaces) ─────────────────────
    Direction = CalculateDirection(CachedCharacter->GetVelocity(),
                                   CachedCharacter->GetActorRotation());

    // ── Air state ─────────────────────────────────────────────────
    bIsInAir = CachedMovement->IsFalling();

    // ── Turn rate (yaw delta per second) ──────────────────────────
    TurnRate = CachedMovement->GetLastUpdateRotation().Yaw - CachedCharacter->GetActorRotation().Yaw;

    // ── Locomotion state machine ───────────────────────────────────
    UpdateLocomotionState();

    // ── Foot IK ───────────────────────────────────────────────────
    UpdateFootIK(DeltaSeconds);

    // ── Play rate scaling by body size ────────────────────────────
    UpdatePlayRates();
}

void UDinosaurAnimInstance::UpdateLocomotionState()
{
    if (bIsDead)
    {
        LocomotionState = EAnim_DinoLocomotionState::Dead;
        return;
    }
    if (bIsAttacking)
    {
        LocomotionState = EAnim_DinoLocomotionState::Attacking;
        return;
    }
    if (bIsRoaring)
    {
        LocomotionState = EAnim_DinoLocomotionState::Roaring;
        return;
    }
    if (bIsEating)
    {
        LocomotionState = EAnim_DinoLocomotionState::Eating;
        return;
    }
    if (bIsInAir)
    {
        LocomotionState = EAnim_DinoLocomotionState::Running;
        return;
    }

    // Speed thresholds vary by body size
    float WalkThreshold  = GetWalkThreshold();
    float TrotThreshold  = GetTrotThreshold();

    if (GroundSpeed < 10.f)
    {
        LocomotionState = EAnim_DinoLocomotionState::Idle;
    }
    else if (GroundSpeed < WalkThreshold)
    {
        LocomotionState = EAnim_DinoLocomotionState::Walking;
    }
    else if (GroundSpeed < TrotThreshold)
    {
        LocomotionState = EAnim_DinoLocomotionState::Trotting;
    }
    else
    {
        LocomotionState = EAnim_DinoLocomotionState::Running;
    }
}

float UDinosaurAnimInstance::GetWalkThreshold() const
{
    switch (BodySize)
    {
        case EAnim_DinoBodySize::Small:   return 150.f;
        case EAnim_DinoBodySize::Medium:  return 200.f;
        case EAnim_DinoBodySize::Large:   return 300.f;
        case EAnim_DinoBodySize::Massive: return 400.f;
        default:                          return 200.f;
    }
}

float UDinosaurAnimInstance::GetTrotThreshold() const
{
    switch (BodySize)
    {
        case EAnim_DinoBodySize::Small:   return 400.f;
        case EAnim_DinoBodySize::Medium:  return 500.f;
        case EAnim_DinoBodySize::Large:   return 600.f;
        case EAnim_DinoBodySize::Massive: return 700.f;
        default:                          return 500.f;
    }
}

void UDinosaurAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!CachedCharacter) return;
    if (bIsInAir || bIsDead) return;

    UWorld* World = CachedCharacter->GetWorld();
    if (!World) return;

    // Left foot IK trace
    FVector LeftStart  = CachedCharacter->GetMesh()->GetSocketLocation(FName("LeftFootSocket"));
    FVector LeftEnd    = LeftStart - FVector(0.f, 0.f, IKTraceDistance);
    FHitResult LeftHit;
    bool bLeftHit = World->LineTraceSingleByChannel(LeftHit, LeftStart, LeftEnd,
                                                     ECC_Visibility);
    if (bLeftHit)
    {
        FVector TargetLeft = FVector(0.f, 0.f, LeftHit.ImpactPoint.Z - LeftStart.Z);
        LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, TargetLeft,
                                             DeltaSeconds, IKInterpSpeed);
        FRotator SlopeLeft = UKismetMathLibrary::MakeRotFromZX(LeftHit.ImpactNormal,
                                                                 CachedCharacter->GetActorForwardVector());
        LeftFootRotation = FMath::RInterpTo(LeftFootRotation, SlopeLeft,
                                             DeltaSeconds, IKInterpSpeed);
    }
    else
    {
        LeftFootIKOffset  = FMath::VInterpTo(LeftFootIKOffset,  FVector::ZeroVector, DeltaSeconds, IKInterpSpeed);
        LeftFootRotation  = FMath::RInterpTo(LeftFootRotation,  FRotator::ZeroRotator, DeltaSeconds, IKInterpSpeed);
    }

    // Right foot IK trace
    FVector RightStart = CachedCharacter->GetMesh()->GetSocketLocation(FName("RightFootSocket"));
    FVector RightEnd   = RightStart - FVector(0.f, 0.f, IKTraceDistance);
    FHitResult RightHit;
    bool bRightHit = World->LineTraceSingleByChannel(RightHit, RightStart, RightEnd,
                                                      ECC_Visibility);
    if (bRightHit)
    {
        FVector TargetRight = FVector(0.f, 0.f, RightHit.ImpactPoint.Z - RightStart.Z);
        RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, TargetRight,
                                              DeltaSeconds, IKInterpSpeed);
        FRotator SlopeRight = UKismetMathLibrary::MakeRotFromZX(RightHit.ImpactNormal,
                                                                  CachedCharacter->GetActorForwardVector());
        RightFootRotation = FMath::RInterpTo(RightFootRotation, SlopeRight,
                                              DeltaSeconds, IKInterpSpeed);
    }
    else
    {
        RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, FVector::ZeroVector, DeltaSeconds, IKInterpSpeed);
        RightFootRotation = FMath::RInterpTo(RightFootRotation, FRotator::ZeroRotator, DeltaSeconds, IKInterpSpeed);
    }
}

void UDinosaurAnimInstance::UpdatePlayRates()
{
    // Larger dinosaurs animate slower — scale play rate inversely with size
    float SpeedRatio = (GroundSpeed > 0.f) ? FMath::Clamp(GroundSpeed / GetTrotThreshold(), 0.5f, 2.f) : 1.f;

    float SizeScale = 1.f;
    switch (BodySize)
    {
        case EAnim_DinoBodySize::Small:   SizeScale = 1.3f; break;
        case EAnim_DinoBodySize::Medium:  SizeScale = 1.1f; break;
        case EAnim_DinoBodySize::Large:   SizeScale = 0.9f; break;
        case EAnim_DinoBodySize::Massive: SizeScale = 0.7f; break;
        default:                          SizeScale = 1.0f; break;
    }

    WalkPlayRate = SizeScale;
    RunPlayRate  = SizeScale * SpeedRatio;
}

void UDinosaurAnimInstance::SetAttacking(bool bAttacking)
{
    bIsAttacking = bAttacking;
}

void UDinosaurAnimInstance::SetRoaring(bool bRoaring)
{
    bIsRoaring = bRoaring;
}

void UDinosaurAnimInstance::SetEating(bool bEating)
{
    bIsEating = bEating;
}

void UDinosaurAnimInstance::SetDead(bool bDead)
{
    bIsDead = bDead;
}

void UDinosaurAnimInstance::SetAggressive(bool bAggressive)
{
    bIsAggressive = bAggressive;
}

void UDinosaurAnimInstance::SetBodySize(EAnim_DinoBodySize NewSize)
{
    BodySize = NewSize;
}
