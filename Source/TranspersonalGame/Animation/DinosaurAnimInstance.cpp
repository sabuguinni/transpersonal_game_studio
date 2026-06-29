// DinosaurAnimInstance.cpp
// Animation Agent #10 — PROD_CYCLE_AUTO_20260629_002
// Runtime animation brain for all dinosaur species in the prehistoric survival game.
// Drives locomotion, combat, behavior blend weights via NativeUpdateAnimation.

#include "DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    // Locomotion defaults
    GroundSpeed = 0.0f;
    VerticalSpeed = 0.0f;
    bIsMoving = false;
    bIsFalling = false;
    bIsSprinting = false;
    MovementDirection = 0.0f;
    TurnRate = 0.0f;
    LocomotionState = EAnim_DinosaurLocomotionState::Idle;

    // Combat defaults
    AttackType = EAnim_DinosaurAttackType::None;
    bIsAttacking = false;
    bIsInCombat = false;
    bIsDead = false;
    bIsWounded = false;
    HealthPercent = 1.0f;
    StaminaPercent = 1.0f;
    AttackBlendWeight = 0.0f;
    WoundedBlendWeight = 0.0f;

    // Behavior defaults
    bIsEating = false;
    bIsDrinking = false;
    bIsResting = false;
    bIsAlerted = false;
    bIsRoaring = false;
    bIsSniffing = false;
    AlertBlendWeight = 0.0f;
    BehaviorBlendWeight = 0.0f;

    // Species / IK defaults
    Species = EAnim_DinosaurSpecies::Velociraptor;
    bEnableFootIK = true;
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    SpineIKBend = 0.0f;
    NeckIKTarget = FVector::ZeroVector;
    TailSwayAmount = 0.0f;
    TailSwaySpeed = 1.0f;

    // Blend space defaults
    LocomotionBlendSpaceX = 0.0f;
    LocomotionBlendSpaceY = 0.0f;
    TurnBlendAlpha = 0.0f;
    LandingImpactAlpha = 0.0f;

    // Cached refs
    OwnerPawn = nullptr;
    OwnerMovement = nullptr;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        ACharacter* OwnerChar = Cast<ACharacter>(OwnerPawn);
        if (OwnerChar)
        {
            OwnerMovement = OwnerChar->GetCharacterMovement();
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

        ACharacter* OwnerChar = Cast<ACharacter>(OwnerPawn);
        if (OwnerChar)
        {
            OwnerMovement = OwnerChar->GetCharacterMovement();
        }
    }

    UpdateLocomotion(DeltaSeconds);
    UpdateCombat(DeltaSeconds);
    UpdateBehavior(DeltaSeconds);
    UpdateIK(DeltaSeconds);
}

void UDinosaurAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    FVector Velocity = OwnerPawn->GetVelocity();
    GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
    VerticalSpeed = Velocity.Z;

    bIsMoving = GroundSpeed > 10.0f;
    bIsFalling = OwnerMovement ? OwnerMovement->IsFalling() : false;

    // Determine locomotion state
    if (bIsDead)
    {
        LocomotionState = EAnim_DinosaurLocomotionState::Dead;
    }
    else if (bIsFalling)
    {
        LocomotionState = EAnim_DinosaurLocomotionState::Falling;
    }
    else if (bIsSprinting && GroundSpeed > 400.0f)
    {
        LocomotionState = EAnim_DinosaurLocomotionState::Sprint;
    }
    else if (bIsMoving && GroundSpeed > 150.0f)
    {
        LocomotionState = EAnim_DinosaurLocomotionState::Run;
    }
    else if (bIsMoving)
    {
        LocomotionState = EAnim_DinosaurLocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_DinosaurLocomotionState::Idle;
    }

    // Blend space inputs: X = direction (-180 to 180), Y = speed (0 to 1)
    if (bIsMoving && OwnerPawn)
    {
        FVector ForwardVec = OwnerPawn->GetActorForwardVector();
        FVector VelocityNorm = Velocity.GetSafeNormal();
        MovementDirection = UKismetMathLibrary::DegAcos(
            FVector::DotProduct(ForwardVec, VelocityNorm)
        );
        // Determine sign (left/right)
        FVector CrossProduct = FVector::CrossProduct(ForwardVec, VelocityNorm);
        if (CrossProduct.Z < 0.0f) MovementDirection = -MovementDirection;
    }
    else
    {
        MovementDirection = 0.0f;
    }

    // Normalize speed for blend space Y axis (0=idle, 1=sprint)
    float MaxSpeed = bIsSprinting ? 800.0f : 500.0f;
    LocomotionBlendSpaceX = MovementDirection;
    LocomotionBlendSpaceY = FMath::Clamp(GroundSpeed / MaxSpeed, 0.0f, 1.0f);

    // Tail sway: faster when running
    TailSwaySpeed = FMath::Lerp(0.5f, 3.0f, LocomotionBlendSpaceY);
    TailSwayAmount = FMath::Lerp(0.2f, 1.0f, LocomotionBlendSpaceY);
}

