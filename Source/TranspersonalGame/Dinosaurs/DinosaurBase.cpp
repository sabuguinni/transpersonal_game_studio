// DinosaurBase.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260702_008
// Base class for all dinosaur pawns in the prehistoric survival game.
// Inherits from ACharacter to leverage UCharacterMovementComponent.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default species data
    SpeciesName = FText::FromString(TEXT("Unknown Dinosaur"));
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    ThreatRadius = 1500.0f;
    AttackRange = 200.0f;
    AttackDamage = 50.0f;
    bIsAggressive = false;
    bIsPredator = false;
    DinoSize = EEng_DinosaurSize::Medium;
    CurrentBehavior = EEng_DinosaurBehavior::Idle;

    // Configure movement
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->GravityScale = 1.0f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    }

    // Capsule sizing — medium dinosaur default
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    CurrentBehavior = EEng_DinosaurBehavior::Idle;

    // Apply walk speed from species data
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
    }

    // Start idle patrol timer
    GetWorldTimerManager().SetTimer(
        BehaviorUpdateTimer,
        this,
        &ADinosaurBase::UpdateBehavior,
        2.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Per-frame behavior logic delegated to UpdateBehavior (timer-driven)
}

void ADinosaurBase::TakeDamageFromPlayer(float DamageAmount, AActor* DamageInstigator)
{
    if (CurrentHealth <= 0.0f) return;

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    // Become aggressive when attacked
    if (!bIsAggressive && DamageInstigator)
    {
        bIsAggressive = true;
        ThreatTarget = DamageInstigator;
        SetBehavior(EEng_DinosaurBehavior::Aggressive);
    }

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
}

void ADinosaurBase::SetBehavior(EEng_DinosaurBehavior NewBehavior)
{
    CurrentBehavior = NewBehavior;

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    switch (NewBehavior)
    {
        case EEng_DinosaurBehavior::Idle:
            MoveComp->MaxWalkSpeed = WalkSpeed * 0.3f;
            break;
        case EEng_DinosaurBehavior::Patrolling:
            MoveComp->MaxWalkSpeed = WalkSpeed;
            break;
        case EEng_DinosaurBehavior::Fleeing:
            MoveComp->MaxWalkSpeed = RunSpeed;
            break;
        case EEng_DinosaurBehavior::Aggressive:
            MoveComp->MaxWalkSpeed = RunSpeed;
            break;
        case EEng_DinosaurBehavior::Feeding:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
        default:
            MoveComp->MaxWalkSpeed = WalkSpeed;
            break;
    }
}

void ADinosaurBase::UpdateBehavior()
{
    // Simple state machine — override in subclasses for species-specific AI
    if (CurrentHealth <= 0.0f) return;

    switch (CurrentBehavior)
    {
        case EEng_DinosaurBehavior::Idle:
            // 30% chance to start patrolling
            if (FMath::RandRange(0, 9) < 3)
            {
                SetBehavior(EEng_DinosaurBehavior::Patrolling);
            }
            break;

        case EEng_DinosaurBehavior::Patrolling:
            // 20% chance to return to idle
            if (FMath::RandRange(0, 9) < 2)
            {
                SetBehavior(EEng_DinosaurBehavior::Idle);
            }
            break;

        case EEng_DinosaurBehavior::Aggressive:
            // Stay aggressive while target is valid
            if (!ThreatTarget.IsValid() || ThreatTarget->IsActorBeingDestroyed())
            {
                bIsAggressive = false;
                ThreatTarget = nullptr;
                SetBehavior(EEng_DinosaurBehavior::Idle);
            }
            break;

        default:
            break;
    }
}

void ADinosaurBase::OnDeath()
{
    // Stop all timers
    GetWorldTimerManager().ClearTimer(BehaviorUpdateTimer);

    // Disable movement
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Broadcast death event (Blueprint-implementable)
    OnDinosaurDeath.Broadcast(this);

    // Destroy after 10 seconds (corpse lingers for resource gathering)
    SetLifeSpan(10.0f);
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f;
}
