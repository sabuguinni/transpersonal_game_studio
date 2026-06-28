// SurvivalComponent.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Full implementation of USurvivalComponent

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick at 10Hz for smooth stamina regen

    CurrentStatus = ECore_SurvivalStatus::Healthy;
    bIsDead = false;
    bIsSprinting = false;
    TimeSinceLastDrain = 0.0f;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize stats to full
    Stats.Health = Stats.MaxHealth;
    Stats.Hunger = Stats.MaxHunger;
    Stats.Thirst = Stats.MaxThirst;
    Stats.Stamina = Stats.MaxStamina;
    Stats.Fear = 0.0f;
    Stats.BodyTemperature = 37.0f;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead)
    {
        return;
    }

    TickSurvivalDrain(DeltaTime);
    UpdateSurvivalStatus();
}

void USurvivalComponent::TickSurvivalDrain(float DeltaTime)
{
    // Stamina — drain while sprinting, regen while idle
    if (bIsSprinting && Stats.Stamina > 0.0f)
    {
        Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - StaminaDrainRate * DeltaTime);
        BroadcastStatChange(TEXT("Stamina"), Stats.Stamina);
    }
    else if (!bIsSprinting && Stats.Stamina < Stats.MaxStamina)
    {
        Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + StaminaRegenRate * DeltaTime);
        BroadcastStatChange(TEXT("Stamina"), Stats.Stamina);
    }

    // Fear decay — fear reduces over time when not threatened
    if (Stats.Fear > 0.0f)
    {
        Stats.Fear = FMath::Max(0.0f, Stats.Fear - FearDecayRate * DeltaTime);
        BroadcastStatChange(TEXT("Fear"), Stats.Fear);
    }

    // Hunger and thirst drain every DrainTickInterval seconds (not every frame)
    TimeSinceLastDrain += DeltaTime;
    if (TimeSinceLastDrain >= DrainTickInterval)
    {
        TimeSinceLastDrain = 0.0f;

        // Drain hunger
        if (Stats.Hunger > 0.0f)
        {
            Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - HungerDrainRate);
            BroadcastStatChange(TEXT("Hunger"), Stats.Hunger);
        }
        else
        {
            // Starving — apply health damage
            ApplyHungerDamage();
        }

        // Drain thirst
        if (Stats.Thirst > 0.0f)
        {
            Stats.Thirst = FMath::Max(0.0f, Stats.Thirst - ThirstDrainRate);
            BroadcastStatChange(TEXT("Thirst"), Stats.Thirst);
        }
        else
        {
            // Dehydrated — apply health damage (faster than starvation)
            ApplyThirstDamage();
        }

        // Temperature effects
        ApplyTemperatureDamage(DrainTickInterval);
    }
}

void USurvivalComponent::UpdateSurvivalStatus()
{
    if (bIsDead)
    {
        CurrentStatus = ECore_SurvivalStatus::Dead;
        return;
    }

    ECore_SurvivalStatus NewStatus = ECore_SurvivalStatus::Healthy;

    if (Stats.Health <= CriticalHealthThreshold)
    {
        NewStatus = ECore_SurvivalStatus::Critical;
    }
    else if (Stats.Thirst <= CriticalThirstThreshold)
    {
        NewStatus = ECore_SurvivalStatus::Thirsty;
    }
    else if (Stats.Hunger <= CriticalHungerThreshold)
    {
        NewStatus = ECore_SurvivalStatus::Hungry;
    }
    else if (Stats.Stamina <= 5.0f)
    {
        NewStatus = ECore_SurvivalStatus::Exhausted;
    }
    else if (Stats.BodyTemperature < 35.0f)
    {
        NewStatus = ECore_SurvivalStatus::Freezing;
    }
    else if (Stats.BodyTemperature > 40.0f)
    {
        NewStatus = ECore_SurvivalStatus::Overheating;
    }

    if (NewStatus != CurrentStatus)
    {
        CurrentStatus = NewStatus;
        OnSurvivalStatusChanged.Broadcast(CurrentStatus);
    }
}

void USurvivalComponent::ApplyHungerDamage()
{
    // Starvation: 1 HP per second
    const float StarveRate = 1.0f;
    ApplyDamage(StarveRate);
}

void USurvivalComponent::ApplyThirstDamage()
{
    // Dehydration: 2 HP per second (faster than starvation)
    const float DehydrateRate = 2.0f;
    ApplyDamage(DehydrateRate);
}

