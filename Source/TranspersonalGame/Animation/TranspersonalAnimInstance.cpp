#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    LocomotionState = EAnim_LocomotionState::Idle;
    StanceType = EAnim_StanceType::Upright;
    bEnableFootIK = true;
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    SprintSpeedThreshold = 500.0f;
    RunSpeedThreshold = 200.0f;
    WalkSpeedThreshold = 10.0f;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
        return;
    }

    UpdateLocomotionData(DeltaSeconds);
    UpdateLocomotionState();
    UpdateAimOffset(DeltaSeconds);
    UpdateBlendWeights(DeltaSeconds);

    if (bEnableFootIK && !LocomotionData.bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        // Reset foot IK when in air
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    // Landing timer
    if (LandingTimer > 0.0f)
    {
        LandingTimer -= DeltaSeconds;
    }
}

void UTranspersonalAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    const FVector Velocity = MovementComponent->Velocity;
    const float CurrentSpeed = Velocity.Size2D();

    LocomotionData.Speed = CurrentSpeed;
    LocomotionData.bIsMoving = CurrentSpeed > WalkSpeedThreshold;
    LocomotionData.bIsInAir = MovementComponent->IsFalling();
    LocomotionData.bIsCrouching = MovementComponent->IsCrouching();

    // Sprint detection — check if moving faster than run threshold
    LocomotionData.bIsSprinting = CurrentSpeed >= SprintSpeedThreshold;

    // Direction calculation
    if (LocomotionData.bIsMoving)
    {
        LocomotionData.Direction = CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());
    }
    else
    {
        LocomotionData.Direction = FMath::FInterpTo(LocomotionData.Direction, 0.0f, DeltaSeconds, 5.0f);
    }

    // Lean calculation (rate of direction change)
    const float TargetLean = FMath::Clamp(LocomotionData.Direction * 0.1f, -1.0f, 1.0f);
    LocomotionData.LeanAmount = FMath::FInterpTo(LocomotionData.LeanAmount, TargetLean, DeltaSeconds, 4.0f);
}

