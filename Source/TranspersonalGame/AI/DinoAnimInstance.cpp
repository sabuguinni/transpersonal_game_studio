#include "DinoAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UDinoAnimInstance::UDinoAnimInstance()
{
    Speed = 0.f;
    Direction = 0.f;
    LeanAngle = 0.f;
    bIsMoving = false;
    bIsInAir = false;
    LocomotionState = ENPC_DinoLocomotionState::Idle;

    bIsAttacking = false;
    bIsAlert = false;
    bIsFeeding = false;
    bIsSleeping = false;
    bIsFleeing = false;
    AlertLevel = ENPC_DinoAlertLevel::Passive;

    PackThreatLevel = 0.f;
    bIsPackLeader = false;
    NearbyPackMemberCount = 0;

    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    FootIKAlpha = 0.f;
    PelvisOffset = 0.f;

    HeadPitch = 0.f;
    HeadYaw = 0.f;

    HealthNormalized = 1.f;
    bIsInjured = false;
    bIsDead = false;

    AttackPhase = 0.f;
    bBiteActive = false;
    bTailSwipeActive = false;

    TailSwingAmount = 0.f;
    TailSwingSpeed = 0.f;

    OwnerPawn = nullptr;
    PreviousSpeed = 0.f;
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

    // ── Velocity / Speed ────────────────────────────────────────────────────
    FVector Velocity = OwnerPawn->GetVelocity();
    PreviousSpeed = Speed;
    Speed = Velocity.Size2D();
    bIsMoving = Speed > 10.f;

    // ── Direction (relative to actor forward) ───────────────────────────────
    if (bIsMoving)
    {
        FRotator ActorRot = OwnerPawn->GetActorRotation();
        FVector LocalVel = ActorRot.UnrotateVector(Velocity);
        Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVel.Y, LocalVel.X));
    }
    else
    {
        Direction = FMath::FInterpTo(Direction, 0.f, DeltaSeconds, 5.f);
    }

    // ── Lean (acceleration-based, organic weight) ────────────────────────────
    float SpeedDelta = Speed - PreviousSpeed;
    float TargetLean = FMath::Clamp(Direction * 0.15f + SpeedDelta * 0.3f, -25.f, 25.f);
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 4.f);

    // ── Air state ────────────────────────────────────────────────────────────
    ACharacter* AsChar = Cast<ACharacter>(OwnerPawn);
    if (AsChar && AsChar->GetCharacterMovement())
    {
        bIsInAir = AsChar->GetCharacterMovement()->IsFalling();
    }

    // ── Locomotion state ─────────────────────────────────────────────────────
    UpdateLocomotionState();

    // ── Foot IK (only when grounded) ─────────────────────────────────────────
    if (!bIsInAir && !bIsDead)
    {
        UpdateFootIK(DeltaSeconds);
        FootIKAlpha = FMath::FInterpTo(FootIKAlpha, 1.f, DeltaSeconds, 8.f);
    }
    else
    {
        FootIKAlpha = FMath::FInterpTo(FootIKAlpha, 0.f, DeltaSeconds, 8.f);
    }

    // ── Tail physics ─────────────────────────────────────────────────────────
    UpdateTailPhysics(DeltaSeconds);

    // ── Head tracking (placeholder — driven by BT in full impl) ─────────────
    HeadPitch = FMath::FInterpTo(HeadPitch, 0.f, DeltaSeconds, 3.f);
    HeadYaw = FMath::FInterpTo(HeadYaw, 0.f, DeltaSeconds, 3.f);
}

