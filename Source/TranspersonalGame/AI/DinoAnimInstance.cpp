#include "DinoAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UDinoAnimInstance::UDinoAnimInstance()
{
}

void UDinoAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn)
    {
        return;
    }

    PreviousVelocity = FVector::ZeroVector;
    HeadTargetWorld = FVector::ZeroVector;
}

void UDinoAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    APawn* OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn || DeltaSeconds <= 0.0f)
    {
        return;
    }

    // ── Ground speed and vertical speed ──────────────────────────
    FVector Velocity = OwnerPawn->GetVelocity();
    GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
    VerticalSpeed = Velocity.Z;
    bIsMoving = GroundSpeed > 10.0f;

    // ── Gait classification ───────────────────────────────────────
    UpdateGait();

    // ── Foot IK terrain adaptation ────────────────────────────────
    UpdateFootIK(DeltaSeconds);

    // ── Directional lean ─────────────────────────────────────────
    UpdateLean(DeltaSeconds);

    // ── Head tracking ─────────────────────────────────────────────
    UpdateHeadTracking(DeltaSeconds);

    PreviousVelocity = Velocity;
}

void UDinoAnimInstance::UpdateGait()
{
    if (GroundSpeed < 10.0f)
    {
        CurrentGait = ENPC_DinoGait::Stationary;
    }
    else if (GroundSpeed < WalkSpeed)
    {
        CurrentGait = ENPC_DinoGait::Walk;
    }
    else if (GroundSpeed < TrotSpeed)
    {
        CurrentGait = ENPC_DinoGait::Trot;
    }
    else if (GroundSpeed < RunSpeed)
    {
        CurrentGait = ENPC_DinoGait::Run;
    }
    else
    {
        CurrentGait = ENPC_DinoGait::Sprint;
    }
}

void UDinoAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    // Trace all four feet and calculate pelvis offset
    FrontLeftFoot  = TraceFootIK(FName("foot_fl"));
    FrontRightFoot = TraceFootIK(FName("foot_fr"));
    RearLeftFoot   = TraceFootIK(FName("foot_rl"));
    RearRightFoot  = TraceFootIK(FName("foot_rr"));

    // Pelvis offset — push pelvis down to lowest foot
    float LowestOffset = FMath::Min3(
        FrontLeftFoot.FootLocation.Z,
        FrontRightFoot.FootLocation.Z,
        FMath::Min(RearLeftFoot.FootLocation.Z, RearRightFoot.FootLocation.Z)
    );

    APawn* OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        float OwnerZ = OwnerPawn->GetActorLocation().Z;
        float TargetPelvisOffset = FMath::Clamp(LowestOffset - OwnerZ, -40.0f, 0.0f);
        PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaSeconds, 8.0f);
    }
}

FNPC_DinoFootIK UDinoAnimInstance::TraceFootIK(FName SocketName, float TraceLength)
{
    FNPC_DinoFootIK Result;

    USkeletalMeshComponent* MeshComp = GetSkelMeshComponent();
    if (!MeshComp)
    {
        return Result;
    }

    UWorld* World = MeshComp->GetWorld();
    if (!World)
    {
        return Result;
    }

    // Check if socket exists
    if (!MeshComp->DoesSocketExist(SocketName))
    {
        // Socket not found — return neutral IK (no terrain adaptation)
        Result.IKAlpha = 0.0f;
        return Result;
    }

    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, TraceLength);
    FVector TraceEnd   = SocketLocation - FVector(0.0f, 0.0f, TraceLength * 2.0f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(MeshComp->GetOwner());

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        Result.FootLocation = HitResult.ImpactPoint;
        Result.bGroundContact = true;

        // Calculate foot rotation from surface normal
        FVector SurfaceNormal = HitResult.ImpactNormal;
        FVector ForwardDir = MeshComp->GetOwner()->GetActorForwardVector();
        FVector RightDir   = FVector::CrossProduct(SurfaceNormal, ForwardDir).GetSafeNormal();
        FVector AdjustedForward = FVector::CrossProduct(RightDir, SurfaceNormal).GetSafeNormal();

        Result.FootRotation = FRotationMatrix::MakeFromZX(SurfaceNormal, AdjustedForward).Rotator();
        Result.IKAlpha = 1.0f;
    }
    else
    {
        Result.FootLocation = SocketLocation;
        Result.bGroundContact = false;
        Result.IKAlpha = 0.0f;
    }

    return Result;
}

