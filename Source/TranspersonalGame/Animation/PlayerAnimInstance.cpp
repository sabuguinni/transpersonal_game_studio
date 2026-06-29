// PlayerAnimInstance.cpp
// Agent #10 — Animation Agent
// Prehistoric survival game — UAnimInstance implementation for the player character

#include "PlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    Speed = 0.f;
    Direction = 0.f;
    SmoothedSpeed = 0.f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsAccelerating = false;
    bIsInCombat = false;
    bIsAiming = false;
    bIsAttacking = false;
    ExhaustionAlpha = 0.f;
    InjuryAlpha = 0.f;
    FearAlpha = 0.f;
    LeanAngle = 0.f;
    TurnRate = 0.f;
    FootIKLeftAlpha = 1.f;
    FootIKRightAlpha = 1.f;
    CachedCharacter = nullptr;
    CachedMovement = nullptr;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn) return;

    CachedCharacter = Cast<ACharacter>(OwnerPawn);
    if (CachedCharacter)
    {
        CachedMovement = CachedCharacter->GetCharacterMovement();
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!CachedCharacter || !CachedMovement) return;

    // --- Locomotion ---
    FVector Velocity = CachedMovement->Velocity;
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.f);
    Speed = HorizontalVelocity.Size();

    // Smooth speed for blend space transitions
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaSeconds, 8.f);

    // Direction relative to character facing
    FRotator ActorRotation = CachedCharacter->GetActorRotation();
    Direction = CalculateDirection(Velocity, ActorRotation);

    // Movement state flags
    bIsInAir = CachedMovement->IsFalling();
    bIsCrouching = CachedCharacter->bIsCrouched;
    bIsAccelerating = CachedMovement->GetCurrentAcceleration().SizeSquared() > 0.f;

    // Sprint: speed above walk threshold (walk ~200, run ~400, sprint ~600)
    bIsSprinting = Speed > 450.f && bIsAccelerating;

    // --- Lean and Turn ---
    FRotator CurrentRotation = CachedCharacter->GetActorRotation();
    TurnRate = FMath::Clamp((CurrentRotation.Yaw - PreviousYaw) / FMath::Max(DeltaSeconds, 0.001f), -180.f, 180.f);
    PreviousYaw = CurrentRotation.Yaw;

    // Lean based on lateral velocity
    float LateralVelocity = FVector::DotProduct(Velocity, CachedCharacter->GetActorRightVector());
    LeanAngle = FMath::FInterpTo(LeanAngle, FMath::Clamp(LateralVelocity * 0.05f, -15.f, 15.f), DeltaSeconds, 5.f);

    // --- Survival State Alphas ---
    // These are set externally by the character/survival system
    // ExhaustionAlpha, InjuryAlpha, FearAlpha remain as set

    // --- Foot IK ---
    UpdateFootIK(DeltaSeconds);
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!CachedCharacter) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Left foot IK trace
    FVector LeftFootLocation = CachedCharacter->GetMesh()
        ? CachedCharacter->GetMesh()->GetSocketLocation(FName("foot_l"))
        : CachedCharacter->GetActorLocation();

    FVector RightFootLocation = CachedCharacter->GetMesh()
        ? CachedCharacter->GetMesh()->GetSocketLocation(FName("foot_r"))
        : CachedCharacter->GetActorLocation();

    FHitResult LeftHit, RightHit;
    FVector TraceStart, TraceEnd;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(CachedCharacter);

    // Left foot trace
    TraceStart = LeftFootLocation + FVector(0.f, 0.f, 50.f);
    TraceEnd   = LeftFootLocation - FVector(0.f, 0.f, 100.f);
    bool bLeftHit = World->LineTraceSingleByChannel(LeftHit, TraceStart, TraceEnd, ECC_Visibility, Params);
    FootIKLeftAlpha  = FMath::FInterpTo(FootIKLeftAlpha,  bLeftHit  ? 1.f : 0.f, DeltaSeconds, 10.f);

    // Right foot trace
    TraceStart = RightFootLocation + FVector(0.f, 0.f, 50.f);
    TraceEnd   = RightFootLocation - FVector(0.f, 0.f, 100.f);
    bool bRightHit = World->LineTraceSingleByChannel(RightHit, TraceStart, TraceEnd, ECC_Visibility, Params);
    FootIKRightAlpha = FMath::FInterpTo(FootIKRightAlpha, bRightHit ? 1.f : 0.f, DeltaSeconds, 10.f);
}

void UPlayerAnimInstance::SetSurvivalStats(float ExhaustionIn, float InjuryIn, float FearIn)
{
    ExhaustionAlpha = FMath::Clamp(ExhaustionIn, 0.f, 1.f);
    InjuryAlpha     = FMath::Clamp(InjuryIn,     0.f, 1.f);
    FearAlpha       = FMath::Clamp(FearIn,        0.f, 1.f);
}

void UPlayerAnimInstance::SetCombatState(bool bInCombat, bool bAiming, bool bAttacking)
{
    bIsInCombat  = bInCombat;
    bIsAiming    = bAiming;
    bIsAttacking = bAttacking;
}
