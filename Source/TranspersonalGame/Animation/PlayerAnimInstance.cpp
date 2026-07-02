// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Implements the core AnimInstance for the survivor character:
// - Locomotion blend space (idle/walk/run/sprint)
// - Jump/fall/land state tracking
// - Foot IK ground adaptation
// - Combat overlay state

#include "PlayerAnimInstance.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsCrouching = false;
    bIsSprinting = false;

    // Foot IK defaults
    bEnableFootIK = true;
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    LeftFootOffset = FVector::ZeroVector;
    RightFootOffset = FVector::ZeroVector;
    IKTraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;

    // State machine defaults
    bIsAttacking = false;
    bIsDead = false;
    bIsInteracting = false;
    JumpStartedTime = 0.0f;
    LandedTime = 0.0f;

    // Survival state defaults
    StaminaNormalized = 1.0f;
    HealthNormalized = 1.0f;
    FearLevel = 0.0f;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache owner pawn
    OwnerCharacter = Cast<ATranspersonalCharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        CachedMovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("PlayerAnimInstance: Initialized for %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerAnimInstance: Owner is not ATranspersonalCharacter"));
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !CachedMovementComponent)
    {
        // Re-attempt cache on first valid frame
        OwnerCharacter = Cast<ATranspersonalCharacter>(TryGetPawnOwner());
        if (OwnerCharacter)
        {
            CachedMovementComponent = OwnerCharacter->GetCharacterMovement();
        }
        return;
    }

    UpdateLocomotionValues(DeltaSeconds);
    UpdateAirborneState(DeltaSeconds);
    UpdateSurvivalState(DeltaSeconds);

    if (bEnableFootIK && !bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        // Blend out foot IK when airborne
        LeftFootIKAlpha  = FMath::FInterpTo(LeftFootIKAlpha,  0.0f, DeltaSeconds, IKInterpSpeed);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, IKInterpSpeed);
    }
}

void UPlayerAnimInstance::UpdateLocomotionValues(float DeltaSeconds)
{
    // Velocity-based speed (horizontal only)
    FVector Velocity = CachedMovementComponent->Velocity;
    Velocity.Z = 0.0f;
    Speed = Velocity.Size();

    // Direction relative to actor forward (for strafe blending)
    if (Speed > 1.0f)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FRotator VelocityRot = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // Acceleration check (is the player pressing movement keys?)
    bIsAccelerating = CachedMovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;

    // Crouch state
    bIsCrouching = CachedMovementComponent->IsCrouching();

    // Sprint detection — speed above walk threshold (400 = walk, 600 = run, 800 = sprint)
    bIsSprinting = Speed > 650.0f;
}

void UPlayerAnimInstance::UpdateAirborneState(float DeltaSeconds)
{
    bool bWasInAir = bIsInAir;
    bIsInAir = CachedMovementComponent->IsFalling();

    // Detect jump start
    if (!bWasInAir && bIsInAir)
    {
        JumpStartedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        UE_LOG(LogTemp, Verbose, TEXT("PlayerAnimInstance: Jump started"));
    }

    // Detect landing
    if (bWasInAir && !bIsInAir)
    {
        LandedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        UE_LOG(LogTemp, Verbose, TEXT("PlayerAnimInstance: Landed"));
    }
}

