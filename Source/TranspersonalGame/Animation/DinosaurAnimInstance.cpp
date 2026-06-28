#include "DinosaurAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
    : LocomotionState(EAnim_DinoLocomotionState::Idle)
    , GroundSpeed(0.f)
    , Direction(0.f)
    , bIsMoving(false)
    , bIsInAir(false)
    , bIsDead(false)
    , BodySize(EAnim_DinoBodySize::Large)
    , AggressionLevel(0.f)
    , HealthRatio(1.f)
    , LeftFootIKTarget(FVector::ZeroVector)
    , RightFootIKTarget(FVector::ZeroVector)
    , IKFootBlendWeight(0.f)
    , HeadLookAtRotation(FRotator::ZeroRotator)
    , HeadLookAtWeight(0.f)
    , TailSwayAmount(0.f)
    , OwnerPawn(nullptr)
    , MovementComponent(nullptr)
    , TailSwayTime(0.f)
{
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn)
    {
        return;
    }

    // Try to get movement component — dinosaurs may be Characters or generic Pawns
    ACharacter* AsCharacter = Cast<ACharacter>(OwnerPawn);
    if (AsCharacter)
    {
        MovementComponent = AsCharacter->GetCharacterMovement();
    }
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        if (!OwnerPawn) return;
    }

    // ─── Ground Speed & Direction ────────────────────────────────────────
    FVector Velocity = OwnerPawn->GetVelocity();
    Velocity.Z = 0.f;
    GroundSpeed = Velocity.Size();
    bIsMoving = GroundSpeed > 10.f;

    // Calculate direction relative to actor forward
    FVector ForwardVector = OwnerPawn->GetActorForwardVector();
    Direction = UKismetMathLibrary::DegAtan2(
        FVector::DotProduct(Velocity.GetSafeNormal(), OwnerPawn->GetActorRightVector()),
        FVector::DotProduct(Velocity.GetSafeNormal(), ForwardVector)
    );

    // ─── Air State ───────────────────────────────────────────────────────
    if (MovementComponent)
    {
        bIsInAir = MovementComponent->IsFalling();
    }

    // ─── Locomotion State Machine ────────────────────────────────────────
    UpdateLocomotionState();

    // ─── Procedural Systems ──────────────────────────────────────────────
    UpdateIKTargets(DeltaSeconds);
    UpdateHeadLookAt(DeltaSeconds);
    UpdateTailSway(DeltaSeconds);
}

void UDinosaurAnimInstance::UpdateLocomotionState()
{
    if (bIsDead)
    {
        LocomotionState = EAnim_DinoLocomotionState::Dead;
        return;
    }

    if (bIsInAir)
    {
        // Dinosaurs generally don't fly — keep current state
        return;
    }

    if (!bIsMoving)
    {
        LocomotionState = EAnim_DinoLocomotionState::Idle;
        return;
    }

    // Speed thresholds vary by body size
    float WalkThreshold  = 150.f;
    float TrotThreshold  = 350.f;

    switch (BodySize)
    {
        case EAnim_DinoBodySize::Small:
            WalkThreshold = 100.f;
            TrotThreshold = 250.f;
            break;
        case EAnim_DinoBodySize::Medium:
            WalkThreshold = 130.f;
            TrotThreshold = 300.f;
            break;
        case EAnim_DinoBodySize::Large:
            WalkThreshold = 150.f;
            TrotThreshold = 350.f;
            break;
        case EAnim_DinoBodySize::Massive:
            WalkThreshold = 200.f;
            TrotThreshold = 450.f;
            break;
    }

    if (GroundSpeed < WalkThreshold)
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

void UDinosaurAnimInstance::UpdateIKTargets(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    UWorld* World = OwnerPawn->GetWorld();
    if (!World) return;

    // Only blend IK when on ground and moving slowly
    float TargetBlend = (bIsMoving && !bIsInAir) ? 1.f : 0.f;
    IKFootBlendWeight = FMath::FInterpTo(IKFootBlendWeight, TargetBlend, DeltaSeconds, 8.f);

    if (IKFootBlendWeight < 0.01f) return;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    // Left foot trace
    FVector LeftStart  = OwnerPawn->GetActorLocation() + FVector(-30.f, -40.f, 50.f);
    FVector LeftEnd    = LeftStart - FVector(0.f, 0.f, 120.f);
    FHitResult LeftHit;
    if (World->LineTraceSingleByChannel(LeftHit, LeftStart, LeftEnd, ECC_WorldStatic, Params))
    {
        LeftFootIKTarget = LeftHit.ImpactPoint;
    }

    // Right foot trace
    FVector RightStart = OwnerPawn->GetActorLocation() + FVector(-30.f, 40.f, 50.f);
    FVector RightEnd   = RightStart - FVector(0.f, 0.f, 120.f);
    FHitResult RightHit;
    if (World->LineTraceSingleByChannel(RightHit, RightStart, RightEnd, ECC_WorldStatic, Params))
    {
        RightFootIKTarget = RightHit.ImpactPoint;
    }
}

void UDinosaurAnimInstance::UpdateHeadLookAt(float DeltaSeconds)
{
    // Head look-at weight: raise when idle or aggression is high
    float TargetWeight = (LocomotionState == EAnim_DinoLocomotionState::Idle || AggressionLevel > 0.5f) ? 1.f : 0.3f;
    HeadLookAtWeight = FMath::FInterpTo(HeadLookAtWeight, TargetWeight, DeltaSeconds, 3.f);
}

void UDinosaurAnimInstance::UpdateTailSway(float DeltaSeconds)
{
    TailSwayTime += DeltaSeconds;

    // Tail sway amplitude scales with speed and body size
    float BaseAmplitude = 1.f;
    switch (BodySize)
    {
        case EAnim_DinoBodySize::Small:   BaseAmplitude = 0.8f; break;
        case EAnim_DinoBodySize::Medium:  BaseAmplitude = 1.0f; break;
        case EAnim_DinoBodySize::Large:   BaseAmplitude = 1.3f; break;
        case EAnim_DinoBodySize::Massive: BaseAmplitude = 1.8f; break;
    }

    float SpeedFactor = FMath::Clamp(GroundSpeed / 400.f, 0.2f, 1.5f);
    TailSwayAmount = FMath::Sin(TailSwayTime * 2.f * SpeedFactor) * BaseAmplitude;
}

void UDinosaurAnimInstance::TriggerAttackMontage()
{
    LocomotionState = EAnim_DinoLocomotionState::Attacking;
    // Montage asset is assigned in Blueprint — this sets the state flag
    // The ABP state machine transitions to Attack state on next tick
}

void UDinosaurAnimInstance::TriggerRoarMontage()
{
    LocomotionState = EAnim_DinoLocomotionState::Roaring;
}

void UDinosaurAnimInstance::TriggerDeathMontage()
{
    bIsDead = true;
    LocomotionState = EAnim_DinoLocomotionState::Dead;
}
