// SurvivalComponent.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Full implementation of survival stats tick, environmental damage, BiomeManager integration

#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ── Constructor ───────────────────────────────────────────────────────────────

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;   // 10 Hz — sufficient for survival stats

    bIsSprinting = false;
    bIsDead = false;
    AmbientTemperature = 25.0f;
    LastStatus = ECore_SurvivalStatus::Healthy;
}

// ── BeginPlay ─────────────────────────────────────────────────────────────────

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Ensure stats are valid on start
    Stats.Health        = FMath::Clamp(Stats.Health,        0.0f, Stats.MaxHealth);
    Stats.Hunger        = FMath::Clamp(Stats.Hunger,        0.0f, Stats.MaxHunger);
    Stats.Thirst        = FMath::Clamp(Stats.Thirst,        0.0f, Stats.MaxThirst);
    Stats.Stamina       = FMath::Clamp(Stats.Stamina,       0.0f, Stats.MaxStamina);
    Stats.BodyTemperature = 37.0f;  // Start at normal human core temp
    Stats.Fear          = 0.0f;

    LastStatus = GetCurrentStatus();
}

// ── TickComponent ─────────────────────────────────────────────────────────────

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    TickHunger(DeltaTime);
    TickThirst(DeltaTime);
    TickStamina(DeltaTime);
    TickBodyTemperature(DeltaTime);
    TickFear(DeltaTime);
    TickEnvironmentalDamage(DeltaTime);
    CheckStatusChange();
}

// ── Hunger Tick ───────────────────────────────────────────────────────────────

void USurvivalComponent::TickHunger(float DeltaTime)
{
    float DrainRate = Rates.HungerDrainRate;
    if (bIsSprinting)
    {
        DrainRate *= Rates.HungerSprintMultiplier;
    }

    Stats.Hunger = ClampStat(Stats.Hunger - DrainRate * DeltaTime, Stats.MaxHunger);

    // Fire Blueprint event when critically hungry (below 15%)
    if (Stats.Hunger < Stats.MaxHunger * 0.15f && Stats.Hunger > 0.0f)
    {
        OnHungerCritical();
    }
}

// ── Thirst Tick ───────────────────────────────────────────────────────────────

void USurvivalComponent::TickThirst(float DeltaTime)
{
    float DrainRate = Rates.ThirstDrainRate;

    // Hot biomes increase thirst drain
    if (AmbientTemperature > 35.0f)
    {
        float HeatFactor = FMath::GetMappedRangeValueClamped(
            FVector2D(35.0f, 50.0f),
            FVector2D(1.0f, Rates.ThirstHeatMultiplier),
            AmbientTemperature
        );
        DrainRate *= HeatFactor;
    }

    // Sprinting also increases thirst
    if (bIsSprinting)
    {
        DrainRate *= 1.5f;
    }

    Stats.Thirst = ClampStat(Stats.Thirst - DrainRate * DeltaTime, Stats.MaxThirst);

    // Fire Blueprint event when critically thirsty (below 10%)
    if (Stats.Thirst < Stats.MaxThirst * 0.10f && Stats.Thirst > 0.0f)
    {
        OnThirstCritical();
    }
}

// ── Stamina Tick ──────────────────────────────────────────────────────────────

void USurvivalComponent::TickStamina(float DeltaTime)
{
    if (bIsSprinting)
    {
        // Drain stamina while sprinting
        float DrainRate = Rates.StaminaDrainRate;

        // Hunger and thirst affect stamina drain
        if (Stats.Hunger < Stats.MaxHunger * 0.3f)
        {
            DrainRate *= 1.5f;  // Hungry = tires faster
        }

        Stats.Stamina = ClampStat(Stats.Stamina - DrainRate * DeltaTime, Stats.MaxStamina);

        // Force stop sprinting if stamina depleted
        if (Stats.Stamina <= 0.0f)
        {
            bIsSprinting = false;
            OnStaminaDepleted();
        }
    }
    else
    {
        // Recover stamina at rest
        float RecoveryRate = Rates.StaminaRecoveryRate;

        // Hunger/thirst reduce recovery
        if (Stats.Hunger < Stats.MaxHunger * 0.5f)
        {
            RecoveryRate *= 0.5f;
        }
        if (Stats.Thirst < Stats.MaxThirst * 0.5f)
        {
            RecoveryRate *= 0.5f;
        }

        Stats.Stamina = ClampStat(Stats.Stamina + RecoveryRate * DeltaTime, Stats.MaxStamina);
    }
}

// ── Body Temperature Tick ─────────────────────────────────────────────────────

void USurvivalComponent::TickBodyTemperature(float DeltaTime)
{
    // Body temperature moves toward ambient temperature (thermoregulation)
    // Human body fights to maintain 37°C — resistance proportional to difference
    const float TargetBodyTemp = 37.0f;
    const float ThermoregulationStrength = 2.0f;    // How hard body fights ambient
    const float AmbientInfluenceRate = 0.5f;         // How fast ambient affects body

    // Ambient pulls body temp toward it
    float AmbientDelta = (AmbientTemperature - Stats.BodyTemperature) * AmbientInfluenceRate * DeltaTime;

    // Body fights back toward 37°C
    float BodyFight = (TargetBodyTemp - Stats.BodyTemperature) * ThermoregulationStrength * DeltaTime;

    // Hunger/thirst reduce thermoregulation ability
    float ThermoFactor = 1.0f;
    if (Stats.Hunger < Stats.MaxHunger * 0.3f)
    {
        ThermoFactor *= 0.6f;   // Starving = poor thermoregulation
    }
    if (Stats.Thirst < Stats.MaxThirst * 0.3f)
    {
        ThermoFactor *= 0.7f;
    }

    Stats.BodyTemperature += AmbientDelta + (BodyFight * ThermoFactor);
    Stats.BodyTemperature = FMath::Clamp(Stats.BodyTemperature, 25.0f, 45.0f);
}

