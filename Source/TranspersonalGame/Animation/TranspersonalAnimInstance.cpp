#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    LocomotionState = EAnim_LocomotionState::Idle;
    UpperBodyState = EAnim_UpperBodyState::None;
    FootIKTraceDistance = 55.0f;
    FootIKInterpSpeed = 15.0f;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
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

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        // Re-attempt to get owner on first valid frame
        APawn* Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            OwnerCharacter = Cast<ACharacter>(Pawn);
            if (OwnerCharacter)
            {
                MovementComponent = OwnerCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    UpdateLocomotionState(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateSurvivalState();
    UpdateBlendWeights(DeltaSeconds);
}

void UTranspersonalAnimInstance::NativePostEvaluateAnimation()
{
    Super::NativePostEvaluateAnimation();
    // Post-evaluate hook — available for procedural bone adjustments
}

// ─── Locomotion ──────────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    // Ground speed (XY plane only)
    FVector Velocity = MovementComponent->Velocity;
    GroundSpeed = Velocity.Size2D();

    // Movement direction relative to actor facing
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector VelocityNorm = Velocity.GetSafeNormal2D();
    if (!VelocityNorm.IsNearlyZero())
    {
        FRotator VelocityRot = VelocityNorm.Rotation();
        MovementDirection = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
    }
    else
    {
        MovementDirection = 0.0f;
    }

    bIsMoving = GroundSpeed > 3.0f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Detect sprint: speed above run threshold
    float MaxWalkSpeed = MovementComponent->MaxWalkSpeed;
    bIsSprinting = GroundSpeed > (MaxWalkSpeed * 0.85f);

    // Aim pitch/yaw from control rotation
    AController* Controller = OwnerCharacter->GetController();
    if (Controller)
    {
        FRotator ControlRot = Controller->GetControlRotation();
        FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
        AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
        AimYaw = FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f);
    }

    // Land detection
    if (bWasInAir && !bIsInAir)
    {
        LandRecoveryTimer = 0.35f; // 350ms land recovery
    }
    bWasInAir = bIsInAir;

    if (LandRecoveryTimer > 0.0f)
    {
        LandRecoveryTimer -= DeltaSeconds;
    }

    // Update locomotion data struct
    LocomotionData.Speed = GroundSpeed;
    LocomotionData.Direction = MovementDirection;
    LocomotionData.bIsInAir = bIsInAir;
    LocomotionData.bIsCrouching = bIsCrouching;
    LocomotionData.bIsSprinting = bIsSprinting;
    LocomotionData.AimPitch = AimPitch;
    LocomotionData.AimYaw = AimYaw;

    // Determine state
    LocomotionState = DetermineLocomotionState();

    PreviousSpeed = GroundSpeed;
}

EAnim_LocomotionState UTranspersonalAnimInstance::DetermineLocomotionState() const
{
    if (bIsInAir)
    {
        FVector Vel = MovementComponent ? MovementComponent->Velocity : FVector::ZeroVector;
        return (Vel.Z < -100.0f) ? EAnim_LocomotionState::Fall : EAnim_LocomotionState::Jump;
    }

    if (LandRecoveryTimer > 0.0f)
    {
        return EAnim_LocomotionState::Land;
    }

    if (bIsCrouching)
    {
        return EAnim_LocomotionState::Crouch;
    }

    if (!bIsMoving)
    {
        return EAnim_LocomotionState::Idle;
    }

    if (bIsSprinting)
    {
        return EAnim_LocomotionState::Sprint;
    }

    // Walk vs Run threshold: 200 cm/s
    return (GroundSpeed > 200.0f) ? EAnim_LocomotionState::Run : EAnim_LocomotionState::Walk;
}

