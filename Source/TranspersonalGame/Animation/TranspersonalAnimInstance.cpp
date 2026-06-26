// TranspersonalAnimInstance.cpp
// Animation Agent #10 — Cycle PROD_CYCLE_AUTO_20260626_004
// Locomotion blend space + IK foot placement for prehistoric survivor character

#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsIdle = true;

    // IK defaults
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    LeftFootOffset = FVector::ZeroVector;
    RightFootOffset = FVector::ZeroVector;
    IKTraceDistance = 50.0f;
    IKInterpSpeed = 12.0f;

    // Survival state defaults
    StaminaNormalized = 1.0f;
    HealthNormalized = 1.0f;
    FearLevel = 0.0f;
    bIsExhausted = false;
    bIsWounded = false;

    // Cached refs
    OwnerCharacter = nullptr;
    OwnerMovement = nullptr;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    OwnerCharacter = Cast<ACharacter>(Pawn);
    if (OwnerCharacter)
    {
        OwnerMovement = OwnerCharacter->GetCharacterMovement();
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !OwnerMovement) return;

    // --- Locomotion ---
    FVector Velocity = OwnerMovement->Velocity;
    Speed = Velocity.Size2D();

    // Direction: angle between character forward and velocity
    if (Speed > 1.0f)
    {
        FRotator CharRot = OwnerCharacter->GetActorRotation();
        FVector VelDir = Velocity.GetSafeNormal();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(
            VelDir.Rotation(), CharRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    bIsInAir = OwnerMovement->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsIdle = (Speed < 10.0f && !bIsInAir);

    // Sprint: check if max walk speed is elevated
    bIsSprinting = (Speed > 400.0f && !bIsInAir);

    // --- Survival State ---
    // Try to read survival stats from character if it exposes them
    // Using safe property access pattern
    UpdateSurvivalState();

    // --- Foot IK ---
    if (!bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 1.0f, DeltaSeconds, IKInterpSpeed);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 1.0f, DeltaSeconds, IKInterpSpeed);
    }
    else
    {
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, IKInterpSpeed);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, IKInterpSpeed);
    }
}

void UTranspersonalAnimInstance::UpdateSurvivalState()
{
    if (!OwnerCharacter) return;

    // Read survival stats via UObject property system (works with TranspersonalCharacter UPROPERTY fields)
    // These properties are exposed via UPROPERTY in TranspersonalCharacter
    float* HealthPtr = nullptr;
    float* StaminaPtr = nullptr;
    float* FearPtr = nullptr;

    // Use reflection to safely read survival stats
    UClass* CharClass = OwnerCharacter->GetClass();
    if (!CharClass) return;

    FProperty* HealthProp = CharClass->FindPropertyByName(FName("Health"));
    FProperty* MaxHealthProp = CharClass->FindPropertyByName(FName("MaxHealth"));
    FProperty* StaminaProp = CharClass->FindPropertyByName(FName("Stamina"));
    FProperty* MaxStaminaProp = CharClass->FindPropertyByName(FName("MaxStamina"));
    FProperty* FearProp = CharClass->FindPropertyByName(FName("FearLevel"));

    float Health = 100.f, MaxHealth = 100.f, Stamina = 100.f, MaxStamina = 100.f, Fear = 0.f;

    if (FFloatProperty* FP = CastField<FFloatProperty>(HealthProp))
        Health = FP->GetPropertyValue_InContainer(OwnerCharacter);
    if (FFloatProperty* FP = CastField<FFloatProperty>(MaxHealthProp))
        MaxHealth = FP->GetPropertyValue_InContainer(OwnerCharacter);
    if (FFloatProperty* FP = CastField<FFloatProperty>(StaminaProp))
        Stamina = FP->GetPropertyValue_InContainer(OwnerCharacter);
    if (FFloatProperty* FP = CastField<FFloatProperty>(MaxStaminaProp))
        MaxStamina = FP->GetPropertyValue_InContainer(OwnerCharacter);
    if (FFloatProperty* FP = CastField<FFloatProperty>(FearProp))
        Fear = FP->GetPropertyValue_InContainer(OwnerCharacter);

    HealthNormalized = (MaxHealth > 0.f) ? FMath::Clamp(Health / MaxHealth, 0.f, 1.f) : 1.f;
    StaminaNormalized = (MaxStamina > 0.f) ? FMath::Clamp(Stamina / MaxStamina, 0.f, 1.f) : 1.f;
    FearLevel = FMath::Clamp(Fear / 100.f, 0.f, 1.f);

    bIsExhausted = (StaminaNormalized < 0.15f);
    bIsWounded = (HealthNormalized < 0.3f);
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    // Trace for left foot
    FVector LeftFootLoc = Mesh->GetSocketLocation(FName("foot_l"));
    FVector RightFootLoc = Mesh->GetSocketLocation(FName("foot_r"));

    FVector TraceUp = FVector(0, 0, IKTraceDistance);
    FVector TraceDown = FVector(0, 0, -IKTraceDistance * 2.f);

    FHitResult LeftHit, RightHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    // Left foot trace
    if (World->LineTraceSingleByChannel(LeftHit,
        LeftFootLoc + TraceUp, LeftFootLoc + TraceDown,
        ECC_Visibility, Params))
    {
        FVector Target = FVector(0, 0, LeftHit.ImpactPoint.Z - LeftFootLoc.Z);
        LeftFootOffset = FMath::VInterpTo(LeftFootOffset, Target, DeltaSeconds, IKInterpSpeed);
    }

    // Right foot trace
    if (World->LineTraceSingleByChannel(RightHit,
        RightFootLoc + TraceUp, RightFootLoc + TraceDown,
        ECC_Visibility, Params))
    {
        FVector Target = FVector(0, 0, RightHit.ImpactPoint.Z - RightFootLoc.Z);
        RightFootOffset = FMath::VInterpTo(RightFootOffset, Target, DeltaSeconds, IKInterpSpeed);
    }
}

float UTranspersonalAnimInstance::GetLocomotionPlayRate() const
{
    // Scale animation playback rate with speed for natural feel
    // Walk: 150-300 speed → rate 0.8-1.0
    // Run: 300-500 speed → rate 1.0-1.2
    // Sprint: 500+ speed → rate 1.2-1.5
    if (Speed < 10.f) return 1.0f;
    if (Speed < 300.f) return FMath::GetMappedRangeValueClamped(
        FVector2D(0.f, 300.f), FVector2D(0.8f, 1.0f), Speed);
    if (Speed < 500.f) return FMath::GetMappedRangeValueClamped(
        FVector2D(300.f, 500.f), FVector2D(1.0f, 1.2f), Speed);
    return FMath::GetMappedRangeValueClamped(
        FVector2D(500.f, 700.f), FVector2D(1.2f, 1.5f), Speed);
}

EAnim_LocomotionState UTranspersonalAnimInstance::GetLocomotionState() const
{
    if (bIsInAir) return EAnim_LocomotionState::InAir;
    if (bIsCrouching) return EAnim_LocomotionState::Crouch;
    if (bIsSprinting) return EAnim_LocomotionState::Sprint;
    if (Speed > 10.f) return EAnim_LocomotionState::Run;
    return EAnim_LocomotionState::Idle;
}
