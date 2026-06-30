// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full AnimInstance implementation: locomotion state machine, foot IK, blendspace logic

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
    Speed = 0.0f;
    Direction = 0.0f;
    LeanAngle = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsMoving = false;
    bIsAccelerating = false;

    // State defaults
    LocomotionState = EAnim_LocomotionState::Idle;
    WeaponState = EAnim_WeaponState::Unarmed;

    // IK defaults
    FootIK_Left = FAnim_FootIKData();
    FootIK_Right = FAnim_FootIKData();
    PelvisOffset = FVector::ZeroVector;

    // Survival defaults
    StaminaRatio = 1.0f;
    FearLevel = 0.0f;
    bIsExhausted = false;
    bIsInjured = false;

    // Blendspace defaults
    BlendSpace_Speed = 0.0f;
    BlendSpace_Direction = 0.0f;
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    // Transition defaults
    LandingImpact = 0.0f;
    bJustLanded = false;
    bJustJumped = false;
    TimeInAir = 0.0f;

    // IK config
    FootIKTraceLength = 50.0f;
    FootIKInterpSpeed = 15.0f;
    PelvisIKInterpSpeed = 10.0f;

    // Internal
    PreviousSpeed = 0.0f;
    PreviousLocomotionState = EAnim_LocomotionState::Idle;
    bWasInAir = false;
    TimeInCurrentState = 0.0f;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache owner character
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        OwnerMovement = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("PlayerAnimInstance: Initialized for %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerAnimInstance: No owner character found at init"));
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !OwnerMovement)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (OwnerCharacter)
        {
            OwnerMovement = OwnerCharacter->GetCharacterMovement();
        }
        else
        {
            return;
        }
    }

    // Store previous state
    PreviousSpeed = Speed;
    PreviousLocomotionState = LocomotionState;
    bWasInAir = bIsInAir;

    // Update all systems
    UpdateLocomotionData(DeltaSeconds);
    UpdateAimData(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateSurvivalAnimState(DeltaSeconds);
    DetermineLocomotionState();
    UpdateBlendSpaceValues(DeltaSeconds);
    UpdateTransitionFlags(DeltaSeconds);

    // Track time in current state
    if (LocomotionState == PreviousLocomotionState)
    {
        TimeInCurrentState += DeltaSeconds;
    }
    else
    {
        TimeInCurrentState = 0.0f;
    }
}

void UPlayerAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    if (!OwnerCharacter || !OwnerMovement) return;

    // Velocity and speed
    FVector Velocity = OwnerMovement->Velocity;
    Speed = Velocity.Size2D();

    // Direction relative to character facing
    FRotator CharRotation = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = CharRotation.UnrotateVector(Velocity);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    // Movement flags
    bIsInAir = OwnerMovement->IsFalling();
    bIsCrouching = OwnerMovement->IsCrouching();
    bIsMoving = Speed > 10.0f;
    bIsAccelerating = OwnerMovement->GetCurrentAcceleration().Size() > 0.0f;

    // Lean angle — based on angular velocity
    float AngularVelocityZ = OwnerCharacter->GetMesh() ? 
        OwnerCharacter->GetMesh()->GetPhysicsAngularVelocityInDegrees().Z : 0.0f;
    LeanAngle = FMath::FInterpTo(LeanAngle, FMath::Clamp(AngularVelocityZ * 0.5f, -45.0f, 45.0f), DeltaSeconds, 5.0f);

    // Sprint detection — speed threshold
    bIsSprinting = Speed > 500.0f && bIsMoving && !bIsCrouching && !bIsInAir;

    // Time in air tracking
    if (bIsInAir)
    {
        TimeInAir += DeltaSeconds;
    }
    else
    {
        TimeInAir = 0.0f;
    }
}

