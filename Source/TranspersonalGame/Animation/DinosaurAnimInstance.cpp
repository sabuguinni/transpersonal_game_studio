#include "DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    LocomotionState      = EAnim_DinoLocomotionState::Idle;
    Speed                = 0.0f;
    Direction            = 0.0f;
    bIsMoving            = false;
    bIsAttacking         = false;
    bIsDead              = false;
    bIsRoaring           = false;
    WalkRunBlend         = 0.0f;
    AttackBlendWeight    = 0.0f;
    IKFootTraceDistance  = 80.0f;
    WalkSpeedThreshold   = 150.0f;
    RunSpeedThreshold    = 400.0f;
    LeftFootIKLocation   = FVector::ZeroVector;
    RightFootIKLocation  = FVector::ZeroVector;
    OwnerPawn            = nullptr;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerPawn = TryGetPawnOwner();
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn)
    {
        return;
    }

    // ── Velocity & Speed ──
    FVector Velocity = OwnerPawn->GetVelocity();
    Speed = Velocity.Size2D();
    bIsMoving = Speed > 10.0f;

    // ── Direction (for strafe blending) ──
    if (bIsMoving)
    {
        FRotator ActorRot = OwnerPawn->GetActorRotation();
        FRotator VelocityRot = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // ── State machine update ──
    UpdateLocomotionState();
    UpdateWalkRunBlend();

    // ── IK foot placement ──
    if (bIsMoving || LocomotionState == EAnim_DinoLocomotionState::Idle)
    {
        UpdateFootIK();
    }

    // ── Attack blend weight ──
    float TargetAttackWeight = bIsAttacking ? 1.0f : 0.0f;
    AttackBlendWeight = FMath::FInterpTo(AttackBlendWeight, TargetAttackWeight, DeltaSeconds, 8.0f);
}

void UDinosaurAnimInstance::UpdateLocomotionState()
{
    if (bIsDead)
    {
        LocomotionState = EAnim_DinoLocomotionState::Death;
        return;
    }
    if (bIsAttacking)
    {
        LocomotionState = EAnim_DinoLocomotionState::Attack;
        return;
    }
    if (bIsRoaring)
    {
        LocomotionState = EAnim_DinoLocomotionState::Roar;
        return;
    }
    if (Speed >= RunSpeedThreshold)
    {
        LocomotionState = EAnim_DinoLocomotionState::Run;
    }
    else if (Speed >= WalkSpeedThreshold)
    {
        LocomotionState = EAnim_DinoLocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_DinoLocomotionState::Idle;
    }
}

void UDinosaurAnimInstance::UpdateWalkRunBlend()
{
    // 0.0 = Walk, 1.0 = Run — used in BlendSpace
    float SpeedRange = RunSpeedThreshold - WalkSpeedThreshold;
    if (SpeedRange > 0.0f && Speed > WalkSpeedThreshold)
    {
        WalkRunBlend = FMath::Clamp((Speed - WalkSpeedThreshold) / SpeedRange, 0.0f, 1.0f);
    }
    else
    {
        WalkRunBlend = 0.0f;
    }
}

void UDinosaurAnimInstance::UpdateFootIK()
{
    TraceFootIK(FName("foot_l"), LeftFootIKLocation);
    TraceFootIK(FName("foot_r"), RightFootIKLocation);
}

void UDinosaurAnimInstance::TraceFootIK(FName FootSocketName, FVector& OutIKLocation)
{
    USkeletalMeshComponent* MeshComp = GetSkelMeshComponent();
    if (!MeshComp || !OwnerPawn)
    {
        return;
    }

    // Get foot socket world location
    FVector SocketLocation = MeshComp->GetSocketLocation(FootSocketName);

    // Trace downward to find ground
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, IKFootTraceDistance);
    FVector TraceEnd   = SocketLocation - FVector(0.0f, 0.0f, IKFootTraceDistance * 2.0f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);

    UWorld* World = OwnerPawn->GetWorld();
    if (!World)
    {
        return;
    }

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        OutIKLocation = HitResult.ImpactPoint;
    }
    else
    {
        OutIKLocation = SocketLocation;
    }
}

void UDinosaurAnimInstance::SetAttacking(bool bAttacking)
{
    bIsAttacking = bAttacking;
}

void UDinosaurAnimInstance::SetDead(bool bDead)
{
    bIsDead = bDead;
}

void UDinosaurAnimInstance::SetRoaring(bool bRoaring)
{
    bIsRoaring = bRoaring;
}
