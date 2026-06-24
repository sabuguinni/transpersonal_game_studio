#include "SurvivalComponent.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

USurvivalComponent::USurvivalComponent()
{
    // Tick at 1 Hz — stat drains don't need per-frame precision
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    // Stats initialise to defaults defined in the struct (100/100/100/100/0)
    bIsAlive = true;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAlive)
    {
        return;
    }

    TickNaturalDrains(DeltaTime);
    CheckCriticalThresholds();
}

// ─────────────────────────────────────────────────────────────────────────────
// Stat Accessors
// ─────────────────────────────────────────────────────────────────────────────

FCore_SurvivalStats USurvivalComponent::GetCurrentStats() const
{
    return Stats;
}

// ─────────────────────────────────────────────────────────────────────────────
// Stat Modifiers
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return;
    }

    Stats.Health = FMath::Max(0.f, Stats.Health - Amount);
    ClampStats();

    if (Stats.Health <= 0.f)
    {
        bIsAlive = false;
        OnPlayerDeath.Broadcast();
    }
}

void USurvivalComponent::Eat(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return;
    }

    Stats.Hunger = FMath::Min(100.f, Stats.Hunger + Amount);

    // Reset critical flag so it can fire again if hunger drops again
    if (Stats.Hunger > CriticalHungerThreshold)
    {
        bHungerCriticalFired = false;
    }
}

void USurvivalComponent::Drink(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return;
    }

    Stats.Thirst = FMath::Min(100.f, Stats.Thirst + Amount);

    if (Stats.Thirst > CriticalThirstThreshold)
    {
        bThirstCriticalFired = false;
    }
}

void USurvivalComponent::ConsumeStamina(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return;
    }

    Stats.Stamina = FMath::Max(0.f, Stats.Stamina - Amount);
}

void USurvivalComponent::RecoverStamina(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return;
    }

    Stats.Stamina = FMath::Min(100.f, Stats.Stamina + Amount);

    if (Stats.Stamina > CriticalStaminaThreshold)
    {
        bStaminaCriticalFired = false;
    }
}

void USurvivalComponent::IncreaseFear(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return;
    }

    Stats.Fear = FMath::Min(100.f, Stats.Fear + Amount);
}

void USurvivalComponent::DecreaseFear(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return;
    }

    Stats.Fear = FMath::Max(0.f, Stats.Fear - Amount);

    if (Stats.Fear < HighFearThreshold)
    {
        bFearHighFired = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::TickNaturalDrains(float DeltaTime)
{
    // Hunger drain
    Stats.Hunger = FMath::Max(0.f, Stats.Hunger - HungerDrainRate * DeltaTime);

    // Thirst drain (faster than hunger — dehydration kills quicker)
    Stats.Thirst = FMath::Max(0.f, Stats.Thirst - ThirstDrainRate * DeltaTime);

    // Stamina passive recovery (only when not sprinting — character calls
    // ConsumeStamina() each frame during sprint, which overrides this)
    Stats.Stamina = FMath::Min(100.f, Stats.Stamina + StaminaRecoveryRate * DeltaTime);

    // Fear natural decay (calm environment, no predators nearby)
    Stats.Fear = FMath::Max(0.f, Stats.Fear - FearDecayRate * DeltaTime);

    // Starvation damage
    if (Stats.Hunger <= 0.f)
    {
        Stats.Health = FMath::Max(0.f, Stats.Health - StarvationDamageRate * DeltaTime);
        if (Stats.Health <= 0.f && bIsAlive)
        {
            bIsAlive = false;
            OnPlayerDeath.Broadcast();
        }
    }

    // Dehydration damage (more severe than starvation)
    if (Stats.Thirst <= 0.f)
    {
        Stats.Health = FMath::Max(0.f, Stats.Health - DehydrationDamageRate * DeltaTime);
        if (Stats.Health <= 0.f && bIsAlive)
        {
            bIsAlive = false;
            OnPlayerDeath.Broadcast();
        }
    }
}

void USurvivalComponent::CheckCriticalThresholds()
{
    // Health critical
    if (Stats.Health <= CriticalHealthThreshold && !bHealthCriticalFired)
    {
        bHealthCriticalFired = true;
        OnStatCritical.Broadcast(ECore_SurvivalStat::Health, Stats.Health);
    }
    else if (Stats.Health > CriticalHealthThreshold)
    {
        bHealthCriticalFired = false;
    }

    // Hunger critical
    if (Stats.Hunger <= CriticalHungerThreshold && !bHungerCriticalFired)
    {
        bHungerCriticalFired = true;
        OnStatCritical.Broadcast(ECore_SurvivalStat::Hunger, Stats.Hunger);
    }

    // Thirst critical
    if (Stats.Thirst <= CriticalThirstThreshold && !bThirstCriticalFired)
    {
        bThirstCriticalFired = true;
        OnStatCritical.Broadcast(ECore_SurvivalStat::Thirst, Stats.Thirst);
    }

    // Stamina critical
    if (Stats.Stamina <= CriticalStaminaThreshold && !bStaminaCriticalFired)
    {
        bStaminaCriticalFired = true;
        OnStatCritical.Broadcast(ECore_SurvivalStat::Stamina, Stats.Stamina);
    }

    // High fear
    if (Stats.Fear >= HighFearThreshold && !bFearHighFired)
    {
        bFearHighFired = true;
        OnStatCritical.Broadcast(ECore_SurvivalStat::Fear, Stats.Fear);
    }
}

void USurvivalComponent::ClampStats()
{
    Stats.Health  = FMath::Clamp(Stats.Health,  0.f, 100.f);
    Stats.Hunger  = FMath::Clamp(Stats.Hunger,  0.f, 100.f);
    Stats.Thirst  = FMath::Clamp(Stats.Thirst,  0.f, 100.f);
    Stats.Stamina = FMath::Clamp(Stats.Stamina, 0.f, 100.f);
    Stats.Fear    = FMath::Clamp(Stats.Fear,    0.f, 100.f);
}
