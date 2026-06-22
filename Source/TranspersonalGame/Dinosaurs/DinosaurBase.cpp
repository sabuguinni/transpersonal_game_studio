// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base implementation for all dinosaur pawns.
// Provides: movement, survival stats, territory logic, damage handling.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule sizing — overridden per species in child classes
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->JumpZVelocity = 0.0f;
    GetCharacterMovement()->AirControl = 0.0f;
    GetCharacterMovement()->GravityScale = 1.2f;

    bUseControllerRotationYaw = false;

    // Survival defaults
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    MaxHunger = 100.0f;
    CurrentHunger = 80.0f;
    MaxStamina = 100.0f;
    CurrentStamina = 100.0f;

    // Behaviour defaults
    WalkSpeed = 200.0f;
    RunSpeed = 600.0f;
    AttackDamage = 40.0f;
    AttackRange = 150.0f;
    DetectionRadius = 1500.0f;
    TerritoryRadius = 3000.0f;

    Species = EDinoSpecies::TRex;
    CurrentBehaviourState = EDinoBehaviourState::Idle;
    bIsAlive = true;
    bIsAggressive = false;
    bIsHunting = false;

    HungerDecayRate = 0.5f;
    StaminaRecoveryRate = 10.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Cache home territory origin
    TerritoryCenter = GetActorLocation();

    // Start hunger decay timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::TickHunger,
        2.0f,   // every 2 seconds
        true    // looping
    );

    // Start behaviour evaluation timer
    GetWorldTimerManager().SetTimer(
        BehaviourTimerHandle,
        this,
        &ADinosaurBase::EvaluateBehaviour,
        1.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina recovery when not running
    if (GetCharacterMovement()->Velocity.SizeSquared() < (WalkSpeed * WalkSpeed * 0.5f))
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRecoveryRate * DeltaTime);
    }
}

// ─── Health / Damage ──────────────────────────────────────────────────────────

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                 AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!bIsAlive) return 0.0f;

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    OnDamageTaken(ActualDamage, DamageCauser);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else if (!bIsAggressive && ActualDamage > 0.0f)
    {
        // Provoked — become aggressive toward attacker
        bIsAggressive = true;
        CurrentBehaviourState = EDinoBehaviourState::Aggressive;
        LastKnownThreatLocation = DamageCauser ? DamageCauser->GetActorLocation() : GetActorLocation();
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (!bIsAlive) return;

    bIsAlive = false;
    CurrentBehaviourState = EDinoBehaviourState::Dead;

    // Stop timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(BehaviourTimerHandle);

    // Disable movement
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    OnDeath();
}

// ─── Hunger ───────────────────────────────────────────────────────────────────

void ADinosaurBase::TickHunger()
{
    if (!bIsAlive) return;

    CurrentHunger = FMath::Max(0.0f, CurrentHunger - HungerDecayRate);

    if (CurrentHunger < 20.0f && !bIsHunting)
    {
        bIsHunting = true;
        CurrentBehaviourState = EDinoBehaviourState::Hunting;
    }
}

// ─── Behaviour Evaluation ─────────────────────────────────────────────────────

void ADinosaurBase::EvaluateBehaviour()
{
    if (!bIsAlive) return;

    // Distance from territory center
    const float DistFromHome = FVector::Dist(GetActorLocation(), TerritoryCenter);

    if (CurrentBehaviourState == EDinoBehaviourState::Dead) return;

    if (bIsAggressive)
    {
        CurrentBehaviourState = EDinoBehaviourState::Aggressive;
        return;
    }

    if (bIsHunting)
    {
        CurrentBehaviourState = EDinoBehaviourState::Hunting;
        return;
    }

    if (DistFromHome > TerritoryRadius)
    {
        CurrentBehaviourState = EDinoBehaviourState::Returning;
        return;
    }

    // Default idle/patrol cycle
    if (CurrentBehaviourState == EDinoBehaviourState::Idle)
    {
        // 30% chance to start patrolling each evaluation tick
        if (FMath::RandRange(0, 9) < 3)
        {
            CurrentBehaviourState = EDinoBehaviourState::Patrolling;
        }
    }
    else if (CurrentBehaviourState == EDinoBehaviourState::Patrolling)
    {
        // 20% chance to rest
        if (FMath::RandRange(0, 9) < 2)
        {
            CurrentBehaviourState = EDinoBehaviourState::Idle;
        }
    }
}

// ─── Blueprint-overridable events ─────────────────────────────────────────────

void ADinosaurBase::OnDamageTaken_Implementation(float DamageAmount, AActor* DamageCauser)
{
    // Default: nothing — override in Blueprint or child C++ class
}

void ADinosaurBase::OnDeath_Implementation()
{
    // Default: nothing — override in Blueprint or child C++ class
}

void ADinosaurBase::OnAttack_Implementation(AActor* Target)
{
    // Default: nothing — override in Blueprint or child C++ class
}

// ─── Attack ───────────────────────────────────────────────────────────────────

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || !bIsAlive) return;

    const float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget > AttackRange) return;

    // Apply damage
    UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);

    OnAttack(Target);
}

// ─── Getters ──────────────────────────────────────────────────────────────────

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ADinosaurBase::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (CurrentHunger / MaxHunger) : 0.0f;
}

float ADinosaurBase::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}
