// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base implementation for all dinosaur pawns.
// Provides: movement, survival stats, basic AI perception hooks.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default survival stats
    MaxHealth        = 500.0f;
    CurrentHealth    = MaxHealth;
    MaxStamina       = 200.0f;
    CurrentStamina   = MaxStamina;
    MaxHunger        = 100.0f;
    CurrentHunger    = MaxHunger;
    WalkSpeed        = 300.0f;
    RunSpeed         = 700.0f;
    TurnRate         = 180.0f;
    bIsAggressive    = false;
    bIsFleeing       = false;
    TerritoryRadius  = 2000.0f;
    DetectionRadius  = 1500.0f;
    Species          = EDinoSpecies::Unknown;

    // Movement component defaults
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed          = WalkSpeed;
        GetCharacterMovement()->JumpZVelocity         = 0.0f;   // Dinos don't jump
        GetCharacterMovement()->AirControl            = 0.0f;
        GetCharacterMovement()->RotationRate          = FRotator(0.0f, TurnRate, 0.0f);
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->GravityScale          = 1.0f;
        GetCharacterMovement()->bUseControllerDesiredRotation = false;
    }

    // Capsule defaults (overridden per species in subclass constructors)
    GetCapsuleComponent()->InitCapsuleSize(88.0f, 96.0f);

    // Auto-possess by AI
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth  = MaxHealth;
    CurrentStamina = MaxStamina;
    CurrentHunger  = MaxHunger;

    // Hunger drain timer — every 10 seconds lose 1 hunger point
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::DrainHunger,
        10.0f,
        true
    );

    // Stamina recovery timer — every 2 seconds recover 5 stamina when not running
    GetWorldTimerManager().SetTimer(
        StaminaTimerHandle,
        this,
        &ADinosaurBase::RecoverStamina,
        2.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Clamp stats
    CurrentHealth  = FMath::Clamp(CurrentHealth,  0.0f, MaxHealth);
    CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
    CurrentHunger  = FMath::Clamp(CurrentHunger,  0.0f, MaxHunger);

    // Speed switching: aggressive/fleeing dinos run
    if (GetCharacterMovement())
    {
        float TargetSpeed = (bIsAggressive || bIsFleeing) ? RunSpeed : WalkSpeed;
        GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
    }

    // Death check
    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        Die();
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                 AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth -= ActualDamage;

    if (ActualDamage > 0.0f)
    {
        OnDinosaurHurt(ActualDamage, DamageCauser);
    }

    return ActualDamage;
}

void ADinosaurBase::OnDinosaurHurt_Implementation(float DamageAmount, AActor* DamageCauser)
{
    // Default: become aggressive when hurt
    if (!bIsDead && DamageAmount > 10.0f)
    {
        bIsAggressive = true;
    }
}

void ADinosaurBase::Die()
{
    if (bIsDead) return;
    bIsDead = true;

    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(StaminaTimerHandle);

    // Disable movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    OnDinosaurDied();

    // Destroy after 30 seconds (corpse despawn)
    SetLifeSpan(30.0f);
}

void ADinosaurBase::OnDinosaurDied_Implementation()
{
    // Override in Blueprint for death animation, sound, loot drop
}

void ADinosaurBase::SetAggressive(bool bAggressive)
{
    bIsAggressive = bAggressive;
    if (bAggressive)
    {
        bIsFleeing = false;
    }
}

void ADinosaurBase::SetFleeing(bool bFleeing)
{
    bIsFleeing = bFleeing;
    if (bFleeing)
    {
        bIsAggressive = false;
    }
}

bool ADinosaurBase::IsAlive() const
{
    return !bIsDead && CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ADinosaurBase::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}

float ADinosaurBase::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (CurrentHunger / MaxHunger) : 0.0f;
}

void ADinosaurBase::DrainHunger()
{
    if (bIsDead) return;
    CurrentHunger -= 1.0f;

    // Starving dino becomes desperate/aggressive
    if (CurrentHunger <= 10.0f && !bIsAggressive)
    {
        bIsAggressive = true;
    }
}

void ADinosaurBase::RecoverStamina()
{
    if (bIsDead) return;
    if (!bIsAggressive && !bIsFleeing)
    {
        CurrentStamina = FMath::Min(CurrentStamina + 5.0f, MaxStamina);
    }
}
