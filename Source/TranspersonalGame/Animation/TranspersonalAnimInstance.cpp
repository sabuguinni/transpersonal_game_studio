// TranspersonalAnimInstance.cpp
// Agent #10 — Animation Agent
// Prehistoric survival character animation instance.
// Drives locomotion blend, survival-state poses, and foot IK.

#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    GroundSpeed = 0.0f;
    StrafeSpeed = 0.0f;
    bIsAccelerating = false;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    MovementDirection = 0.0f;

    // Survival defaults
    StaminaNormalized = 1.0f;
    HealthNormalized = 1.0f;
    FearNormalized = 0.0f;
    bIsCarryingHeavyObject = false;

    // IK defaults
    bEnableFootIK = true;
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    PelvisOffset = 0.0f;
    CurrentPelvisOffset = 0.0f;

    // Config
    SprintThreshold = 400.0f;
    WalkThreshold = 10.0f;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
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
        // Re-attempt cache in case of late binding
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

    UpdateLocomotionData(DeltaSeconds);
    UpdateSurvivalData();

    if (bEnableFootIK && !bIsInAir)
    {
        UpdateFootIK();
    }
}

void UTranspersonalAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    // Velocity in actor-local space
    FVector Velocity = MovementComponent->Velocity;
    FVector LocalVelocity = OwnerCharacter->GetActorTransform().InverseTransformVector(Velocity);

    GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
    StrafeSpeed = LocalVelocity.Y;

    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsSprinting = (GroundSpeed > SprintThreshold);
    bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;

    MovementDirection = CalculateMovementDirection();
}

void UTranspersonalAnimInstance::UpdateSurvivalData()
{
    // Attempt to read survival stats from the character.
    // TranspersonalCharacter exposes Health, Stamina, Fear as floats (0-100 range).
    // We use reflection-safe property access via cast; if the character doesn't
    // expose these properties the values stay at their defaults (full health/stamina).
    if (!OwnerCharacter) return;

    // Try to get survival properties via UObject property system
    // This is safe even if the properties don't exist — we just skip them.
    UClass* CharClass = OwnerCharacter->GetClass();

    // Health
    if (FFloatProperty* HealthProp = FindFProperty<FFloatProperty>(CharClass, TEXT("Health")))
    {
        float RawHealth = HealthProp->GetPropertyValue_InContainer(OwnerCharacter);
        // Assume max health = 100
        HealthNormalized = FMath::Clamp(RawHealth / 100.0f, 0.0f, 1.0f);
    }

    // Stamina
    if (FFloatProperty* StaminaProp = FindFProperty<FFloatProperty>(CharClass, TEXT("Stamina")))
    {
        float RawStamina = StaminaProp->GetPropertyValue_InContainer(OwnerCharacter);
        StaminaNormalized = FMath::Clamp(RawStamina / 100.0f, 0.0f, 1.0f);
    }

    // Fear
    if (FFloatProperty* FearProp = FindFProperty<FFloatProperty>(CharClass, TEXT("Fear")))
    {
        float RawFear = FearProp->GetPropertyValue_InContainer(OwnerCharacter);
        FearNormalized = FMath::Clamp(RawFear / 100.0f, 0.0f, 1.0f);
    }
}

void UTranspersonalAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter) return;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    // Foot bone socket names (standard UE5 mannequin naming)
    const FName LeftFootSocket  = TEXT("foot_l");
    const FName RightFootSocket = TEXT("foot_r");

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    FCollisionQueryParams TraceParams(TEXT("FootIK"), true, OwnerCharacter);
    const float TraceHalfHeight = 50.0f;

    // Left foot
    {
        FVector FootLoc = Mesh->GetSocketLocation(LeftFootSocket);
        FVector TraceStart = FootLoc + FVector(0, 0, TraceHalfHeight);
        FVector TraceEnd   = FootLoc - FVector(0, 0, TraceHalfHeight * 2.0f);

        FHitResult HitResult;
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd,
            ECC_Visibility, TraceParams))
        {
            LeftFootIKLocation = HitResult.Location;
        }
        else
        {
            LeftFootIKLocation = FootLoc;
        }
    }

    // Right foot
    {
        FVector FootLoc = Mesh->GetSocketLocation(RightFootSocket);
        FVector TraceStart = FootLoc + FVector(0, 0, TraceHalfHeight);
        FVector TraceEnd   = FootLoc - FVector(0, 0, TraceHalfHeight * 2.0f);

        FHitResult HitResult;
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd,
            ECC_Visibility, TraceParams))
        {
            RightFootIKLocation = HitResult.Location;
        }
        else
        {
            RightFootIKLocation = FootLoc;
        }
    }

    // Pelvis offset — lower pelvis to the foot that needs to go lower
    float LeftDelta  = LeftFootIKLocation.Z  - Mesh->GetSocketLocation(LeftFootSocket).Z;
    float RightDelta = RightFootIKLocation.Z - Mesh->GetSocketLocation(RightFootSocket).Z;
    float TargetPelvisOffset = FMath::Min(LeftDelta, RightDelta);

    // Smooth interpolation
    CurrentPelvisOffset = FMath::FInterpTo(
        CurrentPelvisOffset,
        TargetPelvisOffset,
        GetWorld()->GetDeltaSeconds(),
        PelvisInterpSpeed
    );
    PelvisOffset = CurrentPelvisOffset;
}

float UTranspersonalAnimInstance::CalculateMovementDirection() const
{
    if (!OwnerCharacter) return 0.0f;

    FVector Velocity = OwnerCharacter->GetVelocity();
    if (Velocity.SizeSquared2D() < 1.0f) return 0.0f;

    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    FRotator VelocityRotation = Velocity.Rotation();

    float Delta = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, ActorRotation).Yaw;
    return FMath::Clamp(Delta, -180.0f, 180.0f);
}
