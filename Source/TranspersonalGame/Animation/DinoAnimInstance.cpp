#include "DinoAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UDinoAnimInstance::UDinoAnimInstance()
    : LocomotionState(EAnim_DinoLocomotionState::Idle)
    , GroundSpeed(0.f)
    , MovementDirection(0.f)
    , bIsInAir(false)
    , bIsAttacking(false)
    , bIsRoaring(false)
    , bIsDead(false)
    , FootIKAlpha(1.f)
    , LeftFootIKLocation(FVector::ZeroVector)
    , RightFootIKLocation(FVector::ZeroVector)
    , OwnerPawn(nullptr)
{
}

void UDinoAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerPawn = TryGetPawnOwner();
}

void UDinoAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        if (!OwnerPawn) return;
    }

    // --- Ground speed ---
    const FVector Velocity = OwnerPawn->GetVelocity();
    GroundSpeed = Velocity.Size2D();

    // --- Movement direction (relative to actor forward) ---
    const FRotator ActorRot = OwnerPawn->GetActorRotation();
    const FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
    MovementDirection = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    // --- Air state (Character subclass) ---
    if (ACharacter* Char = Cast<ACharacter>(OwnerPawn))
    {
        bIsInAir = Char->GetCharacterMovement()->IsFalling();
    }

    // --- Pack AnimData struct ---
    AnimData.Speed = GroundSpeed;
    AnimData.Direction = MovementDirection;
    AnimData.bIsInAir = bIsInAir;
    AnimData.bIsAttacking = bIsAttacking;
    AnimData.bIsRoaring = bIsRoaring;
    AnimData.bIsDead = bIsDead;
    AnimData.LocomotionState = LocomotionState;

    // --- Resolve locomotion state ---
    if (!bIsDead)
    {
        LocomotionState = ResolveLocomotionState();
    }

    // --- Foot IK ---
    UpdateFootIK(DeltaSeconds);
}

EAnim_DinoLocomotionState UDinoAnimInstance::ResolveLocomotionState() const
{
    if (bIsAttacking) return EAnim_DinoLocomotionState::Attack;
    if (bIsRoaring)   return EAnim_DinoLocomotionState::Roar;
    if (bIsEating)    return EAnim_DinoLocomotionState::Eat;
    if (bIsInAir)     return EAnim_DinoLocomotionState::Run; // airborne = sprint

    if (GroundSpeed >= RunSpeedThreshold)  return EAnim_DinoLocomotionState::Run;
    if (GroundSpeed >= WalkSpeedThreshold) return EAnim_DinoLocomotionState::Walk;
    return EAnim_DinoLocomotionState::Idle;
}

void UDinoAnimInstance::TriggerAttack()
{
    if (bIsDead) return;
    bIsAttacking = true;
    LocomotionState = EAnim_DinoLocomotionState::Attack;
    // Montage playback is handled by the AnimBlueprint state machine.
    // Reset flag after montage ends via AnimNotify in BP.
}

void UDinoAnimInstance::TriggerRoar()
{
    if (bIsDead || bIsAttacking) return;
    bIsRoaring = true;
    LocomotionState = EAnim_DinoLocomotionState::Roar;
}

void UDinoAnimInstance::TriggerDeath()
{
    bIsDead = true;
    bIsAttacking = false;
    bIsRoaring = false;
    LocomotionState = EAnim_DinoLocomotionState::Death;
    FootIKAlpha = 0.f; // Disable IK on death
}

void UDinoAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerPawn || bIsDead)
    {
        FootIKAlpha = 0.f;
        return;
    }

    // Smoothly enable IK when idle or walking
    const float TargetAlpha = (GroundSpeed < RunSpeedThreshold) ? 1.f : 0.f;
    FootIKAlpha = FMath::FInterpTo(FootIKAlpha, TargetAlpha, DeltaSeconds, 5.f);

    if (FootIKAlpha < 0.01f) return;

    LeftFootIKLocation  = TraceFootToGround(TEXT("foot_l"));
    RightFootIKLocation = TraceFootToGround(TEXT("foot_r"));
}

FVector UDinoAnimInstance::TraceFootToGround(const FName& FootSocketName) const
{
    if (!OwnerPawn) return FVector::ZeroVector;

    USkeletalMeshComponent* Mesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh) return FVector::ZeroVector;

    const FVector FootWorldPos = Mesh->GetSocketLocation(FootSocketName);
    const FVector TraceStart   = FootWorldPos + FVector(0.f, 0.f, 100.f);
    const FVector TraceEnd     = FootWorldPos - FVector(0.f, 0.f, 150.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    const bool bHit = OwnerPawn->GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd, ECC_WorldStatic, Params);

    return bHit ? HitResult.ImpactPoint : FootWorldPos;
}
