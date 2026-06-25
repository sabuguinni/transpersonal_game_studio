#include "DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    GroundSpeed = 0.0f;
    bIsMoving = false;
    bIsAttacking = false;
    bIsDead = false;
    LocomotionState = EAnim_DinoLocomotionState::Idle;
    MovementDirection = 0.0f;
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    OwnerPawn = nullptr;
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerPawn = TryGetPawnOwner();
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        if (!OwnerPawn) return;
    }

    // Compute ground speed (horizontal velocity only)
    FVector Velocity = OwnerPawn->GetVelocity();
    Velocity.Z = 0.0f;
    GroundSpeed = Velocity.Size();
    bIsMoving = GroundSpeed > WalkSpeedThreshold;

    // Movement direction relative to actor forward
    if (bIsMoving)
    {
        FVector Forward = OwnerPawn->GetActorForwardVector();
        FVector VelNorm = Velocity.GetSafeNormal();
        MovementDirection = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, VelNorm)));
        // Determine sign (left/right)
        FVector Right = OwnerPawn->GetActorRightVector();
        if (FVector::DotProduct(Right, VelNorm) < 0.0f)
        {
            MovementDirection *= -1.0f;
        }
    }
    else
    {
        MovementDirection = 0.0f;
    }

    UpdateLocomotionState();
    UpdateFootIK();
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
    if (GroundSpeed >= RunSpeedThreshold)
    {
        LocomotionState = EAnim_DinoLocomotionState::Run;
    }
    else if (GroundSpeed >= WalkSpeedThreshold)
    {
        LocomotionState = EAnim_DinoLocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_DinoLocomotionState::Idle;
    }
}

void UDinosaurAnimInstance::UpdateFootIK()
{
    if (!OwnerPawn) return;

    USkeletalMeshComponent* Mesh = GetSkelMeshComponent();
    if (!Mesh) return;

    // Only update IK when not running (performance optimization)
    if (LocomotionState == EAnim_DinoLocomotionState::Run) return;

    LeftFootIKLocation = ComputeFootIKLocation(FName("foot_l"));
    RightFootIKLocation = ComputeFootIKLocation(FName("foot_r"));
}

FVector UDinosaurAnimInstance::ComputeFootIKLocation(FName SocketName) const
{
    USkeletalMeshComponent* Mesh = GetSkelMeshComponent();
    if (!Mesh || !OwnerPawn) return FVector::ZeroVector;

    UWorld* World = OwnerPawn->GetWorld();
    if (!World) return FVector::ZeroVector;

    FVector SocketLocation = Mesh->GetSocketLocation(SocketName);

    // Raycast downward to find ground
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, 100.0f);
    FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, 200.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, Params))
    {
        return HitResult.ImpactPoint;
    }

    return SocketLocation;
}