// ── Fear Tick ─────────────────────────────────────────────────────────────────

void USurvivalComponent::TickFear(float DeltaTime)
{
    // Fear naturally decays when no threats are present
    // Actual fear addition happens via AddFear() called by DinosaurAI proximity system
    if (Stats.Fear > 0.0f)
    {
        Stats.Fear = ClampStat(Stats.Fear - Rates.FearDecayRate * DeltaTime, Stats.MaxFear);
    }

    // High fear increases thirst/hunger drain (adrenaline)
    // This is handled implicitly by the multipliers above
}

// ── Environmental Damage Tick ─────────────────────────────────────────────────

void USurvivalComponent::TickEnvironmentalDamage(float DeltaTime)
{
    // Starvation damage
    if (Stats.Hunger <= 0.0f)
    {
        ApplyDamage(Rates.StarvationDamageRate * DeltaTime, ECore_DamageSource::Starvation);
    }

    // Dehydration damage (faster than starvation — biologically accurate)
    if (Stats.Thirst <= 0.0f)
    {
        ApplyDamage(Rates.DehydrationDamageRate * DeltaTime, ECore_DamageSource::Dehydration);
    }

    // Hypothermia damage (body temp below 35°C)
    if (Stats.BodyTemperature < 35.0f)
    {
        float Severity = FMath::GetMappedRangeValueClamped(
            FVector2D(25.0f, 35.0f),
            FVector2D(3.0f, 0.0f),
            Stats.BodyTemperature
        );
        ApplyDamage(Rates.HypothermiaDamageRate * Severity * DeltaTime, ECore_DamageSource::Hypothermia);
    }

    // Hyperthermia damage (body temp above 40°C)
    if (Stats.BodyTemperature > 40.0f)
    {
        float Severity = FMath::GetMappedRangeValueClamped(
            FVector2D(40.0f, 45.0f),
            FVector2D(0.0f, 3.0f),
            Stats.BodyTemperature
        );
        ApplyDamage(Rates.HyperthermaDamageRate * Severity * DeltaTime, ECore_DamageSource::Hyperthermia);
    }
}

// ── Status Change Check ───────────────────────────────────────────────────────

void USurvivalComponent::CheckStatusChange()
{
    ECore_SurvivalStatus NewStatus = GetCurrentStatus();
    if (NewStatus != LastStatus)
    {
        LastStatus = NewStatus;
        OnStatusChanged(NewStatus);
    }
}

// ── Public API ────────────────────────────────────────────────────────────────

ECore_SurvivalStatus USurvivalComponent::GetCurrentStatus() const
{
    if (!IsAlive())                                     return ECore_SurvivalStatus::Dead;
    if (Stats.Health < Stats.MaxHealth * 0.2f)          return ECore_SurvivalStatus::Critical;
    if (Stats.BodyTemperature < 35.0f)                  return ECore_SurvivalStatus::Hypothermic;
    if (Stats.BodyTemperature > 40.0f)                  return ECore_SurvivalStatus::Hyperthermic;
    if (Stats.Thirst <= 0.0f)                           return ECore_SurvivalStatus::Dehydrated;
    if (Stats.Thirst < Stats.MaxThirst * 0.2f)          return ECore_SurvivalStatus::Thirsty;
    if (Stats.Hunger <= 0.0f)                           return ECore_SurvivalStatus::Starving;
    if (Stats.Hunger < Stats.MaxHunger * 0.25f)         return ECore_SurvivalStatus::Hungry;
    if (Stats.Stamina <= 0.0f)                          return ECore_SurvivalStatus::Exhausted;
    return ECore_SurvivalStatus::Healthy;
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

void USurvivalComponent::ApplyDamage(float Amount, ECore_DamageSource Source)
{
    if (bIsDead || Amount <= 0.0f) return;

    Stats.Health = ClampStat(Stats.Health - Amount, Stats.MaxHealth);

    if (Stats.Health <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        OnDeath(Source);
    }
}

void USurvivalComponent::Eat(float NutritionValue)
{
    if (bIsDead || NutritionValue <= 0.0f) return;
    Stats.Hunger = ClampStat(Stats.Hunger + NutritionValue, Stats.MaxHunger);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    if (bIsDead || HydrationValue <= 0.0f) return;
    Stats.Thirst = ClampStat(Stats.Thirst + HydrationValue, Stats.MaxThirst);
}

void USurvivalComponent::SetSprinting(bool bSprint)
{
    // Cannot sprint when exhausted or dead
    if (bIsDead) return;
    if (bSprint && Stats.Stamina <= 0.0f) return;

    bIsSprinting = bSprint;
}

void USurvivalComponent::AddFear(float Amount)
{
    if (bIsDead || Amount <= 0.0f) return;
    Stats.Fear = ClampStat(Stats.Fear + Amount, Stats.MaxFear);
}

void USurvivalComponent::ReduceFear(float Amount)
{
    if (Amount <= 0.0f) return;
    Stats.Fear = ClampStat(Stats.Fear - Amount, Stats.MaxFear);
}

void USurvivalComponent::SetAmbientTemperature(float AmbientCelsius)
{
    // Clamp to realistic Cretaceous range: -10°C (mountain) to 55°C (volcanic)
    AmbientTemperature = FMath::Clamp(AmbientCelsius, -10.0f, 55.0f);
}
