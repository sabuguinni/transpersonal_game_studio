// DinoSurvivorAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full implementation of the prehistoric survivor AnimInstance.
// Drives locomotion blend space, foot IK, combat stance overlay,
// and survival state modifiers (stamina/fear/health).

#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
    : GroundSpeed(0.0f)
    , LateralDirection(0.0f)
    , bIsMoving(false)
    , bIsInAir(false)
    , bIsCrouching(false)
    , bIsSprinting(false)
    , LocomotionState(EAnim_LocomotionState::Idle)
    , AirState(EAnim_AirState::Grounded)
    , WalkThreshold(10.0f)
    , RunThreshold(300.0f)
    , SprintThreshold(550.0f)
    , LeftFootIKOffset(0.0f)
    , RightFootIKOffset(0.0f)
    , PelvisIKOffset(0.0f)
    , IKAlpha(1.0f)
    , CombatStance(EAnim_CombatStance::Unarmed)
    , bIsAttacking(false)
    , StaminaNormalised(1.0f)
    , FearNormalised(0.0f)
    , HealthNormalised(1.0f)
    , SurvivalLeanAngle(0.0f)
    , AimPitch(0.0f)
    , AimYaw(0.0f)
    , OwnerCharacter(nullptr)
    , MovementComponent(nullptr)
    , PreviousVerticalVelocity(0.0f)
    , TimeInAir(0.0f)
    , SmoothedLeftFootIK(0.0f)
    , SmoothedRightFootIK(0.0f)
    , SmoothedPelvisIK(0.0f)
{
}

// ============================================================
// NativeInitializeAnimation
// ============================================================

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache owner references — safe to call here, owner is valid at init
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

// ============================================================
// NativeUpdateAnimation — master dispatcher
// ============================================================

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    // Guard: skip if no owner (editor preview, etc.)
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Dispatch to subsystems in dependency order
    UpdateLocomotionState(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateCombatState(DeltaSeconds);
    UpdateSurvivalModifiers(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
}

// ============================================================
// UpdateLocomotionState
// ============================================================

void UDinoSurvivorAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    // --- Velocity & speed ---
    const FVector Velocity = MovementComponent->Velocity;
    const FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    GroundSpeed = HorizontalVelocity.Size();

    // --- Lateral direction: dot product with right vector ---
    const FVector RightVector = OwnerCharacter->GetActorRightVector();
    const FVector VelocityNorm = HorizontalVelocity.GetSafeNormal();
    LateralDirection = FVector::DotProduct(VelocityNorm, RightVector);

    // --- Boolean flags ---
    bIsMoving    = GroundSpeed > WalkThreshold;
    bIsInAir     = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsSprinting = GroundSpeed > SprintThreshold;

    // --- Air state machine ---
    const float VerticalVelocity = Velocity.Z;

    if (bIsInAir)
    {
        TimeInAir += DeltaSeconds;

        if (AirState == EAnim_AirState::Grounded)
        {
            // Just left ground
            AirState = (VerticalVelocity > 0.0f)
                ? EAnim_AirState::JumpStart
                : EAnim_AirState::Falling;
        }
        else if (AirState == EAnim_AirState::JumpStart && VerticalVelocity <= 0.0f)
        {
            // Apex reached — start falling
            AirState = EAnim_AirState::Falling;
        }

        // IK fades out while airborne
        IKAlpha = FMath::FInterpTo(IKAlpha, 0.0f, DeltaSeconds, 8.0f);
    }
    else
    {
        if (AirState == EAnim_AirState::Falling || AirState == EAnim_AirState::JumpStart)
        {
            // Just landed
            AirState = EAnim_AirState::Landing;
        }
        else if (AirState == EAnim_AirState::Landing)
        {
            // Landing animation plays — return to grounded after brief window
            AirState = EAnim_AirState::Grounded;
        }

        TimeInAir = 0.0f;
        IKAlpha = FMath::FInterpTo(IKAlpha, 1.0f, DeltaSeconds, 6.0f);
    }

    PreviousVerticalVelocity = VerticalVelocity;

    // --- Locomotion state enum ---
    if (bIsInAir)
    {
        // Keep last ground state while in air — blend space freezes
    }
    else if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouch;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (GroundSpeed > RunThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else if (bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }

    // --- Swim check ---
    if (MovementComponent->IsSwimming())
    {
        LocomotionState = EAnim_LocomotionState::Swim;
    }
}

// ============================================================
// UpdateFootIK
// ============================================================

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Only compute IK when grounded
    if (bIsInAir)
    {
        SmoothedLeftFootIK  = InterpToTarget(SmoothedLeftFootIK,  0.0f, DeltaSeconds, 8.0f);
        SmoothedRightFootIK = InterpToTarget(SmoothedRightFootIK, 0.0f, DeltaSeconds, 8.0f);
        SmoothedPelvisIK    = InterpToTarget(SmoothedPelvisIK,    0.0f, DeltaSeconds, 8.0f);
    }
    else
    {
        // Trace from each foot socket downward
        const float RawLeft  = TraceFootIK(FName("foot_l"), 100.0f);
        const float RawRight = TraceFootIK(FName("foot_r"), 100.0f);

        // Smooth toward raw trace values
        SmoothedLeftFootIK  = InterpToTarget(SmoothedLeftFootIK,  RawLeft,  DeltaSeconds, 15.0f);
        SmoothedRightFootIK = InterpToTarget(SmoothedRightFootIK, RawRight, DeltaSeconds, 15.0f);

        // Pelvis drops to accommodate the lower foot
        const float LowestFoot = FMath::Min(SmoothedLeftFootIK, SmoothedRightFootIK);
        SmoothedPelvisIK = InterpToTarget(SmoothedPelvisIK, LowestFoot, DeltaSeconds, 10.0f);
    }

    LeftFootIKOffset  = SmoothedLeftFootIK;
    RightFootIKOffset = SmoothedRightFootIK;
    PelvisIKOffset    = SmoothedPelvisIK;
}