void UPlayerAnimInstance::UpdateAimData(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRotation = Controller->GetControlRotation();
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();

    // Delta between aim and body
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);

    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -180.0f, 180.0f);
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;
    if (bIsInAir) 
    {
        // Disable IK in air — lerp back to zero
        FootIK_Left.IKAlpha = FMath::FInterpTo(FootIK_Left.IKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        FootIK_Right.IKAlpha = FMath::FInterpTo(FootIK_Right.IKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        PelvisOffset = FMath::VInterpTo(PelvisOffset, FVector::ZeroVector, DeltaSeconds, PelvisIKInterpSpeed);
        return;
    }

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Trace for left foot
    FVector LeftFootBone = Mesh->GetSocketLocation(FName("foot_l"));
    FVector RightFootBone = Mesh->GetSocketLocation(FName("foot_r"));

    FVector TraceStart_L = LeftFootBone + FVector(0, 0, FootIKTraceLength);
    FVector TraceEnd_L = LeftFootBone - FVector(0, 0, FootIKTraceLength);
    FVector TraceStart_R = RightFootBone + FVector(0, 0, FootIKTraceLength);
    FVector TraceEnd_R = RightFootBone - FVector(0, 0, FootIKTraceLength);

    FHitResult HitLeft, HitRight;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    bool bHitLeft = World->LineTraceSingleByChannel(HitLeft, TraceStart_L, TraceEnd_L, ECC_Visibility, Params);
    bool bHitRight = World->LineTraceSingleByChannel(HitRight, TraceStart_R, TraceEnd_R, ECC_Visibility, Params);

    // Update left foot IK
    if (bHitLeft)
    {
        FVector TargetLocation = HitLeft.ImpactPoint;
        FRotator TargetRotation = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(HitLeft.ImpactNormal.X, HitLeft.ImpactNormal.Z)) * -1.0f,
            0.0f,
            FMath::RadiansToDegrees(FMath::Atan2(HitLeft.ImpactNormal.Y, HitLeft.ImpactNormal.Z))
        );
        FootIK_Left.FootLocation = FMath::VInterpTo(FootIK_Left.FootLocation, TargetLocation, DeltaSeconds, FootIKInterpSpeed);
        FootIK_Left.FootRotation = FMath::RInterpTo(FootIK_Left.FootRotation, TargetRotation, DeltaSeconds, FootIKInterpSpeed);
        FootIK_Left.IKAlpha = FMath::FInterpTo(FootIK_Left.IKAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        FootIK_Left.IKAlpha = FMath::FInterpTo(FootIK_Left.IKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    // Update right foot IK
    if (bHitRight)
    {
        FVector TargetLocation = HitRight.ImpactPoint;
        FRotator TargetRotation = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(HitRight.ImpactNormal.X, HitRight.ImpactNormal.Z)) * -1.0f,
            0.0f,
            FMath::RadiansToDegrees(FMath::Atan2(HitRight.ImpactNormal.Y, HitRight.ImpactNormal.Z))
        );
        FootIK_Right.FootLocation = FMath::VInterpTo(FootIK_Right.FootLocation, TargetLocation, DeltaSeconds, FootIKInterpSpeed);
        FootIK_Right.FootRotation = FMath::RInterpTo(FootIK_Right.FootRotation, TargetRotation, DeltaSeconds, FootIKInterpSpeed);
        FootIK_Right.IKAlpha = FMath::FInterpTo(FootIK_Right.IKAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        FootIK_Right.IKAlpha = FMath::FInterpTo(FootIK_Right.IKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    // Pelvis offset — lower pelvis to accommodate lower foot
    if (bHitLeft || bHitRight)
    {
        float LeftDiff = bHitLeft ? (HitLeft.ImpactPoint.Z - LeftFootBone.Z) : 0.0f;
        float RightDiff = bHitRight ? (HitRight.ImpactPoint.Z - RightFootBone.Z) : 0.0f;
        float PelvisZ = FMath::Min(LeftDiff, RightDiff);
        FVector TargetPelvis = FVector(0.0f, 0.0f, FMath::Clamp(PelvisZ, -20.0f, 0.0f));
        PelvisOffset = FMath::VInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, PelvisIKInterpSpeed);
    }
}

void UPlayerAnimInstance::UpdateSurvivalAnimState(float DeltaSeconds)
{
    // These values are fed from TranspersonalCharacter survival stats
    // Exhaustion affects movement animation speed multiplier
    bIsExhausted = StaminaRatio < 0.15f;

    // Fear affects posture — high fear = hunched, defensive
    // Injury affects gait — limp when injured
}

void UPlayerAnimInstance::DetermineLocomotionState()
{
    if (bIsInAir)
    {
        if (bJustJumped || TimeInAir < 0.3f)
            LocomotionState = EAnim_LocomotionState::InAir;
        else
            LocomotionState = EAnim_LocomotionState::InAir;
        return;
    }

    if (bJustLanded)
    {
        LocomotionState = EAnim_LocomotionState::Land;
        return;
    }

    if (!bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = (Speed > 80.0f) ? EAnim_LocomotionState::Sneak : EAnim_LocomotionState::Crouch;
        return;
    }

    if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
        return;
    }

    if (Speed > 250.0f)
    {
        LocomotionState = EAnim_LocomotionState::Run;
        return;
    }

    LocomotionState = EAnim_LocomotionState::Walk;
}

void UPlayerAnimInstance::UpdateBlendSpaceValues(float DeltaSeconds)
{
    // Smooth blendspace speed — prevents snapping
    BlendSpace_Speed = FMath::FInterpTo(BlendSpace_Speed, Speed, DeltaSeconds, 8.0f);

    // Direction for strafe blendspace
    BlendSpace_Direction = FMath::FInterpTo(BlendSpace_Direction, Direction, DeltaSeconds, 10.0f);
}

void UPlayerAnimInstance::UpdateTransitionFlags(float DeltaSeconds)
{
    // Landing detection
    if (bWasInAir && !bIsInAir)
    {
        bJustLanded = true;
        LandingImpact = FMath::Clamp(TimeInAir * 0.5f, 0.0f, 1.0f);
    }
    else
    {
        bJustLanded = false;
        LandingImpact = FMath::FInterpTo(LandingImpact, 0.0f, DeltaSeconds, 3.0f);
    }

    // Jump detection
    if (!bWasInAir && bIsInAir)
    {
        bJustJumped = true;
    }
    else
    {
        bJustJumped = false;
    }
}

void UPlayerAnimInstance::SetWeaponState(EAnim_WeaponState NewState)
{
    WeaponState = NewState;
}

void UPlayerAnimInstance::SetSurvivalStats(float Stamina, float Fear, bool bInjured)
{
    StaminaRatio = FMath::Clamp(Stamina, 0.0f, 1.0f);
    FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);
    bIsInjured = bInjured;
}

float UPlayerAnimInstance::GetLocomotionBlendWeight() const
{
    // Returns 0-1 weight for locomotion blend: 0=idle, 1=full sprint
    return FMath::Clamp(BlendSpace_Speed / 600.0f, 0.0f, 1.0f);
}

bool UPlayerAnimInstance::ShouldApplyFootIK() const
{
    return !bIsInAir && LocomotionState != EAnim_LocomotionState::InAir && LocomotionState != EAnim_LocomotionState::Land;
}
