// SurvivalComponent.cpp
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260623_001
// Prehistoric survival simulation: hunger, thirst, stamina, temperature, fear.
// Wires into BiomeManager (ambient temperature + biome modifiers).

#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // We manage our own interval

    // Sensible defaults
    InitialStats.Health         = 100.f;
    InitialStats.MaxHealth      = 100.f;
    InitialStats.Hunger         = 100.f;
    InitialStats.MaxHunger      = 100.f;
    InitialStats.Thirst         = 100.f;
    InitialStats.MaxThirst      = 100.f;
    InitialStats.Stamina        = 100.f;
    InitialStats.MaxStamina     = 100.f;
    InitialStats.Fear           = 0.f;
    InitialStats.BodyTemperature    = 37.f;
    InitialStats.AmbientTemperature = 22.f;
}

// ---------------------------------------------------------------------------
// BeginPlay
// ---------------------------------------------------------------------------

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Copy initial stats to runtime stats
    Stats = InitialStats;
    CurrentStatus = ECore_SurvivalStatus::Healthy;
    AccumulatedTime = 0.f;
}

// ---------------------------------------------------------------------------
// TickComponent
// ---------------------------------------------------------------------------

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableSurvivalTick)
    {
        return;
    }

    AccumulatedTime += DeltaTime;

    if (AccumulatedTime >= TickIntervalSeconds)
    {
        TickSurvivalStats(AccumulatedTime);
        UpdateTemperature(AccumulatedTime);
        UpdateStatus();
        AccumulatedTime = 0.f;
    }
}

// ---------------------------------------------------------------------------
// Stat Queries
// ---------------------------------------------------------------------------

float USurvivalComponent::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.f) return 0.f;
    return Stats.Health / Stats.MaxHealth;
}

float USurvivalComponent::GetHungerPercent() const
{
    if (Stats.MaxHunger <= 0.f) return 0.f;
    return Stats.Hunger / Stats.MaxHunger;
}

float USurvivalComponent::GetThirstPercent() const
{
    if (Stats.MaxThirst <= 0.f) return 0.f;
    return Stats.Thirst / Stats.MaxThirst;
}

float USurvivalComponent::GetStaminaPercent() const
{
    if (Stats.MaxStamina <= 0.f) return 0.f;
    return Stats.Stamina / Stats.MaxStamina;
}

ECore_TemperatureZone USurvivalComponent::GetTemperatureZone() const
{
    const float T = Stats.AmbientTemperature;
    if (T < -10.f)  return ECore_TemperatureZone::Freezing;
    if (T < 5.f)    return ECore_TemperatureZone::Cold;
    if (T < 15.f)   return ECore_TemperatureZone::Cool;
    if (T < 28.f)   return ECore_TemperatureZone::Temperate;
    if (T < 38.f)   return ECore_TemperatureZone::Warm;
    if (T < 50.f)   return ECore_TemperatureZone::Hot;
    return ECore_TemperatureZone::Extreme;
}

// ---------------------------------------------------------------------------
// Stat Modifiers
// ---------------------------------------------------------------------------

void USurvivalComponent::ApplyDamage(float Amount, AActor* DamageCauser)
{
    if (!IsAlive() || Amount <= 0.f) return;

    Stats.Health = FMath::Max(0.f, Stats.Health - Amount);
    BroadcastStatChange(FName("Health"), Stats.Health);

    if (Stats.Health <= 0.f)
    {
        const ECore_SurvivalStatus OldStatus = CurrentStatus;
        CurrentStatus = ECore_SurvivalStatus::Dead;
        OnSurvivalStatusChanged.Broadcast(OldStatus, CurrentStatus);
        OnCharacterDied.Broadcast(GetOwner());
    }
}

void USurvivalComponent::HealHealth(float Amount)
{
    if (!IsAlive() || Amount <= 0.f) return;

    Stats.Health = FMath::Min(Stats.MaxHealth, Stats.Health + Amount);
    BroadcastStatChange(FName("Health"), Stats.Health);
    UpdateStatus();
}

