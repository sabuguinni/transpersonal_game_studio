// DinosaurBase.cpp — Implementation of the base dinosaur class
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260628_012
// All dinosaur species inherit from this class.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species in child classes
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.MoveSpeed;
        MoveComp->MaxWalkSpeedCrouched = Stats.MoveSpeed * 0.5f;
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->GravityScale = 1.5f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    }

    // Mesh offset — child classes set the actual SkeletalMesh asset
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Do not use controller rotation — movement component handles orientation
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Sync movement speed from stats
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.MoveSpeed;
    }

    // Start in Idle state
    BehaviorState = EEng_DinosaurBehaviorState::Idle;
    bIsInCombat   = false;
    CurrentTarget = nullptr;
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Hunger decay over time
    HungerTickAccumulator += DeltaTime;
    if (HungerTickAccumulator >= 1.0f)
    {
        HungerTickAccumulator = 0.0f;
        Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - Stats.HungerDecayRate);

        // Very hungry carnivores become more aggressive
        if (Diet == EEng_DinosaurDiet::Carnivore && Stats.Hunger < 20.0f)
        {
            bIsAggressive = true;
        }
    }
}

// ─── ApplyDamage ─────────────────────────────────────────────────────────────

void ADinosaurBase::ApplyDamage(float Amount, AActor* Causer)
{
    if (!IsAlive()) return;

    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - Amount);
    OnTakeDamage(Amount, Causer);

    // Transition to combat if hit and not already fighting
    if (!bIsInCombat && Causer)
    {
        bIsInCombat   = true;
        CurrentTarget = Causer;
        SetBehaviorState(EEng_DinosaurBehaviorState::Attacking);
        OnTargetDetected(Causer);
    }

    if (!IsAlive())
    {
        OnDeath();
    }
}

// ─── GetHealthPercent ─────────────────────────────────────────────────────────

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f) return 0.0f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

// ─── SetBehaviorState ─────────────────────────────────────────────────────────

void ADinosaurBase::SetBehaviorState(EEng_DinosaurBehaviorState NewState)
{
    if (BehaviorState == NewState) return;
    BehaviorState = NewState;

    // Sync movement speed to state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (NewState)
        {
            case EEng_DinosaurBehaviorState::Hunting:
            case EEng_DinosaurBehaviorState::Attacking:
                MoveComp->MaxWalkSpeed = Stats.SprintSpeed;
                break;
            case EEng_DinosaurBehaviorState::Fleeing:
                MoveComp->MaxWalkSpeed = Stats.SprintSpeed * 1.1f;
                break;
            case EEng_DinosaurBehaviorState::Sleeping:
                MoveComp->MaxWalkSpeed = 0.0f;
                break;
            default:
                MoveComp->MaxWalkSpeed = Stats.MoveSpeed;
                break;
        }
    }
}

// ─── CanSeeTarget ─────────────────────────────────────────────────────────────

bool ADinosaurBase::CanSeeTarget(AActor* Target) const
{
    if (!Target) return false;

    const FVector MyLocation     = GetActorLocation();
    const FVector TargetLocation = Target->GetActorLocation();
    const float   Distance       = FVector::Dist(MyLocation, TargetLocation);

    if (Distance > SensoryData.SightRange) return false;

    // Angle check
    const FVector ToTarget    = (TargetLocation - MyLocation).GetSafeNormal();
    const FVector ForwardDir  = GetActorForwardVector();
    const float   DotProduct  = FVector::DotProduct(ForwardDir, ToTarget);
    const float   HalfAngle   = FMath::DegreesToRadians(SensoryData.SightAngleDegrees * 0.5f);
    const float   CosHalfAngle = FMath::Cos(HalfAngle);

    return DotProduct >= CosHalfAngle;
}

// ─── CanHearTarget ────────────────────────────────────────────────────────────

bool ADinosaurBase::CanHearTarget(AActor* Target) const
{
    if (!Target) return false;

    const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    return Distance <= SensoryData.HearingRange;
}

// ─── IsAlive ─────────────────────────────────────────────────────────────────

bool ADinosaurBase::IsAlive() const
{
    return Stats.CurrentHealth > 0.0f;
}

// ─── BlueprintNativeEvent Implementations ────────────────────────────────────

void ADinosaurBase::OnTakeDamage_Implementation(float DamageAmount, AActor* DamageCauser)
{
    // Base implementation — child classes override for species-specific reactions
    // e.g., TRex roars, Raptor calls pack, Brachiosaurus flees
}

void ADinosaurBase::OnDeath_Implementation()
{
    // Base implementation — disable collision, stop movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }
    SetActorEnableCollision(false);
    bIsInCombat   = false;
    CurrentTarget = nullptr;
}

void ADinosaurBase::OnTargetDetected_Implementation(AActor* DetectedTarget)
{
    // Base implementation — child classes trigger alert sounds, animations, etc.
    CurrentTarget = DetectedTarget;
    bIsInCombat   = true;
}

void ADinosaurBase::OnTargetLost_Implementation()
{
    // Base implementation — return to patrol or idle
    CurrentTarget = nullptr;
    bIsInCombat   = false;
    SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
}
