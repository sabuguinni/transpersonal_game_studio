#include "DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    LocomotionState = EAnim_DinoLocomotionState::Idle;
    Speed = 0.f;
    Direction = 0.f;
    bIsMoving = false;
    bIsAttacking = false;
    bIsRoaring = false;
    bIsDead = false;
    WalkRunAlpha = 0.f;
    TurnRate = 0.f;
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    BodyIKOffset = 0.f;
    WalkSpeedThreshold = 50.f;
    RunSpeedThreshold = 300.f;
    OwnerPawn = nullptr;
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

    // --- Velocity & Speed ---
    FVector Velocity = OwnerPawn->GetVelocity();
    Velocity.Z = 0.f;
    Speed = Velocity.Size();
    bIsMoving = Speed > WalkSpeedThreshold;

    // --- Direction (relative to actor forward) ---
    FRotator ActorRot = OwnerPawn->GetActorRotation();
    FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
    Direction = FMath::Atan2(LocalVelocity.Y, LocalVelocity.X) * (180.f / PI);

    // --- WalkRunAlpha for blend space ---
    if (Speed <= WalkSpeedThreshold)
    {
        WalkRunAlpha = 0.f;
    }
    else if (Speed >= RunSpeedThreshold)
    {
        WalkRunAlpha = 1.f;
    }
    else
    {
        WalkRunAlpha = (Speed - WalkSpeedThreshold) / (RunSpeedThreshold - WalkSpeedThreshold);
    }

    // --- Turn rate (yaw delta) ---
    static float LastYaw = ActorRot.Yaw;
    float CurrentYaw = ActorRot.Yaw;
    TurnRate = FMath::FindDeltaAngleDegrees(LastYaw, CurrentYaw) / FMath::Max(DeltaSeconds, 0.001f);
    LastYaw = CurrentYaw;

    // --- Locomotion state machine ---
    UpdateLocomotionState();

    // --- Foot IK ---
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

void UDinosaurAnimInstance::UpdateFootIK()
{
    if (!OwnerPawn)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = GetSkelMeshComponent();
    if (!MeshComp)
    {
        return;
    }

    // Solve IK for each foot socket
    SolveFootIK(FName("foot_l"), LeftFootIKLocation);
    SolveFootIK(FName("foot_r"), RightFootIKLocation);

    // Body offset: average of both foot offsets relative to ground
    float LeftDelta = LeftFootIKLocation.Z - OwnerPawn->GetActorLocation().Z;
    float RightDelta = RightFootIKLocation.Z - OwnerPawn->GetActorLocation().Z;
    BodyIKOffset = FMath::Min(LeftDelta, RightDelta);
}

void UDinosaurAnimInstance::SolveFootIK(FName FootSocketName, FVector& OutIKLocation)
{
    USkeletalMeshComponent* MeshComp = GetSkelMeshComponent();
    if (!MeshComp || !OwnerPawn)
    {
        OutIKLocation = FVector::ZeroVector;
        return;
    }

    // Get socket world location
    FVector SocketLoc = MeshComp->GetSocketLocation(FootSocketName);

    // Raycast downward to find ground
    FVector TraceStart = SocketLoc + FVector(0.f, 0.f, 100.f);
    FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, 300.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    UWorld* World = OwnerPawn->GetWorld();
    if (World && World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        OutIKLocation = HitResult.ImpactPoint;
    }
    else
    {
        OutIKLocation = SocketLoc;
    }
}
