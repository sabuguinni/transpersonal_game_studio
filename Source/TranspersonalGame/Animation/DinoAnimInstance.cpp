#include "DinoAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UDinoAnimInstance::UDinoAnimInstance()
{
    WalkSpeedThreshold   = 50.f;
    TrotSpeedThreshold   = 200.f;
    RunSpeedThreshold    = 450.f;
    SprintSpeedThreshold = 700.f;
    bEnableFootIK        = true;
    FootIKTraceDistance  = 100.f;
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

    UpdateLocomotionState(DeltaSeconds);
    UpdateGait();

    if (bEnableFootIK)
    {
        UpdateFootIK();
    }
}

void UDinoAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    const FVector Velocity = OwnerPawn->GetVelocity();
    LocomotionState.Speed  = Velocity.Size2D();
    LocomotionState.bIsMoving = LocomotionState.Speed > 10.f;

    // Direction relative to actor forward
    if (LocomotionState.bIsMoving)
    {
        const FVector VelNorm = Velocity.GetSafeNormal2D();
        const FVector Forward = OwnerPawn->GetActorForwardVector();
        const FVector Right   = OwnerPawn->GetActorRightVector();
        LocomotionState.Direction = FMath::RadiansToDegrees(
            FMath::Atan2(FVector::DotProduct(VelNorm, Right),
                         FVector::DotProduct(VelNorm, Forward)));
    }
    else
    {
        LocomotionState.Direction = 0.f;
    }
}

void UDinoAnimInstance::UpdateGait()
{
    if (LocomotionState.bIsDead)
    {
        CurrentGait = EAnim_DinoGait::Dead;
        return;
    }
    if (LocomotionState.bIsAttacking)
    {
        CurrentGait = EAnim_DinoGait::Attack;
        return;
    }
    if (LocomotionState.bIsRoaring)
    {
        CurrentGait = EAnim_DinoGait::Roar;
        return;
    }

    const float S = LocomotionState.Speed;
    if      (S >= SprintSpeedThreshold) CurrentGait = EAnim_DinoGait::Sprint;
    else if (S >= RunSpeedThreshold)    CurrentGait = EAnim_DinoGait::Run;
    else if (S >= TrotSpeedThreshold)   CurrentGait = EAnim_DinoGait::Trot;
    else if (S >= WalkSpeedThreshold)   CurrentGait = EAnim_DinoGait::Walk;
    else                                CurrentGait = EAnim_DinoGait::Idle;
}

void UDinoAnimInstance::UpdateFootIK()
{
    TraceFootIK(FName("LeftFoot"),  LeftFootIKLocation);
    TraceFootIK(FName("RightFoot"), RightFootIKLocation);
}

void UDinoAnimInstance::TraceFootIK(const FName& FootSocketName, FVector& OutIKLocation)
{
    if (!OwnerPawn) return;

    USkeletalMeshComponent* Mesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh) return;

    const FVector SocketLoc = Mesh->GetSocketLocation(FootSocketName);
    const FVector TraceStart = SocketLoc + FVector(0.f, 0.f, FootIKTraceDistance * 0.5f);
    const FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, FootIKTraceDistance);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    UWorld* World = GetWorld();
    if (!World) return;

    if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        OutIKLocation = Hit.ImpactPoint;
    }
    else
    {
        OutIKLocation = SocketLoc;
    }
}

void UDinoAnimInstance::SetAttacking(bool bAttacking)
{
    LocomotionState.bIsAttacking = bAttacking;
}

void UDinoAnimInstance::SetRoaring(bool bRoaring)
{
    LocomotionState.bIsRoaring = bRoaring;
}

void UDinoAnimInstance::SetDead(bool bDead)
{
    LocomotionState.bIsDead = bDead;
}
