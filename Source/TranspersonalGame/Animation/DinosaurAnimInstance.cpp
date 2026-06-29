// DinosaurAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full implementation of dinosaur animation instance
// Handles locomotion, attack states, pack behavior signals, and procedural head tracking

#include "DinosaurAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    // Locomotion defaults
    DinoSpeed = 0.0f;
    DinoSmoothedSpeed = 0.0f;
    DinoDirection = 0.0f;
    bDinoIsMoving = false;
    bDinoIsRunning = false;
    bDinoIsInAir = false;
    DinoGaitCycle = 0.0f;

    // Behavior state defaults
    DinoAIState = EAnim_DinoAIState::Idle;
    bDinoIsAttacking = false;
    bDinoIsEating = false;
    bDinoIsResting = false;
    bDinoIsAlerted = false;
    bDinoIsDead = false;
    AlertLevel = 0.0f;

    // Head tracking defaults
    bEnableHeadTracking = true;
    HeadTargetLocation = FVector::ZeroVector;
    HeadAimRotation = FRotator::ZeroRotator;
    HeadTrackingWeight = 0.0f;
    HeadTrackingInterpSpeed = 4.0f;

    // Tail physics defaults
    TailSwayAmount = 0.0f;
    TailSwayFrequency = 1.2f;
    TailSwayPhase = 0.0f;

    // Internal
    OwnerDinosaur = nullptr;
    OwnerMovement = nullptr;
    SpeedSmoothingRate = 6.0f;
    AccumulatedTime = 0.0f;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        OwnerDinosaur = Cast<ACharacter>(OwnerPawn);
        if (OwnerDinosaur)
        {
            OwnerMovement = OwnerDinosaur->GetCharacterMovement();
        }
    }
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    AccumulatedTime += DeltaSeconds;

    if (!OwnerDinosaur || !OwnerMovement)
    {
        APawn* OwnerPawn = TryGetPawnOwner();
        if (OwnerPawn)
        {
            OwnerDinosaur = Cast<ACharacter>(OwnerPawn);
            if (OwnerDinosaur)
            {
                OwnerMovement = OwnerDinosaur->GetCharacterMovement();
            }
        }
        return;
    }

    UpdateDinoLocomotion(DeltaSeconds);
    UpdateDinoBehaviorState(DeltaSeconds);
    UpdateHeadTracking(DeltaSeconds);
    UpdateTailPhysics(DeltaSeconds);
}

void UDinosaurAnimInstance::UpdateDinoLocomotion(float DeltaSeconds)
{
    if (!OwnerMovement)
        return;

    FVector Velocity = OwnerMovement->Velocity;
    float RawSpeed = Velocity.Size2D();

    DinoSmoothedSpeed = FMath::FInterpTo(DinoSmoothedSpeed, RawSpeed, DeltaSeconds, SpeedSmoothingRate);
    DinoSpeed = DinoSmoothedSpeed;

    bDinoIsMoving = RawSpeed > 15.0f;
    bDinoIsInAir = OwnerMovement->IsFalling();

    // Running threshold: 300 cm/s (walk ~150, trot ~300, run ~600+)
    bDinoIsRunning = RawSpeed > 300.0f;

    // Direction for strafing (most dinos don't strafe but useful for pack animals)
    if (bDinoIsMoving)
    {
        FRotator ActorRotation = OwnerDinosaur->GetActorRotation();
        FVector VelocityNorm = Velocity.GetSafeNormal2D();
        FVector ForwardVector = ActorRotation.Vector();

        DinoDirection = UKismetMathLibrary::DegAtan2(
            FVector::DotProduct(VelocityNorm, FVector::CrossProduct(FVector::UpVector, ForwardVector)),
            FVector::DotProduct(VelocityNorm, ForwardVector)
        );
    }
    else
    {
        DinoDirection = FMath::FInterpTo(DinoDirection, 0.0f, DeltaSeconds, 4.0f);
    }

    // Gait cycle: accumulates with speed for procedural animation timing
    if (bDinoIsMoving)
    {
        float GaitRate = DinoSpeed / 400.0f; // normalized gait rate
        DinoGaitCycle = FMath::Fmod(DinoGaitCycle + DeltaSeconds * GaitRate * 2.0f, 1.0f);
    }
}