void USurvivalComponent::ApplyTemperatureDamage(float DeltaTime)
{
    // Body temperature drifts toward ambient
    const float TempDriftRate = 0.1f;
    Stats.BodyTemperature = FMath::FInterpTo(Stats.BodyTemperature, Stats.AmbientTemperature, DeltaTime, TempDriftRate);

    // Hypothermia: below 35°C
    if (Stats.BodyTemperature < 35.0f)
    {
        const float HypothermiaDamage = (35.0f - Stats.BodyTemperature) * 0.5f;
        ApplyDamage(HypothermiaDamage * DeltaTime);
    }
    // Hyperthermia: above 40°C
    else if (Stats.BodyTemperature > 40.0f)
    {
        const float HyperthermaDamage = (Stats.BodyTemperature - 40.0f) * 0.5f;
        ApplyDamage(HyperthermaDamage * DeltaTime);
    }
}

void USurvivalComponent::BroadcastStatChange(FName StatName, float NewValue)
{
    OnStatChanged.Broadcast(StatName, NewValue);
}

// --- Public API ---

bool USurvivalComponent::ApplyDamage(float DamageAmount)
{
    if (bIsDead || DamageAmount <= 0.0f)
    {
        return false;
    }

    Stats.Health = FMath::Max(0.0f, Stats.Health - DamageAmount);
    BroadcastStatChange(TEXT("Health"), Stats.Health);

    if (Stats.Health <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        CurrentStatus = ECore_SurvivalStatus::Dead;
        OnSurvivalStatusChanged.Broadcast(ECore_SurvivalStatus::Dead);
        OnPlayerDied.Broadcast();
        return true;
    }

    return false;
}

void USurvivalComponent::HealHealth(float HealAmount)
{
    if (bIsDead || HealAmount <= 0.0f)
    {
        return;
    }

    Stats.Health = FMath::Min(Stats.MaxHealth, Stats.Health + HealAmount);
    BroadcastStatChange(TEXT("Health"), Stats.Health);
}

void USurvivalComponent::ConsumeFood(float NutritionValue)
{
    if (NutritionValue <= 0.0f)
    {
        return;
    }

    Stats.Hunger = FMath::Min(Stats.MaxHunger, Stats.Hunger + NutritionValue);
    BroadcastStatChange(TEXT("Hunger"), Stats.Hunger);
}

void USurvivalComponent::DrinkWater(float HydrationValue)
{
    if (HydrationValue <= 0.0f)
    {
        return;
    }

    Stats.Thirst = FMath::Min(Stats.MaxThirst, Stats.Thirst + HydrationValue);
    BroadcastStatChange(TEXT("Thirst"), Stats.Thirst);
}

bool USurvivalComponent::UseStamina(float StaminaCost)
{
    if (Stats.Stamina < StaminaCost)
    {
        return false; // Not enough stamina
    }

    Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - StaminaCost);
    BroadcastStatChange(TEXT("Stamina"), Stats.Stamina);
    return true;
}

void USurvivalComponent::AddFear(float FearAmount)
{
    if (FearAmount <= 0.0f)
    {
        return;
    }

    Stats.Fear = FMath::Min(Stats.MaxFear, Stats.Fear + FearAmount);
    BroadcastStatChange(TEXT("Fear"), Stats.Fear);
}

void USurvivalComponent::ReduceFear(float FearAmount)
{
    if (FearAmount <= 0.0f)
    {
        return;
    }

    Stats.Fear = FMath::Max(0.0f, Stats.Fear - FearAmount);
    BroadcastStatChange(TEXT("Fear"), Stats.Fear);
}

void USurvivalComponent::SetAmbientTemperature(float Temperature)
{
    Stats.AmbientTemperature = Temperature;
}

void USurvivalComponent::SetSprinting(bool bNewIsSprinting)
{
    bIsSprinting = bNewIsSprinting;
}

float USurvivalComponent::GetHealthNormalized() const
{
    return (Stats.MaxHealth > 0.0f) ? (Stats.Health / Stats.MaxHealth) : 0.0f;
}

float USurvivalComponent::GetHungerNormalized() const
{
    return (Stats.MaxHunger > 0.0f) ? (Stats.Hunger / Stats.MaxHunger) : 0.0f;
}

float USurvivalComponent::GetThirstNormalized() const
{
    return (Stats.MaxThirst > 0.0f) ? (Stats.Thirst / Stats.MaxThirst) : 0.0f;
}

float USurvivalComponent::GetStaminaNormalized() const
{
    return (Stats.MaxStamina > 0.0f) ? (Stats.Stamina / Stats.MaxStamina) : 0.0f;
}

float USurvivalComponent::GetFearNormalized() const
{
    return (Stats.MaxFear > 0.0f) ? (Stats.Fear / Stats.MaxFear) : 0.0f;
}

bool USurvivalComponent::IsAlive() const
{
    return !bIsDead && Stats.Health > 0.0f;
}