void UDinoAnimInstance::UpdateLocomotionState()
{
    if (bIsDead)
    {
        LocomotionState = ENPC_DinoLocomotionState::Dead;
        return;
    }
    if (bIsSleeping)
    {
        LocomotionState = ENPC_DinoLocomotionState::Sleeping;
        return;
    }
    if (bIsFeeding)
    {
        LocomotionState = ENPC_DinoLocomotionState::Feeding;
        return;
    }
    if (bIsAttacking)
    {
        LocomotionState = ENPC_DinoLocomotionState::Attack;
        return;
    }

    // Speed thresholds tuned for large predators (T-Rex walk ~300, trot ~600, run ~1200)
    if (Speed < 50.f)
        LocomotionState = ENPC_DinoLocomotionState::Idle;
    else if (Speed < 350.f)
        LocomotionState = ENPC_DinoLocomotionState::Walk;
    else if (Speed < 700.f)
        LocomotionState = ENPC_DinoLocomotionState::Trot;
    else
        LocomotionState = ENPC_DinoLocomotionState::Run;
}

bool UDinoAnimInstance::TraceFootIK(FName SocketName, FVector& OutLocation, FRotator& OutRotation)
{
    if (!OwnerPawn) return false;

    USkeletalMeshComponent* Mesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh) return false;

    FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLoc + FVector(0.f, 0.f, 150.f);
    FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, 200.f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    UWorld* World = OwnerPawn->GetWorld();
    if (!World) return false;

    bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);
    if (bHit)
    {
        OutLocation = Hit.ImpactPoint;
        // Compute foot rotation from surface normal
        FVector Normal = Hit.ImpactNormal;
        FRotator ActorRot = OwnerPawn->GetActorRotation();
        FVector Forward = ActorRot.Vector();
        FVector Right = FVector::CrossProduct(Normal, Forward).GetSafeNormal();
        Forward = FVector::CrossProduct(Right, Normal).GetSafeNormal();
        OutRotation = UKismetMathLibrary::MakeRotFromXZ(Forward, Normal);
        return true;
    }
    return false;
}

void UDinoAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    FVector LFLoc, RFLoc;
    FRotator LFRot, RFRot;

    bool bLeftHit  = TraceFootIK(FName("foot_l"), LFLoc, LFRot);
    bool bRightHit = TraceFootIK(FName("foot_r"), RFLoc, RFRot);

    if (bLeftHit)
    {
        LeftFootIKLocation  = FMath::VInterpTo(LeftFootIKLocation,  LFLoc, DeltaSeconds, 12.f);
        LeftFootIKRotation  = FMath::RInterpTo(LeftFootIKRotation,  LFRot, DeltaSeconds, 12.f);
    }
    if (bRightHit)
    {
        RightFootIKLocation = FMath::VInterpTo(RightFootIKLocation, RFLoc, DeltaSeconds, 12.f);
        RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, RFRot, DeltaSeconds, 12.f);
    }

    // Pelvis offset: sink pelvis to lowest foot to avoid floating
    if (bLeftHit && bRightHit)
    {
        float LeftDelta  = LeftFootIKLocation.Z  - OwnerPawn->GetActorLocation().Z;
        float RightDelta = RightFootIKLocation.Z - OwnerPawn->GetActorLocation().Z;
        float TargetPelvis = FMath::Min(LeftDelta, RightDelta) * 0.5f;
        PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, 10.f);
    }
}

void UDinoAnimInstance::UpdateTailPhysics(float DeltaSeconds)
{
    // Tail swings more at higher speeds and during turns
    float TargetSwingAmount = FMath::GetMappedRangeValueClamped(
        FVector2D(0.f, 1200.f), FVector2D(0.f, 1.f), Speed);

    // Add extra swing during direction changes (lateral momentum)
    float LateralFactor = FMath::Abs(Direction) / 180.f;
    TargetSwingAmount = FMath::Clamp(TargetSwingAmount + LateralFactor * 0.3f, 0.f, 1.f);

    TailSwingAmount = FMath::FInterpTo(TailSwingAmount, TargetSwingAmount, DeltaSeconds, 3.f);
    TailSwingSpeed  = FMath::GetMappedRangeValueClamped(
        FVector2D(0.f, 1200.f), FVector2D(0.5f, 3.f), Speed);
}