void USurvivalComponent::ConsumeFood(float NutritionValue)
{
    if (!IsAlive() || NutritionValue <= 0.f) return;

    Stats.Hunger = FMath::Min(Stats.MaxHunger, Stats.Hunger + NutritionValue);
    BroadcastStatChange(FName("Hunger"), Stats.Hunger);
    UpdateStatus();
}

void USurvivalComponent::DrinkWater(float HydrationValue)
{
    if (!IsAlive() || HydrationValue <= 0.f) return;

    Stats.Thirst = FMath::Min(Stats.MaxThirst, Stats.Thirst + HydrationValue);
    BroadcastStatChange(FName("Thirst"), Stats.Thirst);
    UpdateStatus();
}

void USurvivalComponent::DrainStamina(float Amount)
{
    if (!IsAlive() || Amount <= 0.f) return;

    Stats.Stamina = FMath::Max(0.f, Stats.Stamina - Amount);
    BroadcastStatChange(FName("Stamina"), Stats.Stamina);
}

void USurvivalComponent::RestoreStamina(float Amount)
{
    if (!IsAlive() || Amount <= 0.f) return;

    Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + Amount);
    BroadcastStatChange(FName("Stamina"), Stats.Stamina);
}

void USurvivalComponent::AddFear(float Amount)
{
    if (!IsAlive() || Amount <= 0.f) return;

    Stats.Fear = FMath::Min(100.f, Stats.Fear + Amount);
    BroadcastStatChange(FName("Fear"), Stats.Fear);
}

void USurvivalComponent::ReduceFear(float Amount)
{
    if (Amount <= 0.f) return;

    Stats.Fear = FMath::Max(0.f, Stats.Fear - Amount);
    BroadcastStatChange(FName("Fear"), Stats.Fear);
}

// ---------------------------------------------------------------------------
// Environment Integration
// ---------------------------------------------------------------------------

void USurvivalComponent::SetAmbientTemperature(float CelsiusTemperature)
{
    Stats.AmbientTemperature = CelsiusTemperature;
}

void USurvivalComponent::ApplyBiomeModifiers(float HungerMultiplier,
                                              float ThirstMultiplier,
                                              float StaminaMultiplier)
{
    BiomeHungerMult  = FMath::Max(0.1f, HungerMultiplier);
    BiomeThirstMult  = FMath::Max(0.1f, ThirstMultiplier);
    BiomeStaminaMult = FMath::Max(0.1f, StaminaMultiplier);
}

void USurvivalComponent::SetSprinting(bool bNewSprinting)
{
    bIsSprinting = bNewSprinting;
}

// ---------------------------------------------------------------------------
// Internal — TickSurvivalStats
// ---------------------------------------------------------------------------

void USurvivalComponent::TickSurvivalStats(float DeltaTime)
{
    if (!IsAlive()) return;

    // --- Hunger drain ---
    const float HungerDrain = Rates.HungerDrainPerSecond * BiomeHungerMult * DeltaTime;
    Stats.Hunger = FMath::Max(0.f, Stats.Hunger - HungerDrain);

    // --- Thirst drain ---
    const float ThirstDrain = Rates.ThirstDrainPerSecond * BiomeThirstMult * DeltaTime;
    Stats.Thirst = FMath::Max(0.f, Stats.Thirst - ThirstDrain);

    // --- Stamina ---
    if (bIsSprinting)
    {
        const float StaminaDrain = Rates.StaminaDrainPerSecond * BiomeStaminaMult * DeltaTime;
        Stats.Stamina = FMath::Max(0.f, Stats.Stamina - StaminaDrain);
    }
    else
    {
        const float StaminaRegen = Rates.StaminaRegenPerSecond * DeltaTime;
        Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + StaminaRegen);
    }

    // --- Health damage from starvation / dehydration ---
    if (Stats.Hunger <= 0.f)
    {
        const float StarveHit = Rates.HealthDrainFromStarvation * DeltaTime;
        Stats.Health = FMath::Max(0.f, Stats.Health - StarveHit);
    }

    if (Stats.Thirst <= 0.f)
    {
        const float DehydrateHit = Rates.HealthDrainFromDehydration * DeltaTime;
        Stats.Health = FMath::Max(0.f, Stats.Health - DehydrateHit);
    }

    // --- Fear decay when safe ---
    if (Stats.Fear > 0.f)
    {
        const float FearDecay = Rates.FearDecayPerSecond * DeltaTime;
        Stats.Fear = FMath::Max(0.f, Stats.Fear - FearDecay);
    }

    // Broadcast changes
    BroadcastStatChange(FName("Hunger"), Stats.Hunger);
    BroadcastStatChange(FName("Thirst"), Stats.Thirst);
    BroadcastStatChange(FName("Stamina"), Stats.Stamina);
    BroadcastStatChange(FName("Health"), Stats.Health);

    // Death check
    if (Stats.Health <= 0.f && IsAlive())
    {
        const ECore_SurvivalStatus OldStatus = CurrentStatus;
        CurrentStatus = ECore_SurvivalStatus::Dead;
        OnSurvivalStatusChanged.Broadcast(OldStatus, CurrentStatus);
        OnCharacterDied.Broadcast(GetOwner());
    }
}

