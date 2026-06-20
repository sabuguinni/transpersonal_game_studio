// SurvivalComponent.cpp
// Core Systems Programmer #03 — P3 Character System
// Full implementation of survival stat drain, damage, recovery, and status evaluation

#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance

    CurrentStatus = ECore_SurvivalStatus::Healthy;
    bIsSprinting = false;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialise stats to full
    Stats.Health    = Stats.MaxHealth;
    Stats.Hunger    = Stats.MaxHunger;
    Stats.Thirst    = Stats.MaxThirst;
    Stats.Stamina   = Stats.MaxStamina;
    Stats.Fear      = 0.0f;
    Stats.Temperature = 37.0f;

    UpdateStatus();
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive())
    {
        return;
    }

    // Drain hunger and thirst over time
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - HungerDrainRate * DeltaTime);
    Stats.Thirst = FMath::Max(0.0f, Stats.Thirst - ThirstDrainRate * DeltaTime);

    // Apply damage from starvation / dehydration
    ApplyHungerDamage(DeltaTime);
    ApplyThirstDamage(DeltaTime);

    // Stamina recovery when not sprinting
    if (!bIsSprinting)
    {
        RecoverStamina(DeltaTime);
    }
    else
    {
        // Drain stamina while sprinting
        Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - StaminaDrainRate * DeltaTime);

        // Force stop sprint if stamina depleted
        if (Stats.Stamina <= 0.0f)
        {
            bIsSprinting = false;
        }
    }

    // Fear naturally decays over time
    DecayFear(DeltaTime);

    ClampStats();
    UpdateStatus();
}

// ─── Public API ──────────────────────────────────────────────────────────────

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (!IsAlive() || Amount <= 0.0f)
    {
        return;
    }

    Stats.Health = FMath::Max(0.0f, Stats.Health - Amount);
    UpdateStatus();
}

void USurvivalComponent::Heal(float Amount)
{
    if (!IsAlive() || Amount <= 0.0f)
    {
        return;
    }

    Stats.Health = FMath::Min(Stats.MaxHealth, Stats.Health + Amount);
    UpdateStatus();
}

void USurvivalComponent::Eat(float NutritionValue)
{
    if (!IsAlive() || NutritionValue <= 0.0f)
    {
        return;
    }

    Stats.Hunger = FMath::Min(Stats.MaxHunger, Stats.Hunger + NutritionValue);
    UpdateStatus();
}

void USurvivalComponent::Drink(float HydrationValue)
{
    if (!IsAlive() || HydrationValue <= 0.0f)
    {
        return;
    }

    Stats.Thirst = FMath::Min(Stats.MaxThirst, Stats.Thirst + HydrationValue);
    UpdateStatus();
}

void USurvivalComponent::DrainStamina(float Amount)
{
    if (!IsAlive() || Amount <= 0.0f)
    {
        return;
    }

    Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - Amount);
}

void USurvivalComponent::AddFear(float Amount)
{
    if (!IsAlive() || Amount <= 0.0f)
    {
        return;
    }

    Stats.Fear = FMath::Min(Stats.MaxFear, Stats.Fear + Amount);
    UpdateStatus();
}

bool USurvivalComponent::IsAlive() const
{
    return Stats.Health > 0.0f && CurrentStatus != ECore_SurvivalStatus::Dead;
}

bool USurvivalComponent::IsSprinting() const
{
    return bIsSprinting;
}

void USurvivalComponent::SetSprinting(bool bSprint)
{
    // Cannot sprint if exhausted or dead
    if (!IsAlive())
    {
        bIsSprinting = false;
        return;
    }

    if (bSprint && Stats.Stamina <= 0.0f)
    {
        bIsSprinting = false;
        return;
    }

    bIsSprinting = bSprint;
}

float USurvivalComponent::GetHealthPercent() const
{
    return (Stats.MaxHealth > 0.0f) ? (Stats.Health / Stats.MaxHealth) : 0.0f;
}