void UDinoAnimInstance::UpdateLean(float DeltaSeconds)
{
    APawn* OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn)
    {
        return;
    }

    FVector Velocity = OwnerPawn->GetVelocity();
    FVector Acceleration = (Velocity - PreviousVelocity) / FMath::Max(DeltaSeconds, SMALL_NUMBER);

    FVector ForwardDir = OwnerPawn->GetActorForwardVector();
    FVector RightDir   = OwnerPawn->GetActorRightVector();

    float TargetLeanFB = FVector::DotProduct(Acceleration, ForwardDir) / 1000.0f;
    float TargetLeanLR = FVector::DotProduct(Acceleration, RightDir)   / 1000.0f;

    TargetLeanFB = FMath::Clamp(TargetLeanFB, -1.0f, 1.0f);
    TargetLeanLR = FMath::Clamp(TargetLeanLR, -1.0f, 1.0f);

    LeanData.LeanForwardBack = FMath::FInterpTo(LeanData.LeanForwardBack, TargetLeanFB, DeltaSeconds, LeanInterpSpeed);
    LeanData.LeanLeftRight   = FMath::FInterpTo(LeanData.LeanLeftRight,   TargetLeanLR, DeltaSeconds, LeanInterpSpeed);
}

void UDinoAnimInstance::UpdateHeadTracking(float DeltaSeconds)
{
    if (HeadTargetWorld.IsNearlyZero() || HeadTrackAlpha < 0.01f)
    {
        HeadAimRotation = FMath::RInterpTo(HeadAimRotation, FRotator::ZeroRotator, DeltaSeconds, HeadInterpSpeed);
        return;
    }

    USkeletalMeshComponent* MeshComp = GetSkelMeshComponent();
    if (!MeshComp)
    {
        return;
    }

    FVector HeadSocketLoc = MeshComp->DoesSocketExist(FName("head"))
        ? MeshComp->GetSocketLocation(FName("head"))
        : MeshComp->GetOwner()->GetActorLocation() + FVector(0, 0, 100);

    FVector ToTarget = (HeadTargetWorld - HeadSocketLoc).GetSafeNormal();
    FRotator TargetRot = ToTarget.Rotation();

    // Clamp head rotation to realistic range
    TargetRot.Pitch = FMath::Clamp(TargetRot.Pitch, -45.0f, 30.0f);
    TargetRot.Yaw   = FMath::Clamp(TargetRot.Yaw,   -60.0f, 60.0f);
    TargetRot.Roll  = 0.0f;

    HeadAimRotation = FMath::RInterpTo(HeadAimRotation, TargetRot, DeltaSeconds, HeadInterpSpeed);
}

// ── State setters ─────────────────────────────────────────────────

void UDinoAnimInstance::SetLocoState(ENPC_DinoLocoState NewState)
{
    LocoState = NewState;

    // Sync boolean flags from state
    bIsAttacking = (NewState == ENPC_DinoLocoState::Attack);
    bIsRoaring   = (NewState == ENPC_DinoLocoState::Roar);
    bIsDead      = (NewState == ENPC_DinoLocoState::Dead);
}

void UDinoAnimInstance::SetAttacking(bool bAttacking)
{
    bIsAttacking = bAttacking;
    if (bAttacking)
    {
        LocoState = ENPC_DinoLocoState::Attack;
    }
}

void UDinoAnimInstance::SetRoaring(bool bRoaring)
{
    bIsRoaring = bRoaring;
    if (bRoaring)
    {
        LocoState = ENPC_DinoLocoState::Roar;
    }
}

void UDinoAnimInstance::SetDead(bool bDead)
{
    bIsDead = bDead;
    if (bDead)
    {
        LocoState = ENPC_DinoLocoState::Dead;
    }
}

void UDinoAnimInstance::SetHeadTarget(FVector WorldTarget, float Alpha)
{
    HeadTargetWorld = WorldTarget;
    HeadTrackAlpha  = FMath::Clamp(Alpha, 0.0f, 1.0f);
}