// ---------------------------------------------------------------------------
// Internal — UpdateTemperature
// ---------------------------------------------------------------------------

void USurvivalComponent::UpdateTemperature(float DeltaTime)
{
    // Body temperature drifts toward ambient temperature
    // Real lapse: -6.5°C per 1000m altitude (applied by BiomeManager via SetAmbientTemperature)
    const float TempDiff = Stats.AmbientTemperature - Stats.BodyTemperature;
    Stats.BodyTemperature += TempDiff * Rates.BiomeTemperatureInfluence * DeltaTime;

    // Hypothermia: body temp < 35°C → health drain
    if (Stats.BodyTemperature < 35.f && IsAlive())
    {
        const float HypoHit = (35.f - Stats.BodyTemperature) * 0.1f * DeltaTime;
        Stats.Health = FMath::Max(0.f, Stats.Health - HypoHit);
    }

    // Hyperthermia: body temp > 40°C → health drain
    if (Stats.BodyTemperature > 40.f && IsAlive())
    {
        const float HyperHit = (Stats.BodyTemperature - 40.f) * 0.15f * DeltaTime;
        Stats.Health = FMath::Max(0.f, Stats.Health - HyperHit);
    }
}

// ---------------------------------------------------------------------------
// Internal — UpdateStatus
// ---------------------------------------------------------------------------

void USurvivalComponent::UpdateStatus()
{
    if (CurrentStatus == ECore_SurvivalStatus::Dead) return;

    const ECore_SurvivalStatus NewStatus = ComputeStatus();
    if (NewStatus != CurrentStatus)
    {
        const ECore_SurvivalStatus OldStatus = CurrentStatus;
        CurrentStatus = NewStatus;
        OnSurvivalStatusChanged.Broadcast(OldStatus, CurrentStatus);
    }
}

ECore_SurvivalStatus USurvivalComponent::ComputeStatus() const
{
    if (Stats.Health <= 0.f)                return ECore_SurvivalStatus::Dead;
    if (Stats.Health < 20.f)                return ECore_SurvivalStatus::Critical;
    if (Stats.BodyTemperature < 35.f)       return ECore_SurvivalStatus::Hypothermic;
    if (Stats.BodyTemperature > 40.f)       return ECore_SurvivalStatus::Hyperthermic;
    if (Stats.Stamina < 10.f)               return ECore_SurvivalStatus::Exhausted;
    if (Stats.Thirst < 20.f)               return ECore_SurvivalStatus::Thirsty;
    if (Stats.Hunger < 20.f)               return ECore_SurvivalStatus::Hungry;
    return ECore_SurvivalStatus::Healthy;
}

// ---------------------------------------------------------------------------
// Internal — BroadcastStatChange
// ---------------------------------------------------------------------------

void USurvivalComponent::BroadcastStatChange(FName StatName, float NewValue)
{
    OnStatChanged.Broadcast(StatName, NewValue);
}