// ============================================================
// UpdateCombatState
// ============================================================

void UDinoSurvivorAnimInstance::UpdateCombatState(float DeltaSeconds)
{
    // Combat stance is set externally by the Character/Inventory system
    // via SetCombatStance(). Here we only update the attacking flag
    // by checking if a montage tagged "Attack" is playing.

    if (!OwnerCharacter) return;

    // Check if any attack montage is currently active
    UAnimMontage* CurrentMontage = GetCurrentActiveMontage();
    if (CurrentMontage)
    {
        const FName MontageTag = CurrentMontage->GetFName();
        // Simple name-based check — a full implementation would use asset tags
        const FString MontageName = MontageTag.ToString().ToLower();
        bIsAttacking = MontageName.Contains(TEXT("attack")) || MontageName.Contains(TEXT("strike"));
    }
    else
    {
        bIsAttacking = false;
    }
}

// ============================================================
// UpdateSurvivalModifiers
// ============================================================

void UDinoSurvivorAnimInstance::UpdateSurvivalModifiers(float DeltaSeconds)
{
    // Survival stats are read from the character's properties.
    // TranspersonalCharacter exposes Health, Stamina, Fear as floats.
    // We use reflection to read them safely without a hard dependency.

    if (!OwnerCharacter) return;

    // Default to full health/stamina, no fear if properties not found
    float RawHealth  = 100.0f;
    float RawStamina = 100.0f;
    float RawFear    = 0.0f;

    // Try to read via UObject property reflection
    UClass* CharClass = OwnerCharacter->GetClass();

    if (FFloatProperty* HealthProp = FindFProperty<FFloatProperty>(CharClass, FName("Health")))
    {
        RawHealth = HealthProp->GetPropertyValue_InContainer(OwnerCharacter);
    }
    if (FFloatProperty* StaminaProp = FindFProperty<FFloatProperty>(CharClass, FName("Stamina")))
    {
        RawStamina = StaminaProp->GetPropertyValue_InContainer(OwnerCharacter);
    }
    if (FFloatProperty* FearProp = FindFProperty<FFloatProperty>(CharClass, FName("Fear")))
    {
        RawFear = FearProp->GetPropertyValue_InContainer(OwnerCharacter);
    }

    // Normalise to [0..1] — assume max values of 100
    HealthNormalised  = FMath::Clamp(RawHealth  / 100.0f, 0.0f, 1.0f);
    StaminaNormalised = FMath::Clamp(RawStamina / 100.0f, 0.0f, 1.0f);
    FearNormalised    = FMath::Clamp(RawFear    / 100.0f, 0.0f, 1.0f);

    // Compute additive lean angle:
    //   Low stamina → forward lean (exhaustion)
    //   High fear   → backward lean / crouch (threat response)
    const float ExhaustionLean = (1.0f - StaminaNormalised) * -12.0f; // forward lean up to -12°
    const float FearLean       = FearNormalised * 5.0f;                // backward lean up to +5°
    SurvivalLeanAngle = ExhaustionLean + FearLean;
}

// ============================================================
// UpdateAimOffset
// ============================================================

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    // Delta between controller rotation (where player aims) and actor rotation
    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator ActorRotation   = OwnerCharacter->GetActorRotation();
    const FRotator DeltaRotation   = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);

    // Clamp to aim offset range
    AimPitch = FMath::Clamp(DeltaRotation.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::Clamp(DeltaRotation.Yaw,   -90.0f, 90.0f);
}

// ============================================================
// TraceFootIK
// ============================================================

float UDinoSurvivorAnimInstance::TraceFootIK(FName SocketName, float TraceDistance) const
{
    if (!OwnerCharacter) return 0.0f;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return 0.0f;

    const FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    const FVector TraceStart     = SocketLocation + FVector(0.0f, 0.0f, TraceDistance);
    const FVector TraceEnd       = SocketLocation - FVector(0.0f, 0.0f, TraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    const bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        // Return Z offset relative to socket location
        return HitResult.ImpactPoint.Z - SocketLocation.Z;
    }

    return 0.0f;
}

// ============================================================
// InterpToTarget
// ============================================================

float UDinoSurvivorAnimInstance::InterpToTarget(float Current, float Target, float DeltaSeconds, float Speed) const
{
    return FMath::FInterpTo(Current, Target, DeltaSeconds, Speed);
}
