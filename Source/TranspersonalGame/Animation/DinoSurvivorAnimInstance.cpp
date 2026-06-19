#include "DinoSurvivorAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
    : GroundSpeed(0.f)
    , Direction(0.f)
    , bIsInAir(false)
    , bIsCrouching(false)
    , bIsSprinting(false)
    , bIsIdle(true)
    , FearLevel(0.f)
    , StaminaRatio(1.f)
    , HealthRatio(1.f)
    , bIsWounded(false)
    , LeftFootIKTarget(FVector::ZeroVector)
    , RightFootIKTarget(FVector::ZeroVector)
    , PelvisOffset(0.f)
    , FootIKAlpha(1.f)
    , SprintThreshold(400.f)
    , IdleThreshold(10.f)
    , FootIKTraceDistance(80.f)
    , OwnerCharacter(nullptr)
{
}

// ─────────────────────────────────────────────────────────────────────────────
//  NativeInitializeAnimation
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        OwnerCharacter = Cast<ACharacter>(Pawn);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  NativeUpdateAnimation  (called every frame)
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        APawn* Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            OwnerCharacter = Cast<ACharacter>(Pawn);
        }
        if (!OwnerCharacter)
        {
            return;
        }
    }

    UpdateLocomotionVariables();
    UpdateSurvivalVariables();

    // Only run foot IK when grounded — saves traces while airborne
    if (!bIsInAir)
    {
        UpdateFootIK();
    }
    else
    {
        FootIKAlpha = 0.f;
        PelvisOffset = 0.f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  UpdateLocomotionVariables
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotionVariables()
{
    UCharacterMovementComponent* MovComp =
        OwnerCharacter->GetCharacterMovement();

    if (!MovComp)
    {
        return;
    }

    // Ground speed (horizontal only)
    FVector Velocity = OwnerCharacter->GetVelocity();
    Velocity.Z = 0.f;
    GroundSpeed = Velocity.Size();

    // Direction relative to actor facing (-180..180)
    Direction = UKismetMathLibrary::NormalizedDeltaRotator(
        Velocity.Rotation(),
        OwnerCharacter->GetActorRotation()
    ).Yaw;

    // State flags
    bIsInAir   = MovComp->IsFalling();
    bIsCrouching = MovComp->IsCrouching();
    bIsSprinting = GroundSpeed >= SprintThreshold;
    bIsIdle      = GroundSpeed <= IdleThreshold;

    // Foot IK should fade out while airborne
    if (bIsInAir)
    {
        FootIKAlpha = FMath::FInterpTo(FootIKAlpha, 0.f, GetWorld()->GetDeltaSeconds(), 8.f);
    }
    else
    {
        FootIKAlpha = FMath::FInterpTo(FootIKAlpha, 1.f, GetWorld()->GetDeltaSeconds(), 8.f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  UpdateSurvivalVariables
//  Reads stats from the character if it exposes them; falls back to defaults.
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateSurvivalVariables()
{
    // Try to read survival stats via reflection so we don't hard-depend on
    // TranspersonalCharacter.h here (avoids circular include issues).
    UObject* CharObj = Cast<UObject>(OwnerCharacter);
    if (!CharObj)
    {
        return;
    }

    // Health
    FFloatProperty* HealthProp = FindFProperty<FFloatProperty>(
        CharObj->GetClass(), TEXT("Health"));
    FFloatProperty* MaxHealthProp = FindFProperty<FFloatProperty>(
        CharObj->GetClass(), TEXT("MaxHealth"));

    if (HealthProp && MaxHealthProp)
    {
        float Health    = HealthProp->GetPropertyValue_InContainer(CharObj);
        float MaxHealth = MaxHealthProp->GetPropertyValue_InContainer(CharObj);
        HealthRatio = (MaxHealth > 0.f) ? FMath::Clamp(Health / MaxHealth, 0.f, 1.f) : 1.f;
    }

    // Stamina
    FFloatProperty* StaminaProp = FindFProperty<FFloatProperty>(
        CharObj->GetClass(), TEXT("Stamina"));
    FFloatProperty* MaxStaminaProp = FindFProperty<FFloatProperty>(
        CharObj->GetClass(), TEXT("MaxStamina"));

    if (StaminaProp && MaxStaminaProp)
    {
        float Stamina    = StaminaProp->GetPropertyValue_InContainer(CharObj);
        float MaxStamina = MaxStaminaProp->GetPropertyValue_InContainer(CharObj);
        StaminaRatio = (MaxStamina > 0.f) ? FMath::Clamp(Stamina / MaxStamina, 0.f, 1.f) : 1.f;
    }

    // Fear
    FFloatProperty* FearProp = FindFProperty<FFloatProperty>(
        CharObj->GetClass(), TEXT("Fear"));
    if (FearProp)
    {
        FearLevel = FMath::Clamp(
            FearProp->GetPropertyValue_InContainer(CharObj), 0.f, 1.f);
    }

    // Derived flags
    bIsWounded = HealthRatio < 0.3f;
}

// ─────────────────────────────────────────────────────────────────────────────
//  UpdateFootIK
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateFootIK()
{
    float LeftHitZ  = 0.f;
    float RightHitZ = 0.f;

    LeftFootIKTarget  = TraceFootToGround(TEXT("foot_l"), LeftHitZ);
    RightFootIKTarget = TraceFootToGround(TEXT("foot_r"), RightHitZ);

    // Pelvis drops to keep the lower foot planted
    float LowerFoot = FMath::Min(LeftHitZ, RightHitZ);
    float ActorZ    = OwnerCharacter->GetActorLocation().Z;

    // Smooth pelvis offset to avoid jitter
    float TargetPelvisOffset = LowerFoot - ActorZ;
    TargetPelvisOffset = FMath::Clamp(TargetPelvisOffset, -30.f, 0.f);

    PelvisOffset = FMath::FInterpTo(
        PelvisOffset,
        TargetPelvisOffset,
        GetWorld()->GetDeltaSeconds(),
        12.f
    );
}

// ─────────────────────────────────────────────────────────────────────────────
//  TraceFootToGround
// ─────────────────────────────────────────────────────────────────────────────

FVector UDinoSurvivorAnimInstance::TraceFootToGround(
    const FName& FootSocketName, float& OutHitZ) const
{
    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        OutHitZ = OwnerCharacter->GetActorLocation().Z;
        return OwnerCharacter->GetActorLocation();
    }

    FVector SocketLoc = Mesh->GetSocketLocation(FootSocketName);

    FVector TraceStart = SocketLoc + FVector(0.f, 0.f, FootIKTraceDistance);
    FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        Params
    );

    if (bHit)
    {
        OutHitZ = HitResult.ImpactPoint.Z;
        return HitResult.ImpactPoint;
    }

    OutHitZ = SocketLoc.Z;
    return SocketLoc;
}
