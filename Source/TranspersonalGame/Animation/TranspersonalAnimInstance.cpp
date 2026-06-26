// TranspersonalAnimInstance.cpp
// Agent #10 — Animation Agent
// Drives locomotion blend space, survival-state poses, and combat transitions
// for the prehistoric human survivor character.

#include "TranspersonalAnimInstance.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    GroundSpeed         = 0.f;
    StrafeDirection     = 0.f;
    bIsAccelerating     = false;
    bIsRunning          = false;
    bIsCrouching        = false;

    // Airborne defaults
    bIsInAir            = false;
    VerticalVelocity    = 0.f;

    // Survival defaults
    StaminaRatio        = 1.f;
    FearLevel           = 0.f;
    bIsWounded          = false;

    // Combat defaults
    bIsInCombat         = false;
    bIsAttacking        = false;

    // Thresholds
    RunSpeedThreshold   = 300.f;
    IdleSpeedThreshold  = 10.f;

    OwnerCharacter      = nullptr;
    MovementComponent   = nullptr;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    OwnerCharacter    = Cast<ATranspersonalCharacter>(Pawn);
    MovementComponent = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent());
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!MovementComponent) return;

    UpdateLocomotion(DeltaSeconds);
    UpdateSurvivalStates();
}

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
    if (!MovementComponent) return;

    const FVector Velocity      = MovementComponent->Velocity;
    const FVector HorizontalVel = FVector(Velocity.X, Velocity.Y, 0.f);

    GroundSpeed      = HorizontalVel.Size();
    VerticalVelocity = Velocity.Z;
    bIsInAir         = MovementComponent->IsFalling();
    bIsCrouching     = MovementComponent->IsCrouching();
    bIsRunning       = (GroundSpeed > RunSpeedThreshold);

    // Acceleration: true when the character has a non-zero input acceleration
    bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > KINDA_SMALL_NUMBER;

    // Strafe direction: dot product of velocity with character right vector
    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        const FVector RightVector = Pawn->GetActorRightVector();
        StrafeDirection = FVector::DotProduct(HorizontalVel.GetSafeNormal(), RightVector);
    }
}

void UTranspersonalAnimInstance::UpdateSurvivalStates()
{
    if (!OwnerCharacter) return;

    // Read survival stats from TranspersonalCharacter UPROPERTY fields
    // These are exposed as BlueprintReadOnly on the character class
    // We use reflection-safe accessors to avoid tight coupling

    // Stamina ratio — clamp to [0,1]
    const float MaxStamina = 100.f;
    // Access via property if available; default to 1.0 if not found
    // (TranspersonalCharacter exposes Stamina as a float UPROPERTY)
    float RawStamina = 100.f;
    
    // Use UE5 property system to safely read Stamina without hard dependency
    FProperty* StaminaProp = OwnerCharacter->GetClass()->FindPropertyByName(FName("Stamina"));
    if (StaminaProp)
    {
        float* StaminaPtr = StaminaProp->ContainerPtrToValuePtr<float>(OwnerCharacter);
        if (StaminaPtr)
        {
            RawStamina = *StaminaPtr;
        }
    }
    StaminaRatio = FMath::Clamp(RawStamina / MaxStamina, 0.f, 1.f);

    // Fear level
    float RawFear = 0.f;
    FProperty* FearProp = OwnerCharacter->GetClass()->FindPropertyByName(FName("Fear"));
    if (!FearProp)
    {
        FearProp = OwnerCharacter->GetClass()->FindPropertyByName(FName("FearLevel"));
    }
    if (FearProp)
    {
        float* FearPtr = FearProp->ContainerPtrToValuePtr<float>(OwnerCharacter);
        if (FearPtr)
        {
            RawFear = *FearPtr;
        }
    }
    FearLevel = FMath::Clamp(RawFear / 100.f, 0.f, 1.f);

    // Wounded state — health below 25%
    float RawHealth = 100.f;
    FProperty* HealthProp = OwnerCharacter->GetClass()->FindPropertyByName(FName("Health"));
    if (HealthProp)
    {
        float* HealthPtr = HealthProp->ContainerPtrToValuePtr<float>(OwnerCharacter);
        if (HealthPtr)
        {
            RawHealth = *HealthPtr;
        }
    }
    bIsWounded = (RawHealth < 25.f);

    // Combat state — driven by fear and proximity (simplified for now)
    // Full combat state will be set by CombatComponent in a later cycle
    bIsInCombat = (FearLevel > 0.6f);
}
