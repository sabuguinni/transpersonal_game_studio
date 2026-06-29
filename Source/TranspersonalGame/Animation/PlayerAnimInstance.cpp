#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Defaults set in header via member initializers
}

// ============================================================
// UAnimInstance overrides
// ============================================================

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* PawnOwner = TryGetPawnOwner();
    if (PawnOwner)
    {
        OwnerCharacter = Cast<ACharacter>(PawnOwner);
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UPlayerAnimInstance::NativeBeginPlay()
{
    Super::NativeBeginPlay();

    // Re-cache in case InitializeAnimation ran before BeginPlay
    if (!OwnerCharacter)
    {
        APawn* PawnOwner = TryGetPawnOwner();
        if (PawnOwner)
        {
            OwnerCharacter = Cast<ACharacter>(PawnOwner);
            if (OwnerCharacter)
            {
                MovementComponent = OwnerCharacter->GetCharacterMovement();
            }
        }
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent || DeltaSeconds <= 0.0f)
    {
        return;
    }

    // Core velocity data
    const FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    VerticalVelocity = Velocity.Z;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsSwimming = MovementComponent->IsSwimming();
    bIsMoving = Speed > WalkSpeedThreshold;

    // Direction relative to actor forward
    if (bIsMoving)
    {
        const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        const FRotator VelocityRotation = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, ActorRotation).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // Update subsystems
    UpdateLocomotionState();
    UpdateCombatState();
    UpdateSurvivalPosture();
    UpdateBlendWeights(DeltaSeconds);
    UpdateLeanData(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateAdditiveLayerWeights(DeltaSeconds);

    PreviousVelocity = Velocity;
}

// ============================================================
// Locomotion state machine
// ============================================================

void UPlayerAnimInstance::UpdateLocomotionState()
{
    if (!MovementComponent) return;

    if (bIsInAir)
    {
        LocomotionState = (VerticalVelocity > 0.0f)
            ? EAnim_PlayerLocomotionState::Jump
            : EAnim_PlayerLocomotionState::Fall;
        return;
    }

    if (bIsSwimming)
    {
        LocomotionState = EAnim_PlayerLocomotionState::Swim;
        return;
    }

    if (bIsClimbing)
    {
        LocomotionState = EAnim_PlayerLocomotionState::Climb;
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = EAnim_PlayerLocomotionState::Crouch;
        return;
    }

    if (Speed >= SprintSpeedThreshold && bIsSprinting)
    {
        LocomotionState = EAnim_PlayerLocomotionState::Sprint;
        return;
    }

    if (Speed >= RunSpeedThreshold)
    {
        LocomotionState = EAnim_PlayerLocomotionState::Run;
        return;
    }

    if (Speed >= WalkSpeedThreshold)
    {
        LocomotionState = EAnim_PlayerLocomotionState::Walk;
        return;
    }

    LocomotionState = EAnim_PlayerLocomotionState::Idle;
}

// ============================================================
// Combat state update
// ============================================================

void UPlayerAnimInstance::UpdateCombatState()
{
    // Combat state is set externally via SetCombatState()
    // Here we update derived blend weights
    WeaponDrawAlpha = (CombatState != EAnim_PlayerCombatState::Unarmed) ? 1.0f : 0.0f;
}

// ============================================================
// Survival posture
// ============================================================

void UPlayerAnimInstance::UpdateSurvivalPosture()
{
    // Determine dominant survival state from stats
    if (HealthNormalized < 0.25f)
    {
        SurvivalState = EAnim_PlayerSurvivalState::Injured;
    }
    else if (StaminaNormalized < 0.15f)
    {
        SurvivalState = EAnim_PlayerSurvivalState::Exhausted;
    }
    else if (HungerNormalized < 0.15f)
    {
        SurvivalState = EAnim_PlayerSurvivalState::Starving;
    }
    else if (ThirstNormalized < 0.15f)
    {
        SurvivalState = EAnim_PlayerSurvivalState::Dehydrated;
    }
    else
    {
        SurvivalState = EAnim_PlayerSurvivalState::Healthy;
    }

    // Survival posture alpha: 0 = healthy upright, 1 = hunched/suffering
    const float HealthFactor = FMath::Clamp(1.0f - HealthNormalized, 0.0f, 1.0f);
    const float StaminaFactor = FMath::Clamp(1.0f - StaminaNormalized, 0.0f, 1.0f) * 0.5f;
    SurvivalPostureAlpha = FMath::Max(HealthFactor, StaminaFactor);
}

// ============================================================
// Blend weights
// ============================================================

void UPlayerAnimInstance::UpdateBlendWeights(float DeltaSeconds)
{
    // Walk/run alpha: 0 = walk, 1 = run
    const float TargetWalkRun = FMath::Clamp(
        (Speed - WalkSpeedThreshold) / (RunSpeedThreshold - WalkSpeedThreshold),
        0.0f, 1.0f);
    WalkRunAlpha = FMath::FInterpTo(WalkRunAlpha, TargetWalkRun, DeltaSeconds, BlendInterpSpeed);

    // Sprint alpha
    const float TargetSprint = bIsSprinting ? 1.0f : 0.0f;
    SprintAlpha = FMath::FInterpTo(SprintAlpha, TargetSprint, DeltaSeconds, BlendInterpSpeed);

    // Grounded/airborne alpha: 0 = grounded, 1 = airborne
    const float TargetAirborne = bIsInAir ? 1.0f : 0.0f;
    GroundedAirborneAlpha = FMath::FInterpTo(GroundedAirborneAlpha, TargetAirborne, DeltaSeconds, BlendInterpSpeed * 2.0f);

    // Crouch alpha
    const float TargetCrouch = bIsCrouching ? 1.0f : 0.0f;
    CrouchAlpha = FMath::FInterpTo(CrouchAlpha, TargetCrouch, DeltaSeconds, BlendInterpSpeed);
}

// ============================================================
// Lean / Aim offset
// ============================================================

void UPlayerAnimInstance::UpdateLeanData(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Acceleration-based lean
    const FVector CurrentVelocity = MovementComponent ? MovementComponent->Velocity : FVector::ZeroVector;
    const FVector Acceleration = (CurrentVelocity - PreviousVelocity) / FMath::Max(DeltaSeconds, 0.001f);

    const FRotator ActorRot = OwnerCharacter->GetActorRotation();
    const FVector LocalAccel = ActorRot.UnrotateVector(Acceleration);

    const float TargetLeanRight = FMath::Clamp(LocalAccel.Y / 600.0f, -1.0f, 1.0f);
    const float TargetLeanForward = FMath::Clamp(LocalAccel.X / 600.0f, -1.0f, 1.0f);

    SmoothedLeanRight = FMath::FInterpTo(SmoothedLeanRight, TargetLeanRight, DeltaSeconds, LeanInterpSpeed);
    SmoothedLeanForward = FMath::FInterpTo(SmoothedLeanForward, TargetLeanForward, DeltaSeconds, LeanInterpSpeed);

    LeanData.LeanRight = SmoothedLeanRight;
    LeanData.LeanForward = SmoothedLeanForward;

    // Aim offset from control rotation
    if (OwnerCharacter->GetController())
    {
        const FRotator ControlRot = OwnerCharacter->GetController()->GetControlRotation();
        const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
        LeanData.AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
        LeanData.AimYaw = FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f);
    }
}

// ============================================================
// Foot IK
// ============================================================

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!bEnableFootIK || !OwnerCharacter || bIsInAir || bIsSwimming)
    {
        FootIKData.LeftFootAlpha = 0.0f;
        FootIKData.RightFootAlpha = 0.0f;
        return;
    }

    FVector LeftLoc, RightLoc;
    FRotator LeftRot, RightRot;

    const bool bLeftHit = TraceFootPosition(FName("foot_l"), LeftLoc, LeftRot);
    const bool bRightHit = TraceFootPosition(FName("foot_r"), RightLoc, RightRot);

    if (bLeftHit)
    {
        SmoothFootIKTarget(SmoothedLeftFoot, LeftLoc, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.LeftFootLocation = SmoothedLeftFoot;
        FootIKData.LeftFootRotation = LeftRot;
        FootIKData.LeftFootAlpha = 1.0f;
    }
    else
    {
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    if (bRightHit)
    {
        SmoothFootIKTarget(SmoothedRightFoot, RightLoc, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootLocation = SmoothedRightFoot;
        FootIKData.RightFootRotation = RightRot;
        FootIKData.RightFootAlpha = 1.0f;
    }
    else
    {
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    // Pelvis offset — lower pelvis to the lowest foot
    const float LowestFoot = FMath::Min(SmoothedLeftFoot.Z, SmoothedRightFoot.Z);
    const float ActorZ = OwnerCharacter->GetActorLocation().Z;
    const float TargetPelvis = FMath::Clamp(LowestFoot - ActorZ, -30.0f, 0.0f);
    SmoothedPelvisOffset = FMath::FInterpTo(SmoothedPelvisOffset, TargetPelvis, DeltaSeconds, FootIKInterpSpeed * 0.5f);
    FootIKData.PelvisOffset = SmoothedPelvisOffset;
}

bool UPlayerAnimInstance::TraceFootPosition(const FName& BoneName, FVector& OutLocation, FRotator& OutRotation) const
{
    if (!OwnerCharacter) return false;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return false;

    const FVector BoneLocation = Mesh->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
    const FVector TraceStart = BoneLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
    const FVector TraceEnd = BoneLocation - FVector(0.0f, 0.0f, FootIKTraceDistance * 2.0f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;

    const bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        OutLocation = HitResult.Location;
        // Compute foot rotation from surface normal
        const FVector SurfaceNormal = HitResult.Normal;
        OutRotation = FRotationMatrix::MakeFromZX(SurfaceNormal, OwnerCharacter->GetActorForwardVector()).Rotator();
        return true;
    }

    return false;
}

void UPlayerAnimInstance::SmoothFootIKTarget(FVector& Current, const FVector& Target, float DeltaSeconds, float InterpSpeed) const
{
    Current = FMath::VInterpTo(Current, Target, DeltaSeconds, InterpSpeed);
}

// ============================================================
// Additive layer weights
// ============================================================

void UPlayerAnimInstance::UpdateAdditiveLayerWeights(float DeltaSeconds)
{
    // Breathing — always present, stronger at rest
    const float TargetBreathing = bIsMoving ? 0.4f : 1.0f;
    BreathingAlpha = FMath::FInterpTo(BreathingAlpha, TargetBreathing, DeltaSeconds, 2.0f);

    // Exhaustion additive — increases when stamina is low
    const float TargetExhaustion = FMath::Clamp(1.0f - StaminaNormalized * 2.0f, 0.0f, 1.0f);
    ExhaustionAdditive = FMath::FInterpTo(ExhaustionAdditive, TargetExhaustion, DeltaSeconds, 3.0f);

    // Injury additive — increases when health is low
    const float TargetInjury = FMath::Clamp(1.0f - HealthNormalized * 2.0f, 0.0f, 1.0f);
    InjuryAdditive = FMath::FInterpTo(InjuryAdditive, TargetInjury, DeltaSeconds, 2.0f);
}

// ============================================================
// Blueprint callable methods
// ============================================================

void UPlayerAnimInstance::SetCombatState(EAnim_PlayerCombatState NewState)
{
    CombatState = NewState;
    bIsAiming = (NewState == EAnim_PlayerCombatState::RangedAim);
    bIsAttacking = (NewState == EAnim_PlayerCombatState::MeleeAttack || NewState == EAnim_PlayerCombatState::RangedFire);
}

void UPlayerAnimInstance::SetSurvivalStats(float Health, float Hunger, float Thirst, float Stamina, float Fear)
{
    HealthNormalized = FMath::Clamp(Health, 0.0f, 1.0f);
    HungerNormalized = FMath::Clamp(Hunger, 0.0f, 1.0f);
    ThirstNormalized = FMath::Clamp(Thirst, 0.0f, 1.0f);
    StaminaNormalized = FMath::Clamp(Stamina, 0.0f, 1.0f);
    FearNormalized = FMath::Clamp(Fear, 0.0f, 1.0f);
}

void UPlayerAnimInstance::TriggerAttackMontage()
{
    SetCombatState(EAnim_PlayerCombatState::MeleeAttack);
    bIsAttacking = true;
}

void UPlayerAnimInstance::TriggerLandMontage()
{
    LocomotionState = EAnim_PlayerLocomotionState::Land;
}

float UPlayerAnimInstance::GetSpeedNormalized() const
{
    return FMath::Clamp(Speed / SprintSpeedThreshold, 0.0f, 1.0f);
}

bool UPlayerAnimInstance::ShouldPlayExhaustionLayer() const
{
    return StaminaNormalized < 0.3f || HealthNormalized < 0.3f;
}
