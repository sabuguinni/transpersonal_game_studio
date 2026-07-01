// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full implementation of the player character AnimInstance with locomotion state machine,
// montage support (jump/attack/death), and foot IK integration.

#include "PlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Locomotion defaults
    Speed               = 0.0f;
    Direction           = 0.0f;
    bIsInAir            = false;
    bIsCrouching        = false;
    bIsSprinting        = false;
    bIsMoving           = false;

    // Survival stat defaults
    StaminaNormalized   = 1.0f;
    HealthNormalized    = 1.0f;
    FearNormalized      = 0.0f;

    // Foot IK defaults
    bFootIKEnabled      = true;
    LeftFootOffset      = FVector::ZeroVector;
    RightFootOffset     = FVector::ZeroVector;
    LeftFootRotation    = FRotator::ZeroRotator;
    RightFootRotation   = FRotator::ZeroRotator;
    PelvisOffset        = 0.0f;
    FootIKInterpSpeed   = 15.0f;

    // Montage slot names
    UpperBodySlotName   = FName("UpperBody");
    FullBodySlotName    = FName("FullBody");

    // Blend config
    LeanInterpSpeed     = 8.0f;
    LeanAmount          = 0.0f;
    AimPitch            = 0.0f;
    AimYaw              = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeInitializeAnimation
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("[PlayerAnimInstance] Initialized for: %s"), *OwnerCharacter->GetName());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeUpdateAnimation — called every frame
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        OwnerCharacter    = Cast<ACharacter>(TryGetPawnOwner());
        MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
        return;
    }

    UpdateLocomotionState(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateLean(DeltaSeconds);

    if (bFootIKEnabled)
    {
        UpdateFootIK(DeltaSeconds);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateLocomotionState
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    const FVector Velocity      = MovementComponent->Velocity;
    const FVector VelocityXY    = FVector(Velocity.X, Velocity.Y, 0.0f);

    Speed       = VelocityXY.Size();
    bIsInAir    = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsMoving   = Speed > 10.0f;

    // Sprint detection: speed above walk threshold (300 cm/s default walk = 600 sprint)
    bIsSprinting = Speed > 400.0f && !bIsInAir && !bIsCrouching;

    // Direction relative to actor forward (for strafe blend)
    if (bIsMoving)
    {
        const FRotator ActorRot  = OwnerCharacter->GetActorRotation();
        const FRotator VelRot    = UKismetMathLibrary::MakeRotFromX(VelocityXY);
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot).Yaw;
    }
    else
    {
        Direction = FMath::FInterpTo(Direction, 0.0f, DeltaSeconds, 5.0f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateAimOffset
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    const AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    const FRotator ControlRot   = Controller->GetControlRotation();
    const FRotator ActorRot     = OwnerCharacter->GetActorRotation();
    const FRotator DeltaRot     = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::ClampAngle(DeltaRot.Yaw,   -90.0f, 90.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateLean
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateLean(float DeltaSeconds)
{
    // Lean based on lateral acceleration
    if (!MovementComponent) return;

    const FVector Accel     = MovementComponent->GetCurrentAcceleration();
    const FVector LocalAccel = OwnerCharacter->GetActorTransform().InverseTransformVector(Accel);
    const float TargetLean  = FMath::Clamp(LocalAccel.Y / 600.0f, -1.0f, 1.0f);

    LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, LeanInterpSpeed);
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateFootIK — two-bone IK via line traces
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    // Trace parameters
    const float TraceDistance   = 80.0f;
    const float InterpSpeed     = FootIKInterpSpeed;

    // ── Left foot ──
    {
        const FName BoneName    = FName("foot_l");
        const FVector BoneLoc   = Mesh->GetSocketLocation(BoneName);
        const FVector TraceStart = FVector(BoneLoc.X, BoneLoc.Y, BoneLoc.Z + 30.0f);
        const FVector TraceEnd   = FVector(BoneLoc.X, BoneLoc.Y, BoneLoc.Z - TraceDistance);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            const FVector TargetOffset = FVector(0.0f, 0.0f, (Hit.Location.Z - BoneLoc.Z));
            LeftFootOffset = FMath::VInterpTo(LeftFootOffset, TargetOffset, DeltaSeconds, InterpSpeed);

            // Foot rotation from surface normal
            const FRotator TargetRot = FRotator(
                FMath::RadiansToDegrees(FMath::Atan2(Hit.Normal.X, Hit.Normal.Z)),
                0.0f,
                -FMath::RadiansToDegrees(FMath::Atan2(Hit.Normal.Y, Hit.Normal.Z))
            );
            LeftFootRotation = FMath::RInterpTo(LeftFootRotation, TargetRot, DeltaSeconds, InterpSpeed);
        }
        else
        {
            LeftFootOffset   = FMath::VInterpTo(LeftFootOffset, FVector::ZeroVector, DeltaSeconds, InterpSpeed);
            LeftFootRotation = FMath::RInterpTo(LeftFootRotation, FRotator::ZeroRotator, DeltaSeconds, InterpSpeed);
        }
    }

    // ── Right foot ──
    {
        const FName BoneName    = FName("foot_r");
        const FVector BoneLoc   = Mesh->GetSocketLocation(BoneName);
        const FVector TraceStart = FVector(BoneLoc.X, BoneLoc.Y, BoneLoc.Z + 30.0f);
        const FVector TraceEnd   = FVector(BoneLoc.X, BoneLoc.Y, BoneLoc.Z - TraceDistance);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            const FVector TargetOffset = FVector(0.0f, 0.0f, (Hit.Location.Z - BoneLoc.Z));
            RightFootOffset = FMath::VInterpTo(RightFootOffset, TargetOffset, DeltaSeconds, InterpSpeed);

            const FRotator TargetRot = FRotator(
                FMath::RadiansToDegrees(FMath::Atan2(Hit.Normal.X, Hit.Normal.Z)),
                0.0f,
                -FMath::RadiansToDegrees(FMath::Atan2(Hit.Normal.Y, Hit.Normal.Z))
            );
            RightFootRotation = FMath::RInterpTo(RightFootRotation, TargetRot, DeltaSeconds, InterpSpeed);
        }
        else
        {
            RightFootOffset   = FMath::VInterpTo(RightFootOffset, FVector::ZeroVector, DeltaSeconds, InterpSpeed);
            RightFootRotation = FMath::RInterpTo(RightFootRotation, FRotator::ZeroRotator, DeltaSeconds, InterpSpeed);
        }
    }

    // ── Pelvis offset — lowest foot drives pelvis down ──
    const float LowestFoot = FMath::Min(LeftFootOffset.Z, RightFootOffset.Z);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, LowestFoot, DeltaSeconds, InterpSpeed * 0.5f);
}

