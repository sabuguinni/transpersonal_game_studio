#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UPlayerAnimInstance::UPlayerAnimInstance()
    : Speed(0.f)
    , SmoothedSpeed(0.f)
    , Direction(0.f)
    , LeanAmount(0.f)
    , VerticalVelocity(0.f)
    , bIsMoving(false)
    , bIsFalling(false)
    , bIsCrouching(false)
    , bIsSprinting(false)
    , LocomotionState(EAnim_LocomotionState::Idle)
    , WeaponState(EAnim_WeaponState::Unarmed)
    , LeftFootIKLocation(FVector::ZeroVector)
    , RightFootIKLocation(FVector::ZeroVector)
    , LeftFootAlpha(0.f)
    , RightFootAlpha(0.f)
    , StaminaRatio(1.f)
    , FearLevel(0.f)
    , bIsExhausted(false)
    , bIsInjured(false)
    , OwnerCharacter(nullptr)
    , MovementComponent(nullptr)
{
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        OwnerCharacter = Cast<ACharacter>(Pawn);
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // --- Raw velocity data ---
    FVector Velocity = OwnerCharacter->GetVelocity();
    Speed = Velocity.Size2D();
    VerticalVelocity = Velocity.Z;

    // --- Smoothed speed for blend space ---
    SmoothedSpeed = SmoothFloat(SmoothedSpeed, Speed, 8.f, DeltaSeconds);

    // --- Direction (relative to actor forward) ---
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    Direction = UKismetMathLibrary::NormalizedDeltaRotator(
        UKismetMathLibrary::MakeRotFromX(Velocity),
        ActorRot
    ).Yaw;

    // --- Lean (based on direction change rate) ---
    float TargetLean = FMath::Clamp(Direction * 0.1f, -1.f, 1.f);
    LeanAmount = SmoothFloat(LeanAmount, TargetLean, 5.f, DeltaSeconds);

    // --- State flags ---
    bIsMoving = Speed > 10.f;
    bIsFalling = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Sprint: speed > 400 cm/s (approx 4 m/s)
    bIsSprinting = Speed > 400.f && bIsMoving && !bIsCrouching;

    // --- Update subsystems ---
    UpdateLocomotionState();
    UpdateFootIK(DeltaSeconds);
    UpdateSurvivalState();
}

void UPlayerAnimInstance::UpdateLocomotionState()
{
    if (bIsFalling)
    {
        if (VerticalVelocity < -200.f)
        {
            LocomotionState = EAnim_LocomotionState::InAir;
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Land;
        }
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = Speed > 10.f
            ? EAnim_LocomotionState::Sneak
            : EAnim_LocomotionState::Crouch;
        return;
    }

    if (!bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
        return;
    }

    if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed > 200.f)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter)
    {
        return;
    }

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
    {
        return;
    }

    // Only apply foot IK when grounded
    if (bIsFalling)
    {
        LeftFootAlpha  = SmoothFloat(LeftFootAlpha,  0.f, 10.f, DeltaSeconds);
        RightFootAlpha = SmoothFloat(RightFootAlpha, 0.f, 10.f, DeltaSeconds);
        return;
    }

    const float TraceLength = 60.f;
    const FName LeftFootBone  = TEXT("foot_l");
    const FName RightFootBone = TEXT("foot_r");

    auto TraceFootIK = [&](FName BoneName, FVector& OutLocation, float& OutAlpha)
    {
        USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
        if (!Mesh)
        {
            return;
        }

        FVector BoneLocation = Mesh->GetBoneLocation(BoneName);
        FVector TraceStart   = BoneLocation + FVector(0.f, 0.f, TraceLength);
        FVector TraceEnd     = BoneLocation - FVector(0.f, 0.f, TraceLength);

        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        bool bHit = World->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,
            Params
        );

        if (bHit)
        {
            OutLocation = HitResult.ImpactPoint;
            OutAlpha    = SmoothFloat(OutAlpha, 1.f, 10.f, DeltaSeconds);
        }
        else
        {
            OutAlpha = SmoothFloat(OutAlpha, 0.f, 10.f, DeltaSeconds);
        }
    };

    TraceFootIK(LeftFootBone,  LeftFootIKLocation,  LeftFootAlpha);
    TraceFootIK(RightFootBone, RightFootIKLocation, RightFootAlpha);
}

void UPlayerAnimInstance::UpdateSurvivalState()
{
    // Stub: in full implementation, query TranspersonalCharacter survival stats
    // For now, derive exhaustion from speed and stamina ratio
    bIsExhausted = StaminaRatio < 0.15f;
    bIsInjured   = false; // Will be set by damage system integration
}

float UPlayerAnimInstance::SmoothFloat(float Current, float Target, float Speed, float DeltaSeconds) const
{
    return FMath::FInterpTo(Current, Target, DeltaSeconds, Speed);
}
