// TranspersonalAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Implements locomotion blend, foot IK, survival state, and combat aim offset.

#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    MovementSpeed = 0.0f;
    StrafeDirection = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;

    // Survival defaults
    StaminaRatio = 1.0f;
    FearLevel = 0.0f;
    bIsInjured = false;

    // Foot IK defaults
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    PelvisOffset = 0.0f;

    // Combat defaults
    bIsArmed = false;
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache owner pawn as ACharacter
    APawn* OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        OwnerCharacter = Cast<ACharacter>(OwnerPawn);
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
        return;
    }

    // ---- Locomotion ----
    FVector Velocity = OwnerCharacter->GetVelocity();
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    MovementSpeed = HorizontalVelocity.Size();

    // Strafe direction: dot product of velocity with right vector
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    float RawStrafe = FVector::DotProduct(HorizontalVelocity.GetSafeNormal(), RightVector);
    StrafeDirection = FMath::FInterpTo(StrafeDirection, RawStrafe, DeltaSeconds, 8.0f);

    // Air state
    bIsInAir = MovementComponent->IsFalling();

    // Crouch state
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Sprint: speed > walk threshold (300 = walk, 600 = run)
    bIsSprinting = (MovementSpeed > 450.0f) && !bIsInAir;

    // ---- Aim offset ----
    FRotator ControlRotation = OwnerCharacter->GetControlRotation();
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);

    AimPitch = FMath::ClampAngle(DeltaRotation.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::ClampAngle(DeltaRotation.Yaw, -90.0f, 90.0f);

    // ---- Foot IK ----
    // Only apply foot IK when grounded and moving slowly (not sprinting)
    if (!bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 1.0f, DeltaSeconds, 10.0f);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 1.0f, DeltaSeconds, 10.0f);
    }
    else
    {
        // Disable foot IK in air
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, 10.0f);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, 10.0f);
        LeftFootIKOffset = FVector::ZeroVector;
        RightFootIKOffset = FVector::ZeroVector;
        PelvisOffset = 0.0f;
    }
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
    {
        return;
    }

    // Get socket locations for feet
    FVector LeftFootLocation = MeshComp->GetSocketLocation(FName("foot_l"));
    FVector RightFootLocation = MeshComp->GetSocketLocation(FName("foot_r"));

    // Trace parameters
    FCollisionQueryParams TraceParams(FName("FootIKTrace"), true, OwnerCharacter);
    TraceParams.bReturnPhysicalMaterial = false;

    // Left foot trace
    FHitResult LeftHit;
    FVector LeftTraceStart = LeftFootLocation + FVector(0.0f, 0.0f, FootIKTraceDistance * 0.5f);
    FVector LeftTraceEnd = LeftFootLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FVector TargetLeftOffset = FVector::ZeroVector;
    if (World->LineTraceSingleByChannel(LeftHit, LeftTraceStart, LeftTraceEnd, ECC_Visibility, TraceParams))
    {
        float FootZ = LeftHit.Location.Z - LeftFootLocation.Z;
        TargetLeftOffset = FVector(0.0f, 0.0f, FootZ);
    }
    SmoothFootIK(LeftFootIKOffset, TargetLeftOffset, DeltaSeconds, FootIKInterpSpeed);

    // Right foot trace
    FHitResult RightHit;
    FVector RightTraceStart = RightFootLocation + FVector(0.0f, 0.0f, FootIKTraceDistance * 0.5f);
    FVector RightTraceEnd = RightFootLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FVector TargetRightOffset = FVector::ZeroVector;
    if (World->LineTraceSingleByChannel(RightHit, RightTraceStart, RightTraceEnd, ECC_Visibility, TraceParams))
    {
        float FootZ = RightHit.Location.Z - RightFootLocation.Z;
        TargetRightOffset = FVector(0.0f, 0.0f, FootZ);
    }
    SmoothFootIK(RightFootIKOffset, TargetRightOffset, DeltaSeconds, FootIKInterpSpeed);

    // Pelvis adjustment: lower pelvis to the lowest foot to prevent stretching
    float LowestFoot = FMath::Min(LeftFootIKOffset.Z, RightFootIKOffset.Z);
    float TargetPelvis = FMath::Clamp(LowestFoot, -30.0f, 0.0f);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, FootIKInterpSpeed);
}

void UTranspersonalAnimInstance::SmoothFootIK(FVector& CurrentOffset, const FVector& TargetOffset, float DeltaSeconds, float InterpSpeed)
{
    CurrentOffset = FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaSeconds, InterpSpeed);
}
