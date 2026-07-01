// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260701_004
// Full implementation of locomotion state machine, foot IK, and lean system

#include "PlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Locomotion defaults
    LocomotionState = EAnim_LocomotionState::Idle;
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsMoving = false;

    // Speed thresholds
    WalkSpeedThreshold = 10.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 550.0f;

    // IK defaults
    FootIKTraceDistance = 55.0f;
    FootIKInterpSpeed = 15.0f;
    bFootIKEnabled = true;

    // Lean defaults
    LeanInterpSpeed = 5.0f;

    // Jump/land
    JumpVelocityZ = 0.0f;
    bJustLanded = false;
    LandImpactAlpha = 0.0f;

    // Aim offset
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    // Internal
    OwnerCharacter = nullptr;
    OwnerMovement = nullptr;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    OwnerCharacter = Cast<ACharacter>(Pawn);
    if (OwnerCharacter)
    {
        OwnerMovement = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("PlayerAnimInstance: Initialized for %s"), *OwnerCharacter->GetName());
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !OwnerMovement) return;

    UpdateLocomotionData(DeltaSeconds);
    UpdateLocomotionState();
    UpdateFootIK(DeltaSeconds);
    UpdateLean(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateJumpLand(DeltaSeconds);
}

void UPlayerAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    if (!OwnerCharacter || !OwnerMovement) return;

    FVector Velocity = OwnerMovement->Velocity;
    Speed = Velocity.Size2D();
    bIsInAir = OwnerMovement->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsMoving = Speed > WalkSpeedThreshold;

    // Direction relative to actor forward
    if (bIsMoving)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
        Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
    }
    else
    {
        Direction = 0.0f;
    }

    // Vertical velocity for jump/fall
    JumpVelocityZ = Velocity.Z;
}

void UPlayerAnimInstance::UpdateLocomotionState()
{
    if (!OwnerMovement) return;

    EAnim_LocomotionState NewState = LocomotionState;

    if (OwnerMovement->MovementMode == EMovementMode::MOVE_None)
    {
        NewState = EAnim_LocomotionState::Dead;
    }
    else if (bIsInAir)
    {
        if (JumpVelocityZ > 0.0f)
            NewState = EAnim_LocomotionState::Jump;
        else
            NewState = EAnim_LocomotionState::Fall;
    }
    else if (bIsCrouching)
    {
        NewState = bIsMoving ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    }
    else if (Speed >= SprintSpeedThreshold)
    {
        NewState = EAnim_LocomotionState::Sprint;
        bIsSprinting = true;
    }
    else if (Speed >= RunSpeedThreshold)
    {
        NewState = EAnim_LocomotionState::Run;
        bIsSprinting = false;
    }
    else if (Speed >= WalkSpeedThreshold)
    {
        NewState = EAnim_LocomotionState::Walk;
        bIsSprinting = false;
    }
    else
    {
        NewState = EAnim_LocomotionState::Idle;
        bIsSprinting = false;
    }

    LocomotionState = NewState;
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!bFootIKEnabled || !OwnerCharacter) return;

    UpdateSingleFootIK(LeftFootIK, FName("foot_l"), DeltaSeconds);
    UpdateSingleFootIK(RightFootIK, FName("foot_r"), DeltaSeconds);
}

void UPlayerAnimInstance::UpdateSingleFootIK(FAnim_FootIKData& FootData, FName SocketName, float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
    FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

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
        // Target IK location
        FVector TargetLocation = HitResult.ImpactPoint;
        FVector TargetNormal = HitResult.ImpactNormal;

        // Smooth interpolation
        FootData.IKLocation = FMath::VInterpTo(
            FootData.IKLocation,
            TargetLocation,
            DeltaSeconds,
            FootIKInterpSpeed
        );

        // Compute foot rotation from surface normal
        FRotator TargetRotation = FRotationMatrix::MakeFromZX(TargetNormal, OwnerCharacter->GetActorForwardVector()).Rotator();
        FootData.IKRotation = FMath::RInterpTo(
            FootData.IKRotation,
            TargetRotation,
            DeltaSeconds,
            FootIKInterpSpeed
        );

        FootData.bGroundContact = true;
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        FootData.bGroundContact = false;
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }
}

void UPlayerAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter || !OwnerMovement) return;

    FVector Acceleration = OwnerMovement->GetCurrentAcceleration();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector LocalAccel = ActorRot.UnrotateVector(Acceleration);

    // Normalize to -1..1 range based on max acceleration
    float MaxAccel = OwnerMovement->GetMaxAcceleration();
    if (MaxAccel > 0.0f)
    {
        FAnim_LeanData TargetLean;
        TargetLean.LeanForwardBack = FMath::Clamp(LocalAccel.X / MaxAccel, -1.0f, 1.0f);
        TargetLean.LeanLeftRight = FMath::Clamp(LocalAccel.Y / MaxAccel, -1.0f, 1.0f);

        LeanData.LeanForwardBack = FMath::FInterpTo(LeanData.LeanForwardBack, TargetLean.LeanForwardBack, DeltaSeconds, LeanInterpSpeed);
        LeanData.LeanLeftRight = FMath::FInterpTo(LeanData.LeanLeftRight, TargetLean.LeanLeftRight, DeltaSeconds, LeanInterpSpeed);
    }
}

void UPlayerAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f);
}

void UPlayerAnimInstance::UpdateJumpLand(float DeltaSeconds)
{
    // Decay land impact alpha
    if (bJustLanded)
    {
        LandImpactAlpha = FMath::FInterpTo(LandImpactAlpha, 0.0f, DeltaSeconds, 3.0f);
        if (LandImpactAlpha < 0.01f)
        {
            bJustLanded = false;
            LandImpactAlpha = 0.0f;
        }
    }
}

void UPlayerAnimInstance::OnLanded(const FHitResult& Hit)
{
    // Called from character on landing
    bJustLanded = true;
    float FallSpeed = FMath::Abs(JumpVelocityZ);
    // Scale impact by fall speed: 600 = light, 1200 = heavy
    LandImpactAlpha = FMath::Clamp(FallSpeed / 1200.0f, 0.2f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("PlayerAnimInstance: Landed with impact alpha %.2f (fall speed %.0f)"), LandImpactAlpha, FallSpeed);
}

FString UPlayerAnimInstance::GetLocomotionStateName() const
{
    switch (LocomotionState)
    {
        case EAnim_LocomotionState::Idle:        return TEXT("Idle");
        case EAnim_LocomotionState::Walk:        return TEXT("Walk");
        case EAnim_LocomotionState::Run:         return TEXT("Run");
        case EAnim_LocomotionState::Sprint:      return TEXT("Sprint");
        case EAnim_LocomotionState::Jump:        return TEXT("Jump");
        case EAnim_LocomotionState::Fall:        return TEXT("Fall");
        case EAnim_LocomotionState::Land:        return TEXT("Land");
        case EAnim_LocomotionState::Crouch:      return TEXT("Crouch");
        case EAnim_LocomotionState::CrouchWalk:  return TEXT("CrouchWalk");
        case EAnim_LocomotionState::Dead:        return TEXT("Dead");
        default:                                 return TEXT("Unknown");
    }
}