// ─── Foot IK ─────────────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir) return;

    FVector LeftLoc = FootIKData.LeftFootLocation;
    FVector RightLoc = FootIKData.RightFootLocation;
    FRotator LeftRot = FootIKData.LeftFootRotation;
    FRotator RightRot = FootIKData.RightFootRotation;

    FVector TargetLeftLoc, TargetRightLoc;
    FRotator TargetLeftRot, TargetRightRot;

    bool bLeftHit = TraceFootIK(FName("foot_l"), TargetLeftLoc, TargetLeftRot);
    bool bRightHit = TraceFootIK(FName("foot_r"), TargetRightLoc, TargetRightRot);

    float InterpAlpha = FMath::Clamp(FootIKInterpSpeed * DeltaSeconds, 0.0f, 1.0f);

    if (bLeftHit)
    {
        FootIKData.LeftFootLocation = FMath::VInterpTo(LeftLoc, TargetLeftLoc, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.LeftFootRotation = FMath::RInterpTo(LeftRot, TargetLeftRot, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    if (bRightHit)
    {
        FootIKData.RightFootLocation = FMath::VInterpTo(RightLoc, TargetRightLoc, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootRotation = FMath::RInterpTo(RightRot, TargetRightRot, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    // Pelvis offset: lower pelvis when one foot is significantly lower
    float LeftZ = FootIKData.LeftFootLocation.Z;
    float RightZ = FootIKData.RightFootLocation.Z;
    float LowestFoot = FMath::Min(LeftZ, RightZ);
    float ActorZ = OwnerCharacter->GetActorLocation().Z;
    float TargetPelvisOffset = FMath::Clamp(LowestFoot - ActorZ, -30.0f, 0.0f);
    FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, TargetPelvisOffset, DeltaSeconds, FootIKInterpSpeed);
}

bool UTranspersonalAnimInstance::TraceFootIK(const FName& FootBoneName, FVector& OutLocation, FRotator& OutRotation)
{
    if (!OwnerCharacter) return false;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return false;

    FVector BoneLocation = Mesh->GetBoneLocation(FootBoneName);
    FVector TraceStart = BoneLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
    FVector TraceEnd = BoneLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return false;

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        OutLocation = HitResult.ImpactPoint;

        // Calculate foot rotation from surface normal
        FVector SurfaceNormal = HitResult.ImpactNormal;
        FRotator SurfaceRot = UKismetMathLibrary::MakeRotFromZX(SurfaceNormal, OwnerCharacter->GetActorForwardVector());
        OutRotation = SurfaceRot;
        return true;
    }

    return false;
}

// ─── Survival State ──────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateSurvivalState()
{
    // Try to get survival stats from the character
    // These are driven by TranspersonalCharacter survival properties
    if (!OwnerCharacter) return;

    // Read health/stamina via reflection if available
    // Default: full health unless character exposes these
    float* HealthPtr = nullptr;
    float* StaminaPtr = nullptr;

    // Use UObject property system to read survival stats without hard dependency
    UClass* CharClass = OwnerCharacter->GetClass();
    if (CharClass)
    {
        FFloatProperty* HealthProp = FindFProperty<FFloatProperty>(CharClass, TEXT("Health"));
        FFloatProperty* MaxHealthProp = FindFProperty<FFloatProperty>(CharClass, TEXT("MaxHealth"));
        FFloatProperty* StaminaProp = FindFProperty<FFloatProperty>(CharClass, TEXT("Stamina"));
        FFloatProperty* MaxStaminaProp = FindFProperty<FFloatProperty>(CharClass, TEXT("MaxStamina"));

        if (HealthProp && MaxHealthProp)
        {
            float Health = HealthProp->GetPropertyValue_InContainer(OwnerCharacter);
            float MaxHealth = MaxHealthProp->GetPropertyValue_InContainer(OwnerCharacter);
            if (MaxHealth > 0.0f)
            {
                HealthPercent = FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f);
            }
        }

        if (StaminaProp && MaxStaminaProp)
        {
            float Stamina = StaminaProp->GetPropertyValue_InContainer(OwnerCharacter);
            float MaxStamina = MaxStaminaProp->GetPropertyValue_InContainer(OwnerCharacter);
            if (MaxStamina > 0.0f)
            {
                StaminaPercent = FMath::Clamp(Stamina / MaxStamina, 0.0f, 1.0f);
            }
        }
    }

    bIsInjured = HealthPercent < 0.35f;
    bIsExhausted = StaminaPercent < 0.15f;

    // Injured upper body override
    if (bIsInjured && UpperBodyState == EAnim_UpperBodyState::None)
    {
        UpperBodyState = EAnim_UpperBodyState::Injured;
    }
    else if (!bIsInjured && UpperBodyState == EAnim_UpperBodyState::Injured)
    {
        UpperBodyState = EAnim_UpperBodyState::None;
    }
}

// ─── Blend Weights ───────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateBlendWeights(float DeltaSeconds)
{
    // Walk/Run blend: 0=walk, 1=run, based on speed
    float MaxWalkSpeed = MovementComponent ? MovementComponent->MaxWalkSpeed : 400.0f;
    float RunThreshold = MaxWalkSpeed * 0.5f;
    float TargetWalkRunBlend = bIsMoving ? FMath::Clamp((GroundSpeed - 100.0f) / (MaxWalkSpeed - 100.0f), 0.0f, 1.0f) : 0.0f;
    WalkRunBlend = FMath::FInterpTo(WalkRunBlend, TargetWalkRunBlend, DeltaSeconds, 5.0f);

    // Injured blend weight
    float TargetInjuredWeight = bIsInjured ? 1.0f : 0.0f;
    InjuredBlendWeight = FMath::FInterpTo(InjuredBlendWeight, TargetInjuredWeight, DeltaSeconds, 3.0f);

    // Land recovery alpha
    float TargetLandAlpha = (LandRecoveryTimer > 0.0f) ? 1.0f : 0.0f;
    LandRecoveryAlpha = FMath::FInterpTo(LandRecoveryAlpha, TargetLandAlpha, DeltaSeconds, 8.0f);
}

// ─── Blueprint Helpers ───────────────────────────────────────────────────────

void UTranspersonalAnimInstance::SetUpperBodyState(EAnim_UpperBodyState NewState)
{
    UpperBodyState = NewState;
}

bool UTranspersonalAnimInstance::ShouldPlayLandAnimation() const
{
    return LandRecoveryTimer > 0.0f;
}

float UTranspersonalAnimInstance::GetLocomotionPlayRate() const
{
    // Scale animation playback rate with speed for natural movement
    if (GroundSpeed < 1.0f) return 1.0f;

    float BaseSpeed = 150.0f; // Reference walk speed
    float PlayRate = GroundSpeed / BaseSpeed;
    return FMath::Clamp(PlayRate, 0.5f, 2.5f);
}