void UDinosaurAnimInstance::UpdateDinoBehaviorState(float DeltaSeconds)
{
    // Alert level smoothing
    AlertLevel = FMath::Clamp(AlertLevel, 0.0f, 1.0f);

    // Auto-transition to alert if moving fast
    if (DinoSpeed > 350.0f && DinoAIState == EAnim_DinoAIState::Idle)
    {
        DinoAIState = EAnim_DinoAIState::Alert;
    }

    // Head tracking weight: higher when alerted or hunting
    float TargetHeadWeight = 0.0f;
    switch (DinoAIState)
    {
        case EAnim_DinoAIState::Idle:
            TargetHeadWeight = 0.3f;
            break;
        case EAnim_DinoAIState::Alert:
            TargetHeadWeight = 0.7f;
            break;
        case EAnim_DinoAIState::Hunting:
            TargetHeadWeight = 1.0f;
            break;
        case EAnim_DinoAIState::Attacking:
            TargetHeadWeight = 0.5f; // attacking uses montage, reduce IK
            break;
        case EAnim_DinoAIState::Fleeing:
            TargetHeadWeight = 0.2f;
            break;
        case EAnim_DinoAIState::Eating:
            TargetHeadWeight = 0.0f; // eating animation handles head
            break;
        case EAnim_DinoAIState::Resting:
            TargetHeadWeight = 0.1f;
            break;
        case EAnim_DinoAIState::Dead:
            TargetHeadWeight = 0.0f;
            break;
        default:
            TargetHeadWeight = 0.0f;
            break;
    }

    HeadTrackingWeight = FMath::FInterpTo(HeadTrackingWeight, TargetHeadWeight, DeltaSeconds, 3.0f);
}

void UDinosaurAnimInstance::UpdateHeadTracking(float DeltaSeconds)
{
    if (!bEnableHeadTracking || !OwnerDinosaur)
        return;

    // Calculate desired head rotation to look at target
    if (!HeadTargetLocation.IsNearlyZero())
    {
        USkeletalMeshComponent* MeshComp = OwnerDinosaur->GetMesh();
        if (MeshComp)
        {
            FVector HeadBoneLocation = MeshComp->GetSocketLocation(TEXT("head"));
            FVector ToTarget = (HeadTargetLocation - HeadBoneLocation).GetSafeNormal();

            FRotator DesiredRotation = ToTarget.Rotation();
            FRotator ActorRotation = OwnerDinosaur->GetActorRotation();

            // Clamp head rotation to realistic limits
            FRotator RelativeDesired = DesiredRotation - ActorRotation;
            RelativeDesired.Yaw = FMath::Clamp(RelativeDesired.Yaw, -80.0f, 80.0f);
            RelativeDesired.Pitch = FMath::Clamp(RelativeDesired.Pitch, -40.0f, 40.0f);
            RelativeDesired.Roll = 0.0f;

            HeadAimRotation = FMath::RInterpTo(HeadAimRotation, RelativeDesired, DeltaSeconds, HeadTrackingInterpSpeed);
        }
    }
    else
    {
        // No target: return to neutral
        HeadAimRotation = FMath::RInterpTo(HeadAimRotation, FRotator::ZeroRotator, DeltaSeconds, HeadTrackingInterpSpeed);
    }
}

void UDinosaurAnimInstance::UpdateTailPhysics(float DeltaSeconds)
{
    // Procedural tail sway based on movement speed
    float SpeedFactor = FMath::Clamp(DinoSpeed / 600.0f, 0.0f, 1.0f);
    float TargetSwayAmount = bDinoIsMoving ? FMath::Lerp(0.3f, 1.0f, SpeedFactor) : 0.15f;

    TailSwayAmount = FMath::FInterpTo(TailSwayAmount, TargetSwayAmount, DeltaSeconds, 3.0f);

    // Phase accumulates over time for sinusoidal sway
    TailSwayPhase = FMath::Fmod(TailSwayPhase + DeltaSeconds * TailSwayFrequency, 2.0f * PI);
}

void UDinosaurAnimInstance::SetAIState(EAnim_DinoAIState NewState)
{
    if (DinoAIState == EAnim_DinoAIState::Dead)
        return; // Can't leave dead state

    DinoAIState = NewState;

    // Update convenience booleans
    bDinoIsAttacking = (NewState == EAnim_DinoAIState::Attacking);
    bDinoIsEating = (NewState == EAnim_DinoAIState::Eating);
    bDinoIsResting = (NewState == EAnim_DinoAIState::Resting);
    bDinoIsAlerted = (NewState == EAnim_DinoAIState::Alert || NewState == EAnim_DinoAIState::Hunting);
}

void UDinosaurAnimInstance::SetHeadTrackingTarget(FVector WorldLocation)
{
    HeadTargetLocation = WorldLocation;
}

void UDinosaurAnimInstance::ClearHeadTrackingTarget()
{
    HeadTargetLocation = FVector::ZeroVector;
}

void UDinosaurAnimInstance::TriggerDinoAttack()
{
    if (bDinoIsDead)
        return;

    SetAIState(EAnim_DinoAIState::Attacking);

    // Reset to hunting after attack window (montage notify should handle this,
    // but we set a timer as fallback)
    GetWorld()->GetTimerManager().SetTimer(
        AttackResetTimer,
        [this]()
        {
            if (DinoAIState == EAnim_DinoAIState::Attacking)
            {
                SetAIState(EAnim_DinoAIState::Hunting);
            }
        },
        1.2f,
        false
    );
}

void UDinosaurAnimInstance::TriggerDinoDeath()
{
    bDinoIsDead = true;
    bDinoIsAttacking = false;
    bDinoIsEating = false;
    bDinoIsResting = false;
    bDinoIsAlerted = false;
    DinoAIState = EAnim_DinoAIState::Dead;
    HeadTrackingWeight = 0.0f;
    TailSwayAmount = 0.0f;
}
