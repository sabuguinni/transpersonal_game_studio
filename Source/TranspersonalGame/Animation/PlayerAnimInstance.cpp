// PlayerAnimInstance.cpp
// Agent #10 — Animation Agent | PROD_CYCLE_AUTO_20260629_004
// Implements UAnimInstance for the prehistoric human player character.
// Drives idle/walk/run/sprint blend space, jump, fall, land, crouch states.

#include "PlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsAccelerating = false;

    // Survival state defaults
    StaminaRatio = 1.0f;
    FearLevel = 0.0f;
    HealthRatio = 1.0f;

    // IK defaults
    bEnableFootIK = true;
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    LeftFootOffset = FVector::ZeroVector;
    RightFootOffset = FVector::ZeroVector;

    // Lean defaults
    LeanAngle = 0.0f;
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    // State defaults
    LocomotionState = EAnim_PlayerLocomotionState::Idle;
    CombatStance = EAnim_PlayerCombatStance::Unarmed;
    ActionState = EAnim_PlayerActionState::None;

    // Blend weights
    UpperBodyBlendWeight = 0.0f;
    CrouchBlendAlpha = 0.0f;
    SprintBlendAlpha = 0.0f;

    // Cached refs
    CachedCharacter = nullptr;
    CachedMovement = nullptr;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* PawnOwner = TryGetPawnOwner();
    if (!PawnOwner)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerAnimInstance: No pawn owner on init"));
        return;
    }

    CachedCharacter = Cast<ACharacter>(PawnOwner);
    if (CachedCharacter)
    {
        CachedMovement = CachedCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("PlayerAnimInstance: Initialized for %s"), *CachedCharacter->GetName());
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!CachedCharacter || !CachedMovement)
    {
        // Try to re-cache if lost
        APawn* PawnOwner = TryGetPawnOwner();
        if (PawnOwner)
        {
            CachedCharacter = Cast<ACharacter>(PawnOwner);
            if (CachedCharacter)
            {
                CachedMovement = CachedCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    // --- LOCOMOTION ---
    FVector Velocity = CachedMovement->Velocity;
    Speed = Velocity.Size2D();

    // Direction relative to actor forward
    FRotator ActorRotation = CachedCharacter->GetActorRotation();
    Direction = CalculateDirection(Velocity, ActorRotation);

    // Air state
    bIsInAir = CachedMovement->IsFalling();

    // Crouch state
    bIsCrouching = CachedMovement->IsCrouching();

    // Acceleration check
    bIsAccelerating = CachedMovement->GetCurrentAcceleration().SizeSquared() > 0.0f;

    // Sprint detection: speed > walk threshold and accelerating forward
    const float SprintSpeedThreshold = 400.0f;
    bIsSprinting = (Speed > SprintSpeedThreshold) && bIsAccelerating;

    // --- LOCOMOTION STATE MACHINE ---
    UpdateLocomotionState();

    // --- BLEND ALPHAS ---
    CrouchBlendAlpha = FMath::FInterpTo(CrouchBlendAlpha, bIsCrouching ? 1.0f : 0.0f, DeltaSeconds, 8.0f);
    SprintBlendAlpha = FMath::FInterpTo(SprintBlendAlpha, bIsSprinting ? 1.0f : 0.0f, DeltaSeconds, 6.0f);

    // --- LEAN ---
    const float LeanInterpSpeed = 4.0f;
    float TargetLean = FMath::Clamp(Direction * 0.5f, -45.0f, 45.0f);
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, LeanInterpSpeed);

    // --- FOOT IK ---
    if (bEnableFootIK && !bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, 10.0f);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, 10.0f);
    }

    // --- FEAR TREMOR ---
    if (FearLevel > 0.5f)
    {
        UpdateFearTremor(DeltaSeconds);
    }
}