void UTranspersonalAnimInstance::UpdateLocomotionState()
{
    if (!bIsAlive)
    {
        LocomotionState = EAnim_LocomotionState::Death;
        return;
    }

    if (bIsAttacking)
    {
        LocomotionState = EAnim_LocomotionState::Attack;
        return;
    }

    if (LocomotionData.bIsInAir)
    {
        if (MovementComponent && MovementComponent->Velocity.Z < -100.0f)
        {
            LocomotionState = EAnim_LocomotionState::Fall;
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Jump;
        }
        return;
    }

    if (LandingTimer > 0.0f)
    {
        LocomotionState = EAnim_LocomotionState::Land;
        return;
    }

    if (LocomotionData.bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouch;
        StanceType = EAnim_StanceType::Crouched;
        return;
    }
    else
    {
        StanceType = EAnim_StanceType::Upright;
    }

    if (!LocomotionData.bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
    else if (LocomotionData.bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (LocomotionData.Speed >= RunSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    FVector LeftLoc, RightLoc;
    FRotator LeftRot, RightRot;

    const bool bLeftHit = PerformFootTrace(FName("foot_l"), LeftLoc, LeftRot);
    const bool bRightHit = PerformFootTrace(FName("foot_r"), RightLoc, RightRot);

    if (bLeftHit)
    {
        FootIKData.LeftFootLocation = FMath::VInterpTo(FootIKData.LeftFootLocation, LeftLoc, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.LeftFootRotation = FMath::RInterpTo(FootIKData.LeftFootRotation, LeftRot, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    if (bRightHit)
    {
        FootIKData.RightFootLocation = FMath::VInterpTo(FootIKData.RightFootLocation, RightLoc, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootRotation = FMath::RInterpTo(FootIKData.RightFootRotation, RightRot, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    // Pelvis offset — lower pelvis to accommodate the lower foot
    const float LeftDelta = bLeftHit ? (FootIKData.LeftFootLocation.Z - OwnerCharacter->GetActorLocation().Z) : 0.0f;
    const float RightDelta = bRightHit ? (FootIKData.RightFootLocation.Z - OwnerCharacter->GetActorLocation().Z) : 0.0f;
    const float TargetPelvisOffset = FMath::Min(LeftDelta, RightDelta);
    FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, TargetPelvisOffset, DeltaSeconds, FootIKInterpSpeed);
}

bool UTranspersonalAnimInstance::PerformFootTrace(FName SocketName, FVector& OutLocation, FRotator& OutRotation)
{
    if (!OwnerCharacter) return false;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return false;

    const FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    const FVector TraceStart = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z + FootIKTraceDistance);
    const FVector TraceEnd = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z - FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        OutLocation = HitResult.ImpactPoint;
        // Align foot rotation to surface normal
        const FVector Normal = HitResult.ImpactNormal;
        OutRotation = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
            0.0f,
            FMath::RadiansToDegrees(FMath::Atan2(-Normal.Y, Normal.Z))
        );
        return true;
    }

    return false;
}

void UTranspersonalAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    const FRotator ActorRot = OwnerCharacter->GetActorRotation();
    const FRotator ControlRot = OwnerCharacter->GetControlRotation();

    const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    const float TargetPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    const float TargetYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.0f, 90.0f);

    AimPitch = FMath::FInterpTo(AimPitch, TargetPitch, DeltaSeconds, 10.0f);
    AimYaw = FMath::FInterpTo(AimYaw, TargetYaw, DeltaSeconds, 10.0f);

    LocomotionData.AimPitch = AimPitch;
    LocomotionData.AimYaw = AimYaw;
}

void UTranspersonalAnimInstance::UpdateBlendWeights(float DeltaSeconds)
{
    // Upper body weight: full when attacking or aiming, reduced when sprinting
    float TargetUpperWeight = 1.0f;
    if (LocomotionData.bIsSprinting && !bIsAttacking)
    {
        TargetUpperWeight = 0.3f;
    }
    UpperBodyLayerWeight = FMath::FInterpTo(UpperBodyLayerWeight, TargetUpperWeight, DeltaSeconds, 5.0f);
    LowerBodyLayerWeight = 1.0f;
}

float UTranspersonalAnimInstance::CalculateDirection(const FVector& Velocity, const FRotator& ActorRotation) const
{
    if (Velocity.IsNearlyZero()) return 0.0f;

    const FMatrix RotMatrix = FRotationMatrix(ActorRotation);
    const FVector ForwardVector = RotMatrix.GetScaledAxis(EAxis::X);
    const FVector RightVector = RotMatrix.GetScaledAxis(EAxis::Y);
    const FVector NormalVelocity = Velocity.GetSafeNormal2D();

    const float ForwardCosAngle = FVector::DotProduct(ForwardVector, NormalVelocity);
    const float ForwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(ForwardCosAngle));

    const float RightCosAngle = FVector::DotProduct(RightVector, NormalVelocity);
    return RightCosAngle < 0.0f ? -ForwardDeltaDegree : ForwardDeltaDegree;
}

// ── Blueprint Callable Methods ───────────────────────────────────────────────

void UTranspersonalAnimInstance::TriggerAttackMontage()
{
    bIsAttacking = true;
    AttackPlayRate = 1.0f;
}

void UTranspersonalAnimInstance::TriggerDeathMontage()
{
    bIsAlive = false;
    LocomotionState = EAnim_LocomotionState::Death;
}

void UTranspersonalAnimInstance::SetAttackState(bool bAttacking)
{
    bIsAttacking = bAttacking;
}

float UTranspersonalAnimInstance::GetMovementSpeed() const
{
    return LocomotionData.Speed;
}

float UTranspersonalAnimInstance::GetMovementDirection() const
{
    return LocomotionData.Direction;
}

bool UTranspersonalAnimInstance::GetIsMoving() const
{
    return LocomotionData.bIsMoving;
}

bool UTranspersonalAnimInstance::GetIsInAir() const
{
    return LocomotionData.bIsInAir;
}

EAnim_LocomotionState UTranspersonalAnimInstance::GetLocomotionState() const
{
    return LocomotionState;
}