// ─────────────────────────────────────────────────────────────────────────────
// PlayMontageByType — Blueprint callable montage dispatcher
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::PlayMontageByType(EAnim_MontageType MontageType, float PlayRate)
{
    UAnimMontage* TargetMontage = nullptr;

    switch (MontageType)
    {
        case EAnim_MontageType::Jump:
            TargetMontage = JumpMontage;
            break;
        case EAnim_MontageType::MeleeAttack:
            TargetMontage = MeleeAttackMontage;
            break;
        case EAnim_MontageType::ThrowSpear:
            TargetMontage = ThrowSpearMontage;
            break;
        case EAnim_MontageType::Death:
            TargetMontage = DeathMontage;
            break;
        case EAnim_MontageType::Interact:
            TargetMontage = InteractMontage;
            break;
        case EAnim_MontageType::Eat:
            TargetMontage = EatMontage;
            break;
        case EAnim_MontageType::Drink:
            TargetMontage = DrinkMontage;
            break;
        case EAnim_MontageType::Craft:
            TargetMontage = CraftMontage;
            break;
        default:
            break;
    }

    if (TargetMontage)
    {
        Montage_Play(TargetMontage, PlayRate);
        UE_LOG(LogTemp, Log, TEXT("[PlayerAnimInstance] Playing montage: %s at rate %.2f"),
            *TargetMontage->GetName(), PlayRate);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[PlayerAnimInstance] Montage not assigned for type %d"),
            static_cast<int32>(MontageType));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateSurvivalStats — called by character to push survival data into anim
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateSurvivalStats(float Health, float MaxHealth,
                                               float Stamina, float MaxStamina,
                                               float Fear, float MaxFear)
{
    HealthNormalized  = (MaxHealth  > 0.0f) ? FMath::Clamp(Health  / MaxHealth,  0.0f, 1.0f) : 1.0f;
    StaminaNormalized = (MaxStamina > 0.0f) ? FMath::Clamp(Stamina / MaxStamina, 0.0f, 1.0f) : 1.0f;
    FearNormalized    = (MaxFear    > 0.0f) ? FMath::Clamp(Fear    / MaxFear,    0.0f, 1.0f) : 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// IsPlayingMontageOfType
// ─────────────────────────────────────────────────────────────────────────────

bool UPlayerAnimInstance::IsPlayingMontageOfType(EAnim_MontageType MontageType) const
{
    UAnimMontage* TargetMontage = nullptr;

    switch (MontageType)
    {
        case EAnim_MontageType::Jump:        TargetMontage = JumpMontage;        break;
        case EAnim_MontageType::MeleeAttack: TargetMontage = MeleeAttackMontage; break;
        case EAnim_MontageType::ThrowSpear:  TargetMontage = ThrowSpearMontage;  break;
        case EAnim_MontageType::Death:       TargetMontage = DeathMontage;       break;
        case EAnim_MontageType::Interact:    TargetMontage = InteractMontage;    break;
        case EAnim_MontageType::Eat:         TargetMontage = EatMontage;         break;
        case EAnim_MontageType::Drink:       TargetMontage = DrinkMontage;       break;
        case EAnim_MontageType::Craft:       TargetMontage = CraftMontage;       break;
        default:                             break;
    }

    if (!TargetMontage) return false;
    return Montage_IsPlaying(TargetMontage);
}
