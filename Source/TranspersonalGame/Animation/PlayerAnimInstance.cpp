#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Default config values set in header
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache owner character reference
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        // Re-attempt cache if not set
        OwnerCharacter = Cast<ACharacter>(GetOwningActor());
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
        return;
    }

    UpdateLocomotionData(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateLean(DeltaSeconds);
    UpdateSurvivalBlending(DeltaSeconds);

    // Foot IK runs on game thread (requires world trace)
    if (bEnableFootIK && !bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        // Reset foot IK when in air
        LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
    // Thread-safe updates (read-only, no world traces)
}

void UPlayerAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    // Get velocity and speed
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    VerticalVelocity = Velocity.Z;

    // Determine if moving
    bIsMoving = Speed > 5.0f;

    // Movement state flags
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Determine direction angle relative to character facing
    if (bIsMoving)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FRotator VelocityRot = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // Determine locomotion state
    EAnim_LocomotionState NewState = DetermineLocomotionState();
    if (NewState != LocomotionState)
    {
        EAnim_LocomotionState OldState = LocomotionState;
        LocomotionState = NewState;

        // Fire events on state transitions
        if (NewState == EAnim_LocomotionState::Jump)
        {
            OnJumped();
        }
        else if (NewState == EAnim_LocomotionState::Land)
        {
            OnLanded();
        }
        else if (NewState == EAnim_LocomotionState::Dead)
        {
            OnDeathStarted();
        }
    }
}

EAnim_LocomotionState UPlayerAnimInstance::DetermineLocomotionState() const
{
    if (bIsDead) return EAnim_LocomotionState::Dead;

    if (bIsInAir)
    {
        return VerticalVelocity > 0.0f ? EAnim_LocomotionState::Jump : EAnim_LocomotionState::Fall;
    }

    if (bIsCrouching)
    {
        return bIsMoving ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    }

    if (!bIsMoving) return EAnim_LocomotionState::Idle;

    if (Speed >= SprintSpeedThreshold) return EAnim_LocomotionState::Sprint;
    if (Speed >= RunSpeedThreshold)    return EAnim_LocomotionState::Run;
    if (Speed >= WalkSpeedThreshold)   return EAnim_LocomotionState::Walk;

    return EAnim_LocomotionState::Idle;
}

void UPlayerAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    // Clamp aim offset to reasonable range
    AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f);
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Trace for left and right foot
    bool bLeftHit = TraceForFootIK(FName("foot_l"), LeftFootIK);
    bool bRightHit = TraceForFootIK(FName("foot_r"), RightFootIK);

    // Interpolate IK alpha
    float TargetLeftAlpha = bLeftHit ? 1.0f : 0.0f;
    float TargetRightAlpha = bRightHit ? 1.0f : 0.0f;
    LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, TargetLeftAlpha, DeltaSeconds, FootIKInterpSpeed);
    RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, TargetRightAlpha, DeltaSeconds, FootIKInterpSpeed);

    // Calculate pelvis offset (lower pelvis to accommodate lower foot)
    float LowestFoot = FMath::Min(LeftFootIK.FootLocation.Z, RightFootIK.FootLocation.Z);
    float CharZ = OwnerCharacter->GetActorLocation().Z;
    float TargetPelvisOffset = FMath::Min(0.0f, LowestFoot - CharZ);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaSeconds, FootIKInterpSpeed * 0.5f);
}

bool UPlayerAnimInstance::TraceForFootIK(const FName& FootSocketName, FAnim_FootIKData& OutFootData)
{
    if (!OwnerCharacter) return false;

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp) return false;

    // Get foot socket world location
    FVector FootSocketLoc = MeshComp->GetSocketLocation(FootSocketName);

    // Trace downward from above the foot
    FVector TraceStart = FootSocketLoc + FVector(0.0f, 0.0f, FootIKTraceDistance);
    FVector TraceEnd = FootSocketLoc - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        OutFootData.FootLocation = HitResult.ImpactPoint;
        OutFootData.bFootOnGround = true;

        // Calculate foot rotation from surface normal
        FVector Normal = HitResult.ImpactNormal;
        FRotator FootRot = UKismetMathLibrary::MakeRotFromZX(Normal, OwnerCharacter->GetActorForwardVector());
        OutFootData.FootRotation = FootRot;
    }
    else
    {
        OutFootData.bFootOnGround = false;
    }

    return bHit;
}

void UPlayerAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    FVector CurrentVelocity = MovementComponent->Velocity;
    FVector VelocityDelta = CurrentVelocity - PreviousVelocity;

    // Project acceleration onto character axes for lean
    FVector ForwardVec = OwnerCharacter->GetActorForwardVector();
    FVector RightVec = OwnerCharacter->GetActorRightVector();

    float TargetLeanFB = FVector::DotProduct(VelocityDelta, ForwardVec) * 0.01f;
    float TargetLeanLR = FVector::DotProduct(VelocityDelta, RightVec) * 0.01f;

    // Clamp lean values
    TargetLeanFB = FMath::Clamp(TargetLeanFB, -15.0f, 15.0f);
    TargetLeanLR = FMath::Clamp(TargetLeanLR, -15.0f, 15.0f);

    // Smooth lean
    SmoothedLeanFB = FMath::FInterpTo(SmoothedLeanFB, TargetLeanFB, DeltaSeconds, 8.0f);
    SmoothedLeanLR = FMath::FInterpTo(SmoothedLeanLR, TargetLeanLR, DeltaSeconds, 8.0f);

    LeanData.LeanForwardBack = SmoothedLeanFB;
    LeanData.LeanLeftRight = SmoothedLeanLR;

    PreviousVelocity = CurrentVelocity;
}

void UPlayerAnimInstance::UpdateSurvivalBlending(float DeltaSeconds)
{
    // Injury and exhaustion blend weights are set externally via SetInjuryState / SetExhaustionState
    // Smooth the transitions
    float TargetInjuryWeight = bIsInjured ? InjuryBlendWeight : 0.0f;
    float TargetExhaustionWeight = bIsExhausted ? ExhaustionBlendWeight : 0.0f;

    InjuryBlendWeight = FMath::FInterpTo(InjuryBlendWeight, TargetInjuryWeight, DeltaSeconds, 3.0f);
    ExhaustionBlendWeight = FMath::FInterpTo(ExhaustionBlendWeight, TargetExhaustionWeight, DeltaSeconds, 3.0f);
}

void UPlayerAnimInstance::SetInjuryState(float InjuryAlpha, bool bInjured)
{
    bIsInjured = bInjured;
    InjuryBlendWeight = FMath::Clamp(InjuryAlpha, 0.0f, 1.0f);
}

void UPlayerAnimInstance::SetExhaustionState(float ExhaustionAlpha, bool bExhausted)
{
    bIsExhausted = bExhausted;
    ExhaustionBlendWeight = FMath::Clamp(ExhaustionAlpha, 0.0f, 1.0f);
}
