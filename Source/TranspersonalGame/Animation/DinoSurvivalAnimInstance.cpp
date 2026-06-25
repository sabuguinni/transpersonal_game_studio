#include "DinoSurvivalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivalAnimInstance::UDinoSurvivalAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsMoving = false;

    // Survival defaults
    Stamina = 100.0f;
    FearLevel = 0.0f;
    bIsThreatened = false;

    // Combat defaults
    bIsInCombat = false;
    bIsAttacking = false;
    bIsArmed = false;

    // IK defaults
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    PelvisOffset = 0.0f;

    // Thresholds
    MovingThreshold = 10.0f;
    SprintThreshold = 400.0f;
    ThreatFearThreshold = 60.0f;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UDinoSurvivalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UDinoSurvivalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // ── Locomotion ──────────────────────────────────────────────────────────

    const FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    bIsMoving = Speed > MovingThreshold;
    bIsSprinting = Speed > SprintThreshold;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Direction: angle between actor forward and velocity direction
    if (bIsMoving)
    {
        const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        const FRotator VelocityRotation = Velocity.Rotation();
        const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, ActorRotation);
        Direction = DeltaRot.Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // ── Survival Stats ───────────────────────────────────────────────────────
    // These are read from the character's survival component if available.
    // For now we use direct property access with a soft interface.
    // The TranspersonalCharacter exposes these as public floats.
    {
        // Try to read survival stats via reflection (safe — no hard dependency)
        UObject* CharObj = Cast<UObject>(OwnerCharacter);
        if (CharObj)
        {
            // Stamina
            FFloatProperty* StaminaProp = FindFProperty<FFloatProperty>(CharObj->GetClass(), TEXT("Stamina"));
            if (StaminaProp)
            {
                Stamina = StaminaProp->GetPropertyValue_InContainer(CharObj);
            }

            // Fear
            FFloatProperty* FearProp = FindFProperty<FFloatProperty>(CharObj->GetClass(), TEXT("Fear"));
            if (FearProp)
            {
                FearLevel = FearProp->GetPropertyValue_InContainer(CharObj);
            }
        }
    }

    bIsThreatened = FearLevel >= ThreatFearThreshold;

    // ── Foot IK ──────────────────────────────────────────────────────────────
    if (!bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        // Reset IK while airborne
        LeftFootIKLocation = FVector::ZeroVector;
        RightFootIKLocation = FVector::ZeroVector;
        PelvisOffset = 0.0f;
    }
}

void UDinoSurvivalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    bool bLeftHit = TraceFootIK(TEXT("foot_l"), LeftFootIKLocation);
    bool bRightHit = TraceFootIK(TEXT("foot_r"), RightFootIKLocation);

    // Pelvis offset: lower pelvis to the lowest foot so both can reach ground
    if (bLeftHit && bRightHit)
    {
        const float LeftZ = LeftFootIKLocation.Z;
        const float RightZ = RightFootIKLocation.Z;
        const float LowestFoot = FMath::Min(LeftZ, RightZ);
        const FVector CharLoc = OwnerCharacter->GetActorLocation();
        PelvisOffset = FMath::FInterpTo(PelvisOffset, LowestFoot - CharLoc.Z, DeltaSeconds, 10.0f);
    }
    else
    {
        PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.0f, DeltaSeconds, 10.0f);
    }
}

bool UDinoSurvivalAnimInstance::TraceFootIK(const FName& FootSocketName, FVector& OutLocation)
{
    if (!OwnerCharacter)
    {
        return false;
    }

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        return false;
    }

    const FVector SocketLocation = Mesh->GetSocketLocation(FootSocketName);
    const FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, 50.0f);
    const FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, 75.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    const bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        Params
    );

    if (bHit)
    {
        OutLocation = HitResult.ImpactPoint;
        return true;
    }

    OutLocation = SocketLocation;
    return false;
}
