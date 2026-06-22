#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    FootIKTraceDistance = 80.0f;
    FootIKInterpSpeed = 15.0f;
    AimInterpSpeed = 10.0f;
    WalkSpeedThreshold = 10.0f;
    RunSpeedThreshold = 250.0f;
    SprintSpeedThreshold = 450.0f;
    bEnableFootIK = true;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
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

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        // Re-attempt cache on first valid frame
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
    UpdateSurvivalPosture(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateLean(DeltaSeconds);

    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaSeconds);
    }
}

void UDinoSurvivorAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
    // Thread-safe reads only — locomotion data already updated in NativeUpdateAnimation
}

// ─── Locomotion ───────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    // Ground speed (XY plane only)
    FVector Velocity = MovementComponent->Velocity;
    GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
    VerticalVelocity = Velocity.Z;

    bIsMoving = GroundSpeed > WalkSpeedThreshold;
    bIsCrouching = MovementComponent->IsCrouching();
    bIsInAir = MovementComponent->IsFalling();

    // Sprint detection — check max walk speed vs current
    float MaxWalkSpeed = MovementComponent->MaxWalkSpeed;
    bIsSprinting = bIsMoving && (MaxWalkSpeed >= SprintSpeedThreshold) && !bIsCrouching;

    // Movement direction (relative to character facing)
    if (bIsMoving)
    {
        FRotator CharacterRotation = OwnerCharacter->GetActorRotation();
        FRotator VelocityRotation = Velocity.Rotation();
        MovementDirection = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, CharacterRotation).Yaw;
    }
    else
    {
        MovementDirection = 0.0f;
    }

    // State machine
    if (bIsInAir)
    {
        LocomotionState = (VerticalVelocity > 0.0f) ? EAnim_LocomotionState::Jump : EAnim_LocomotionState::Fall;
    }
    else if (bIsCrouching)
    {
        LocomotionState = bIsMoving ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (GroundSpeed > RunSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else if (GroundSpeed > WalkSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
}

// ─── Foot IK ─────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir) return;

    FVector LeftLoc, RightLoc;
    FRotator LeftRot, RightRot;

    bool bLeftHit = TraceFootIK(FName("foot_l"), LeftLoc, LeftRot);
    bool bRightHit = TraceFootIK(FName("foot_r"), RightLoc, RightRot);

    // Interpolate foot locations
    CurrentLeftFootLoc = FMath::VInterpTo(CurrentLeftFootLoc, LeftLoc, DeltaSeconds, FootIKInterpSpeed);
    CurrentRightFootLoc = FMath::VInterpTo(CurrentRightFootLoc, RightLoc, DeltaSeconds, FootIKInterpSpeed);

    FootIKData.LeftFootLocation = CurrentLeftFootLoc;
    FootIKData.RightFootLocation = CurrentRightFootLoc;
    FootIKData.LeftFootRotation = LeftRot;
    FootIKData.RightFootRotation = RightRot;
    FootIKData.LeftFootAlpha = bLeftHit ? 1.0f : 0.0f;
    FootIKData.RightFootAlpha = bRightHit ? 1.0f : 0.0f;

    // Pelvis offset — lower pelvis to the lowest foot
    float LeftDelta = LeftLoc.Z - OwnerCharacter->GetActorLocation().Z;
    float RightDelta = RightLoc.Z - OwnerCharacter->GetActorLocation().Z;
    float TargetPelvisOffset = FMath::Min(LeftDelta, RightDelta);
    TargetPelvisOffset = FMath::Clamp(TargetPelvisOffset, -40.0f, 0.0f);

    CurrentPelvisOffset = FMath::FInterpTo(CurrentPelvisOffset, TargetPelvisOffset, DeltaSeconds, FootIKInterpSpeed);
    FootIKData.PelvisOffset = CurrentPelvisOffset;
}

bool UDinoSurvivorAnimInstance::TraceFootIK(const FName& SocketName, FVector& OutLocation, FRotator& OutRotation)
{
    if (!OwnerCharacter) return false;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return false;

    FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
    FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return false;

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
        // Align foot rotation to surface normal
        FVector Normal = HitResult.ImpactNormal;
        OutRotation = FRotationMatrix::MakeFromZX(Normal, OwnerCharacter->GetActorForwardVector()).Rotator();
        return true;
    }

    OutLocation = SocketLocation;
    OutRotation = FRotator::ZeroRotator;
    return false;
}