void UDinosaurAnimInstance::UpdateCombat(float DeltaSeconds)
{
    // Blend attack weight in/out smoothly
    float TargetAttackWeight = bIsAttacking ? 1.0f : 0.0f;
    AttackBlendWeight = FMath::FInterpTo(AttackBlendWeight, TargetAttackWeight, DeltaSeconds, 8.0f);

    // Wounded blend based on health
    float TargetWoundedWeight = (HealthPercent < 0.35f) ? 1.0f : 0.0f;
    WoundedBlendWeight = FMath::FInterpTo(WoundedBlendWeight, TargetWoundedWeight, DeltaSeconds, 3.0f);

    // Spine IK bend during attack (lean forward)
    float TargetSpineBend = bIsAttacking ? 15.0f : 0.0f;
    SpineIKBend = FMath::FInterpTo(SpineIKBend, TargetSpineBend, DeltaSeconds, 6.0f);
}

void UDinosaurAnimInstance::UpdateBehavior(float DeltaSeconds)
{
    // Alert blend
    float TargetAlertWeight = bIsAlerted ? 1.0f : 0.0f;
    AlertBlendWeight = FMath::FInterpTo(AlertBlendWeight, TargetAlertWeight, DeltaSeconds, 4.0f);

    // Behavior blend (eating/drinking/resting)
    float TargetBehaviorWeight = (bIsEating || bIsDrinking || bIsResting) ? 1.0f : 0.0f;
    BehaviorBlendWeight = FMath::FInterpTo(BehaviorBlendWeight, TargetBehaviorWeight, DeltaSeconds, 2.0f);

    // Neck IK: point toward food/water source when eating/drinking
    if (bIsEating || bIsDrinking)
    {
        // Neck lowers toward ground
        if (OwnerPawn)
        {
            FVector ActorLoc = OwnerPawn->GetActorLocation();
            NeckIKTarget = ActorLoc + FVector(100.0f, 0.0f, -80.0f);
        }
    }
    else if (bIsAlerted && OwnerPawn)
    {
        // Neck raises and points forward when alerted
        FVector ActorLoc = OwnerPawn->GetActorLocation();
        NeckIKTarget = ActorLoc + OwnerPawn->GetActorForwardVector() * 300.0f + FVector(0.0f, 0.0f, 50.0f);
    }
}

void UDinosaurAnimInstance::UpdateIK(float DeltaSeconds)
{
    if (!bEnableFootIK || !OwnerPawn) return;

    UWorld* World = OwnerPawn->GetWorld();
    if (!World) return;

    // Foot IK: trace downward from each foot socket to find ground
    FVector ActorLoc = OwnerPawn->GetActorLocation();
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(OwnerPawn);

    // Left foot trace
    FVector LeftFootStart = ActorLoc + FVector(-30.0f, 30.0f, 50.0f);
    FVector LeftFootEnd = LeftFootStart + FVector(0.0f, 0.0f, -150.0f);
    FHitResult LeftHit;
    bool bLeftHit = World->LineTraceSingleByChannel(LeftHit, LeftFootStart, LeftFootEnd, ECC_WorldStatic, TraceParams);

    if (bLeftHit)
    {
        FVector TargetOffset = FVector(0.0f, 0.0f, LeftHit.ImpactPoint.Z - ActorLoc.Z + 90.0f);
        LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, TargetOffset, DeltaSeconds, 12.0f);
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 1.0f, DeltaSeconds, 8.0f);
    }
    else
    {
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, 8.0f);
    }

    // Right foot trace
    FVector RightFootStart = ActorLoc + FVector(-30.0f, -30.0f, 50.0f);
    FVector RightFootEnd = RightFootStart + FVector(0.0f, 0.0f, -150.0f);
    FHitResult RightHit;
    bool bRightHit = World->LineTraceSingleByChannel(RightHit, RightFootStart, RightFootEnd, ECC_WorldStatic, TraceParams);

    if (bRightHit)
    {
        FVector TargetOffset = FVector(0.0f, 0.0f, RightHit.ImpactPoint.Z - ActorLoc.Z + 90.0f);
        RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, TargetOffset, DeltaSeconds, 12.0f);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 1.0f, DeltaSeconds, 8.0f);
    }
    else
    {
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, 8.0f);
    }
}

void UDinosaurAnimInstance::SetLocomotionState(EAnim_DinosaurLocomotionState NewState)
{
    LocomotionState = NewState;
}

void UDinosaurAnimInstance::TriggerAttack(EAnim_DinosaurAttackType InAttackType)
{
    AttackType = InAttackType;
    bIsAttacking = true;
    bIsInCombat = true;
}

void UDinosaurAnimInstance::OnAttackEnd()
{
    bIsAttacking = false;
    AttackType = EAnim_DinosaurAttackType::None;
}

void UDinosaurAnimInstance::SetHealthPercent(float NewHealth)
{
    HealthPercent = FMath::Clamp(NewHealth, 0.0f, 1.0f);
    if (HealthPercent <= 0.0f)
    {
        bIsDead = true;
        bIsAttacking = false;
        bIsInCombat = false;
    }
    bIsWounded = HealthPercent < 0.35f;
}

void UDinosaurAnimInstance::SetBehaviorState(bool bEating, bool bDrinking, bool bResting, bool bAlerted, bool bRoaring, bool bSniffing)
{
    bIsEating = bEating;
    bIsDrinking = bDrinking;
    bIsResting = bResting;
    bIsAlerted = bAlerted;
    bIsRoaring = bRoaring;
    bIsSniffing = bSniffing;
}