float USurvivalComponent::GetHungerPercent() const
{
    return (Stats.MaxHunger > 0.0f) ? (Stats.Hunger / Stats.MaxHunger) : 0.0f;
}

float USurvivalComponent::GetThirstPercent() const
{
    return (Stats.MaxThirst > 0.0f) ? (Stats.Thirst / Stats.MaxThirst) : 0.0f;
}

float USurvivalComponent::GetStaminaPercent() const
{
    return (Stats.MaxStamina > 0.0f) ? (Stats.Stamina / Stats.MaxStamina) : 0.0f;
}

float USurvivalComponent::GetFearPercent() const
{
    return (Stats.MaxFear > 0.0f) ? (Stats.Fear / Stats.MaxFear) : 0.0f;
}

ECore_SurvivalStatus USurvivalComponent::GetCurrentStatus() const
{
    return CurrentStatus;
}

// ─── Private helpers ─────────────────────────────────────────────────────────

void USurvivalComponent::UpdateStatus()
{
    if (Stats.Health <= 0.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Dead;
        return;
    }

    if (Stats.Health <= CriticalHealthThreshold)
    {
        CurrentStatus = ECore_SurvivalStatus::Critical;
        return;
    }

    if (Stats.Fear >= Stats.MaxFear * 0.8f)
    {
        CurrentStatus = ECore_SurvivalStatus::Terrified;
        return;
    }

    if (Stats.Temperature < 35.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Hypothermic;
        return;
    }

    if (Stats.Temperature > 40.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Hyperthermic;
        return;
    }

    if (Stats.Stamina <= 0.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Exhausted;
        return;
    }

    if (Stats.Thirst <= ThirstDamageThreshold)
    {
        CurrentStatus = ECore_SurvivalStatus::Thirsty;
        return;
    }

    if (Stats.Hunger <= HungerDamageThreshold)
    {
        CurrentStatus = ECore_SurvivalStatus::Hungry;
        return;
    }

    CurrentStatus = ECore_SurvivalStatus::Healthy;
}

void USurvivalComponent::ApplyHungerDamage(float DeltaTime)
{
    if (Stats.Hunger <= HungerDamageThreshold)
    {
        // Starving — deal 1 HP/s damage
        const float StarvationDamage = 1.0f * DeltaTime;
        Stats.Health = FMath::Max(0.0f, Stats.Health - StarvationDamage);
    }
}

void USurvivalComponent::ApplyThirstDamage(float DeltaTime)
{
    if (Stats.Thirst <= ThirstDamageThreshold)
    {
        // Dehydrated — deal 2 HP/s damage (more urgent than hunger)
        const float DehydrationDamage = 2.0f * DeltaTime;
        Stats.Health = FMath::Max(0.0f, Stats.Health - DehydrationDamage);
    }
}

void USurvivalComponent::RecoverStamina(float DeltaTime)
{
    // Only recover stamina if not starving or dehydrated
    if (Stats.Hunger > HungerDamageThreshold && Stats.Thirst > ThirstDamageThreshold)
    {
        Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + StaminaRecoveryRate * DeltaTime);
    }
}

void USurvivalComponent::DecayFear(float DeltaTime)
{
    if (Stats.Fear > 0.0f)
    {
        Stats.Fear = FMath::Max(0.0f, Stats.Fear - FearDecayRate * DeltaTime);
    }
}

void USurvivalComponent::ClampStats()
{
    Stats.Health    = FMath::Clamp(Stats.Health,    0.0f, Stats.MaxHealth);
    Stats.Hunger    = FMath::Clamp(Stats.Hunger,    0.0f, Stats.MaxHunger);
    Stats.Thirst    = FMath::Clamp(Stats.Thirst,    0.0f, Stats.MaxThirst);
    Stats.Stamina   = FMath::Clamp(Stats.Stamina,   0.0f, Stats.MaxStamina);
    Stats.Fear      = FMath::Clamp(Stats.Fear,      0.0f, Stats.MaxFear);
    Stats.Temperature = FMath::Clamp(Stats.Temperature, 20.0f, 45.0f);
}