// ─── Aim Offset ───────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRotation = Controller->GetControlRotation();
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);

    float TargetYaw = FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f);
    float TargetPitch = FMath::Clamp(ControlRotation.Pitch, -90.0f, 90.0f);

    AimOffsetData.AimYaw = FMath::FInterpTo(AimOffsetData.AimYaw, TargetYaw, DeltaSeconds, AimInterpSpeed);
    AimOffsetData.AimPitch = FMath::FInterpTo(AimOffsetData.AimPitch, TargetPitch, DeltaSeconds, AimInterpSpeed);
    AimOffsetData.AimAlpha = 1.0f;
}

// ─── Survival Posture ─────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateSurvivalPosture(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Try to read survival stats from the character
    // TranspersonalCharacter exposes these as UPROPERTY — access via reflection
    // Fallback to defaults if not available
    float Health = 1.0f;
    float Stamina = 1.0f;
    float Fear = 0.0f;

    // Attempt to read via UObject property reflection
    UClass* CharClass = OwnerCharacter->GetClass();
    if (CharClass)
    {
        FFloatProperty* HealthProp = FindFieldChecked<FFloatProperty>(CharClass, FName("Health"));
        if (HealthProp)
        {
            float RawHealth = HealthProp->GetPropertyValue_InContainer(OwnerCharacter);
            Health = FMath::Clamp(RawHealth / 100.0f, 0.0f, 1.0f);
        }

        FFloatProperty* StaminaProp = FindFProperty<FFloatProperty>(CharClass, FName("Stamina"));
        if (StaminaProp)
        {
            float RawStamina = StaminaProp->GetPropertyValue_InContainer(OwnerCharacter);
            Stamina = FMath::Clamp(RawStamina / 100.0f, 0.0f, 1.0f);
        }

        FFloatProperty* FearProp = FindFProperty<FFloatProperty>(CharClass, FName("Fear"));
        if (FearProp)
        {
            float RawFear = FearProp->GetPropertyValue_InContainer(OwnerCharacter);
            Fear = FMath::Clamp(RawFear / 100.0f, 0.0f, 1.0f);
        }
    }

    HealthNormalized = Health;
    StaminaNormalized = Stamina;
    FearIntensity = Fear;

    // Determine posture from survival stats
    if (Fear > 0.7f)
    {
        SurvivalPosture = EAnim_SurvivalPosture::Terrified;
        SurvivalBlendWeight = Fear;
    }
    else if (Health < 0.25f)
    {
        SurvivalPosture = EAnim_SurvivalPosture::Injured;
        SurvivalBlendWeight = 1.0f - Health;
    }
    else if (Stamina < 0.1f)
    {
        SurvivalPosture = EAnim_SurvivalPosture::Exhausted;
        SurvivalBlendWeight = 1.0f - Stamina;
    }
    else if (Stamina < 0.4f)
    {
        SurvivalPosture = EAnim_SurvivalPosture::Tired;
        SurvivalBlendWeight = (0.4f - Stamina) / 0.4f;
    }
    else
    {
        SurvivalPosture = EAnim_SurvivalPosture::Healthy;
        SurvivalBlendWeight = 0.0f;
    }
}

// ─── Lean ─────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    FVector CurrentVelocity = MovementComponent->Velocity;
    FVector VelocityDelta = CurrentVelocity - PreviousVelocity;
    PreviousVelocity = CurrentVelocity;

    // Project delta onto character right axis for lateral lean
    FVector RightAxis = OwnerCharacter->GetActorRightVector();
    float LateralAccel = FVector::DotProduct(VelocityDelta, RightAxis);

    float TargetLean = FMath::Clamp(LateralAccel * 0.05f, -15.0f, 15.0f);
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 8.0f);
}
