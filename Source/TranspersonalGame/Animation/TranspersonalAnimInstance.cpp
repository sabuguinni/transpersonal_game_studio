#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Default thresholds — tuned for prehistoric human movement feel
    WalkThreshold = 10.0f;
    RunThreshold = 300.0f;
    FootTraceDistance = 50.0f;
    bEnableFootIK = true;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache owner character reference
    APawn* OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        OwnerCharacter = Cast<ACharacter>(OwnerPawn);
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        APawn* OwnerPawn = TryGetPawnOwner();
        if (OwnerPawn)
        {
            OwnerCharacter = Cast<ACharacter>(OwnerPawn);
        }
        return;
    }

    UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
    if (!MovComp)
    {
        return;
    }

    // ── Update Locomotion Variables ───────────────────────────────────────

    // Speed — horizontal velocity magnitude
    FVector Velocity = MovComp->Velocity;
    Speed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();

    // Direction — angle between character forward and velocity direction
    if (Speed > WalkThreshold)
    {
        FRotator AimRotation = OwnerCharacter->GetActorRotation();
        FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Velocity);
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // Airborne state
    bool bCurrentlyInAir = MovComp->IsFalling();

    // Landing detection
    if (bWasInAir && !bCurrentlyInAir)
    {
        OnLanded();
    }

    bWasInAir = bCurrentlyInAir;
    bIsInAir = bCurrentlyInAir;

    // Crouch state
    bIsCrouching = MovComp->IsCrouching();

    // Sprint detection
    bIsSprinting = (Speed >= RunThreshold);

    // ── Compute Animation State ───────────────────────────────────────────
    CurrentState = ComputeAnimState();

    // ── Foot IK ───────────────────────────────────────────────────────────
    if (bEnableFootIK && !bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        // Reset IK when airborne
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaSeconds, 10.0f);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, 10.0f);
        FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, 0.0f, DeltaSeconds, 10.0f);
    }
}

EAnim_CharacterState UTranspersonalAnimInstance::ComputeAnimState() const
{
    if (bIsInAir)
    {
        // Distinguish jump (upward velocity) from fall (downward)
        if (OwnerCharacter && OwnerCharacter->GetVelocity().Z > 0.0f)
        {
            return EAnim_CharacterState::Jump;
        }
        return EAnim_CharacterState::Fall;
    }

    if (bIsCrouching)
    {
        return EAnim_CharacterState::Crouch;
    }

    if (Speed >= RunThreshold)
    {
        return EAnim_CharacterState::Run;
    }

    if (Speed >= WalkThreshold)
    {
        return EAnim_CharacterState::Walk;
    }

    return EAnim_CharacterState::Idle;
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
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

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        return;
    }

    // Bone names — adjust to match the actual skeletal mesh rig
    const FName LeftFootBone  = TEXT("foot_l");
    const FName RightFootBone = TEXT("foot_r");

    auto TraceFootIK = [&](const FName& BoneName, FVector& OutTarget, float& OutAlpha) -> void
    {
        FVector BoneLocation = Mesh->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
        if (BoneLocation.IsZero())
        {
            OutAlpha = 0.0f;
            return;
        }

        FVector TraceStart = BoneLocation + FVector(0.0f, 0.0f, FootTraceDistance);
        FVector TraceEnd   = BoneLocation - FVector(0.0f, 0.0f, FootTraceDistance * 2.0f);

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
            OutTarget = HitResult.ImpactPoint;
            OutAlpha  = FMath::FInterpTo(OutAlpha, 1.0f, DeltaSeconds, 15.0f);

            // Compute pelvis offset from foot height difference
            float HeightDiff = BoneLocation.Z - HitResult.ImpactPoint.Z;
            TargetPelvisOffset = FMath::Min(TargetPelvisOffset, -HeightDiff);
        }
        else
        {
            OutAlpha = FMath::FInterpTo(OutAlpha, 0.0f, DeltaSeconds, 10.0f);
        }
    };

    // Reset pelvis offset each frame, let foot traces accumulate it
    TargetPelvisOffset = 0.0f;

    TraceFootIK(LeftFootBone,  FootIKData.LeftFootTarget,  FootIKData.LeftFootAlpha);
    TraceFootIK(RightFootBone, FootIKData.RightFootTarget, FootIKData.RightFootAlpha);

    // Smooth pelvis offset
    FootIKData.PelvisOffset = FMath::FInterpTo(
        FootIKData.PelvisOffset,
        TargetPelvisOffset,
        DeltaSeconds,
        10.0f
    );
}

void UTranspersonalAnimInstance::TriggerAttackMontage()
{
    // Attack montage is played via Blueprint — this C++ function
    // sets the state so the Blueprint state machine can react.
    // The actual UAnimMontage::Play() call happens in Blueprint
    // after the montage asset is assigned in the editor.
    CurrentState = EAnim_CharacterState::Attack;
}

bool UTranspersonalAnimInstance::IsMovingOnGround() const
{
    return !bIsInAir && Speed > WalkThreshold;
}
