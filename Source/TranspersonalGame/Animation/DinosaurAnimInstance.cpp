// DinosaurAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Dinosaur animation: bipedal/quadruped locomotion, attack montages, procedural tail

#include "DinosaurAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    // Locomotion
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsBipedal = true;

    // Behavior state
    BehaviorState = EAnim_DinoState::Idle;
    bIsAggressive = false;
    bIsAlert = false;
    bIsFleeing = false;
    bIsEating = false;
    bIsSleeping = false;

    // Attack
    bIsAttacking = false;
    AttackType = EAnim_DinoAttack::None;
    AttackBlendWeight = 0.0f;

    // Procedural tail
    TailSwaySpeed = 1.5f;
    TailSwayAmount = 8.0f;
    TailSwayTime = 0.0f;

    // Head tracking
    bEnableHeadTracking = true;
    HeadTrackingWeight = 0.0f;
    HeadTargetLocation = FVector::ZeroVector;

    // Breath
    BreathCycleSpeed = 0.8f;
    BreathAlpha = 0.0f;
    BreathTime = 0.0f;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        // Try to get movement component
        MovementComponent = Cast<UCharacterMovementComponent>(
            OwnerPawn->GetMovementComponent()
        );
    }
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        return;
    }

    UpdateLocomotion(DeltaSeconds);
    UpdateBehaviorState();
    UpdateProceduralTail(DeltaSeconds);
    UpdateHeadTracking(DeltaSeconds);
    UpdateBreathing(DeltaSeconds);
    UpdateAttackBlend(DeltaSeconds);
}

// ─── LOCOMOTION ───────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
    FVector Velocity = OwnerPawn->GetVelocity();
    Speed = Velocity.Size2D();

    if (Speed > 1.0f)
    {
        FRotator ActorRot = OwnerPawn->GetActorRotation();
        FVector Forward = ActorRot.Vector();
        FVector VelNorm = Velocity.GetSafeNormal2D();
        float Dot = FVector::DotProduct(Forward, VelNorm);
        Direction = UKismetMathLibrary::DegAcos(Dot);

        FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);
        if (FVector::DotProduct(Right, VelNorm) < 0.0f)
            Direction = -Direction;
    }
    else
    {
        Direction = FMath::FInterpTo(Direction, 0.0f, DeltaSeconds, 3.0f);
    }

    if (MovementComponent)
    {
        bIsInAir = MovementComponent->IsFalling();
    }
}

// ─── BEHAVIOR STATE ───────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateBehaviorState()
{
    EAnim_DinoState NewState = EAnim_DinoState::Idle;

    if (bIsSleeping)
    {
        NewState = EAnim_DinoState::Sleep;
    }
    else if (bIsEating)
    {
        NewState = EAnim_DinoState::Eat;
    }
    else if (bIsAttacking)
    {
        NewState = EAnim_DinoState::Attack;
    }
    else if (bIsFleeing)
    {
        NewState = EAnim_DinoState::Flee;
    }
    else if (bIsAggressive)
    {
        NewState = EAnim_DinoState::Aggressive;
    }
    else if (bIsAlert)
    {
        NewState = EAnim_DinoState::Alert;
    }
    else if (Speed > 300.0f)
    {
        NewState = EAnim_DinoState::Run;
    }
    else if (Speed > 50.0f)
    {
        NewState = EAnim_DinoState::Walk;
    }
    else
    {
        NewState = EAnim_DinoState::Idle;
    }

    BehaviorState = NewState;
}

// ─── PROCEDURAL TAIL ──────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateProceduralTail(float DeltaSeconds)
{
    // Tail sway increases with speed and aggression
    float SpeedFactor = FMath::Clamp(Speed / 600.0f, 0.3f, 1.5f);
    float AggressionFactor = bIsAggressive ? 2.0f : 1.0f;

    float CurrentSwaySpeed = TailSwaySpeed * SpeedFactor * AggressionFactor;
    TailSwayTime += DeltaSeconds * CurrentSwaySpeed;

    // Sinusoidal sway
    float SwayAmount = TailSwayAmount * SpeedFactor;
    TailSwayAngle = FMath::Sin(TailSwayTime) * SwayAmount;

    // Secondary frequency for natural feel
    TailSwayAngle += FMath::Sin(TailSwayTime * 2.3f) * SwayAmount * 0.3f;
}

// ─── HEAD TRACKING ────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateHeadTracking(float DeltaSeconds)
{
    if (!bEnableHeadTracking || !OwnerPawn) return;

    // Head tracking weight — higher when alert or aggressive
    float TargetWeight = 0.0f;
    if (bIsAlert || bIsAggressive)
        TargetWeight = 1.0f;
    else if (BehaviorState == EAnim_DinoState::Idle)
        TargetWeight = 0.4f;

    HeadTrackingWeight = FMath::FInterpTo(HeadTrackingWeight, TargetWeight, DeltaSeconds, 3.0f);
}

void UDinosaurAnimInstance::SetHeadTarget(FVector TargetWorldLocation)
{
    HeadTargetLocation = TargetWorldLocation;
    bIsAlert = true;
}

// ─── BREATHING ────────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateBreathing(float DeltaSeconds)
{
    // Breathing rate increases when running or aggressive
    float BreathRate = BreathCycleSpeed;
    if (Speed > 300.0f)
        BreathRate *= 2.5f;
    else if (bIsAggressive)
        BreathRate *= 1.8f;
    else if (bIsSleeping)
        BreathRate *= 0.4f;

    BreathTime += DeltaSeconds * BreathRate;
    BreathAlpha = (FMath::Sin(BreathTime * TWO_PI) + 1.0f) * 0.5f;
}

// ─── ATTACK BLEND ─────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateAttackBlend(float DeltaSeconds)
{
    float TargetBlend = bIsAttacking ? 1.0f : 0.0f;
    AttackBlendWeight = FMath::FInterpTo(AttackBlendWeight, TargetBlend, DeltaSeconds, 12.0f);
}

// ─── STATE SETTERS ────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::SetAggressive(bool bAggressive)
{
    bIsAggressive = bAggressive;
    if (bAggressive) bIsAlert = true;
}

void UDinosaurAnimInstance::SetAlert(bool bAlert)
{
    bIsAlert = bAlert;
}

void UDinosaurAnimInstance::SetFleeing(bool bFlee)
{
    bIsFleeing = bFlee;
    bIsAggressive = false;
}

void UDinosaurAnimInstance::SetEating(bool bEat)
{
    bIsEating = bEat;
}

void UDinosaurAnimInstance::SetSleeping(bool bSleep)
{
    bIsSleeping = bSleep;
    bIsAggressive = false;
    bIsAlert = false;
}

void UDinosaurAnimInstance::TriggerAttack(EAnim_DinoAttack Type)
{
    bIsAttacking = true;
    AttackType = Type;
    AttackBlendWeight = 0.0f;
}

void UDinosaurAnimInstance::EndAttack()
{
    bIsAttacking = false;
    AttackType = EAnim_DinoAttack::None;
}

void UDinosaurAnimInstance::SetBipedal(bool bBipedal)
{
    bIsBipedal = bBipedal;
}