void UPlayerAnimInstance::UpdateLocomotionState()
{
    if (bIsInAir)
    {
        if (CachedMovement->Velocity.Z > 0.0f)
        {
            LocomotionState = EAnim_PlayerLocomotionState::Jump;
        }
        else
        {
            LocomotionState = EAnim_PlayerLocomotionState::Fall;
        }
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = (Speed > 10.0f)
            ? EAnim_PlayerLocomotionState::CrouchWalk
            : EAnim_PlayerLocomotionState::Crouch;
        return;
    }

    if (Speed < 10.0f)
    {
        LocomotionState = EAnim_PlayerLocomotionState::Idle;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_PlayerLocomotionState::Sprint;
    }
    else if (Speed > 150.0f)
    {
        LocomotionState = EAnim_PlayerLocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_PlayerLocomotionState::Walk;
    }
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!CachedCharacter)
    {
        return;
    }

    UWorld* World = CachedCharacter->GetWorld();
    if (!World)
    {
        return;
    }

    // Trace parameters
    FCollisionQueryParams TraceParams(FName(TEXT("FootIK")), true, CachedCharacter);
    const float TraceLength = 50.0f;
    const float IKInterpSpeed = 12.0f;

    // Left foot bone location (approximate — Blueprint sets exact socket)
    FVector LeftFootWorld = CachedCharacter->GetMesh()
        ? CachedCharacter->GetMesh()->GetSocketLocation(FName("foot_l"))
        : CachedCharacter->GetActorLocation();

    FVector LeftStart = LeftFootWorld + FVector(0, 0, TraceLength);
    FVector LeftEnd   = LeftFootWorld - FVector(0, 0, TraceLength);

    FHitResult LeftHit;
    bool bLeftHit = World->LineTraceSingleByChannel(LeftHit, LeftStart, LeftEnd,
        ECollisionChannel::ECC_WorldStatic, TraceParams);

    // Right foot
    FVector RightFootWorld = CachedCharacter->GetMesh()
        ? CachedCharacter->GetMesh()->GetSocketLocation(FName("foot_r"))
        : CachedCharacter->GetActorLocation();

    FVector RightStart = RightFootWorld + FVector(0, 0, TraceLength);
    FVector RightEnd   = RightFootWorld - FVector(0, 0, TraceLength);

    FHitResult RightHit;
    bool bRightHit = World->LineTraceSingleByChannel(RightHit, RightStart, RightEnd,
        ECollisionChannel::ECC_WorldStatic, TraceParams);

    // Compute offsets
    FVector TargetLeftOffset  = bLeftHit  ? (LeftHit.ImpactPoint  - LeftFootWorld)  : FVector::ZeroVector;
    FVector TargetRightOffset = bRightHit ? (RightHit.ImpactPoint - RightFootWorld) : FVector::ZeroVector;

    LeftFootOffset  = FMath::VInterpTo(LeftFootOffset,  TargetLeftOffset,  DeltaSeconds, IKInterpSpeed);
    RightFootOffset = FMath::VInterpTo(RightFootOffset, TargetRightOffset, DeltaSeconds, IKInterpSpeed);

    // Alpha: blend IK in when grounded and moving slowly enough
    float TargetAlpha = (Speed < 300.0f) ? 1.0f : FMath::Clamp(1.0f - (Speed - 300.0f) / 200.0f, 0.0f, 1.0f);
    LeftFootIKAlpha  = FMath::FInterpTo(LeftFootIKAlpha,  TargetAlpha, DeltaSeconds, IKInterpSpeed);
    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, TargetAlpha, DeltaSeconds, IKInterpSpeed);
}

void UPlayerAnimInstance::UpdateFearTremor(float DeltaSeconds)
{
    // Procedural tremor driven by FearLevel — adds subtle shake to upper body
    // Implemented as a time-based sine wave modulating AimPitch/AimYaw slightly
    static float TremorTime = 0.0f;
    TremorTime += DeltaSeconds;

    float TremorIntensity = FMath::Clamp((FearLevel - 0.5f) * 2.0f, 0.0f, 1.0f);
    float TremorMagnitude = TremorIntensity * 3.0f; // max 3 degrees shake

    // High-frequency tremor for fear
    float TremorX = FMath::Sin(TremorTime * 12.0f) * TremorMagnitude;
    float TremorY = FMath::Sin(TremorTime * 9.0f  + 1.2f) * TremorMagnitude * 0.6f;

    // Modulate aim offsets slightly
    AimPitch += TremorX;
    AimYaw   += TremorY;
}

void UPlayerAnimInstance::SetCombatStance(EAnim_PlayerCombatStance NewStance)
{
    if (CombatStance != NewStance)
    {
        CombatStance = NewStance;
        UpperBodyBlendWeight = (NewStance != EAnim_PlayerCombatStance::Unarmed) ? 1.0f : 0.0f;
    }
}

void UPlayerAnimInstance::SetActionState(EAnim_PlayerActionState NewAction)
{
    ActionState = NewAction;
}

void UPlayerAnimInstance::TriggerJump()
{
    LocomotionState = EAnim_PlayerLocomotionState::Jump;
}

void UPlayerAnimInstance::TriggerLand()
{
    LocomotionState = EAnim_PlayerLocomotionState::Land;
}

void UPlayerAnimInstance::SetSurvivalStats(float InHealth, float InStamina, float InFear)
{
    HealthRatio  = FMath::Clamp(InHealth,  0.0f, 1.0f);
    StaminaRatio = FMath::Clamp(InStamina, 0.0f, 1.0f);
    FearLevel    = FMath::Clamp(InFear,    0.0f, 1.0f);
}