void UPlayerAnimInstance::UpdateSurvivalState(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Read survival stats from the character
    // These properties are defined on ATranspersonalCharacter
    // We use safe property access via reflection to avoid hard coupling
    float Health   = 100.0f;
    float Stamina  = 100.0f;
    float Fear     = 0.0f;

    // Try to read via UE5 property system (safe fallback to defaults if not found)
    FFloatProperty* HealthProp   = FindFProperty<FFloatProperty>(OwnerCharacter->GetClass(), TEXT("Health"));
    FFloatProperty* StaminaProp  = FindFProperty<FFloatProperty>(OwnerCharacter->GetClass(), TEXT("Stamina"));
    FFloatProperty* FearProp     = FindFProperty<FFloatProperty>(OwnerCharacter->GetClass(), TEXT("FearLevel"));

    if (HealthProp)  Health  = HealthProp->GetPropertyValue_InContainer(OwnerCharacter);
    if (StaminaProp) Stamina = StaminaProp->GetPropertyValue_InContainer(OwnerCharacter);
    if (FearProp)    Fear    = FearProp->GetPropertyValue_InContainer(OwnerCharacter);

    HealthNormalized  = FMath::Clamp(Health  / 100.0f, 0.0f, 1.0f);
    StaminaNormalized = FMath::Clamp(Stamina / 100.0f, 0.0f, 1.0f);
    FearLevel         = FMath::Clamp(Fear    / 100.0f, 0.0f, 1.0f);
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    // Trace for left foot
    FVector LeftFootBoneLocation  = Mesh->GetSocketLocation(TEXT("foot_l"));
    FVector RightFootBoneLocation = Mesh->GetSocketLocation(TEXT("foot_r"));

    FVector LeftFootTarget  = TraceFootIK(LeftFootBoneLocation,  DeltaSeconds);
    FVector RightFootTarget = TraceFootIK(RightFootBoneLocation, DeltaSeconds);

    // Smooth interpolation of foot offsets
    LeftFootOffset  = FMath::VInterpTo(LeftFootOffset,  LeftFootTarget,  DeltaSeconds, IKInterpSpeed);
    RightFootOffset = FMath::VInterpTo(RightFootOffset, RightFootTarget, DeltaSeconds, IKInterpSpeed);

    // Alpha based on whether feet are on uneven terrain
    float LeftDelta  = FMath::Abs(LeftFootOffset.Z);
    float RightDelta = FMath::Abs(RightFootOffset.Z);

    LeftFootIKAlpha  = FMath::FInterpTo(LeftFootIKAlpha,  FMath::Clamp(LeftDelta  / 20.0f, 0.0f, 1.0f), DeltaSeconds, IKInterpSpeed);
    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, FMath::Clamp(RightDelta / 20.0f, 0.0f, 1.0f), DeltaSeconds, IKInterpSpeed);
}

FVector UPlayerAnimInstance::TraceFootIK(const FVector& FootWorldLocation, float DeltaSeconds)
{
    if (!GetWorld()) return FVector::ZeroVector;

    FVector TraceStart = FootWorldLocation + FVector(0.0f, 0.0f, IKTraceDistance);
    FVector TraceEnd   = FootWorldLocation - FVector(0.0f, 0.0f, IKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        // Return offset from foot bone to hit surface
        FVector Offset = HitResult.ImpactPoint - FootWorldLocation;
        return Offset;
    }

    return FVector::ZeroVector;
}

void UPlayerAnimInstance::SetAttackState(bool bAttacking)
{
    bIsAttacking = bAttacking;
}

void UPlayerAnimInstance::SetDeathState(bool bDead)
{
    bIsDead = bDead;
}

void UPlayerAnimInstance::SetInteractState(bool bInteracting)
{
    bIsInteracting = bInteracting;
}

void UPlayerAnimInstance::PlayAttackMontage(UAnimMontage* AttackMontage)
{
    if (AttackMontage && !bIsDead)
    {
        Montage_Play(AttackMontage, 1.0f);
        bIsAttacking = true;
    }
}

void UPlayerAnimInstance::PlayDeathMontage(UAnimMontage* DeathMontage)
{
    if (DeathMontage)
    {
        Montage_Play(DeathMontage, 1.0f);
        bIsDead = true;
    }
}

void UPlayerAnimInstance::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    Super::OnMontageEnded(Montage, bInterrupted);

    // Reset attack state when attack montage ends
    if (bIsAttacking && !bInterrupted)
    {
        bIsAttacking = false;
    }
}
