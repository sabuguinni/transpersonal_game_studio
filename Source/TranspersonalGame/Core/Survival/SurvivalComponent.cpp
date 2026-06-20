// SurvivalComponent.cpp
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260620_001
// Full implementation of survival mechanics: hunger, thirst, stamina, temperature, fear, death

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick 10x/sec — sufficient for survival drain
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Reset state flags
    bIsDead = false;
    bHungerCriticalFired = false;
    bThirstCriticalFired = false;
    bHealthCriticalFired = false;

    // Initialise stats to full
    Stats.Health = Stats.MaxHealth;
    Stats.Hunger = Stats.MaxHunger;
    Stats.Thirst = Stats.MaxThirst;
    Stats.Stamina = Stats.MaxStamina;
    Stats.BodyTemperature = 37.0f;
    Stats.Fear = 0.0f;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead)
    {
        return;
    }

    TickHunger(DeltaTime);
    TickThirst(DeltaTime);
    TickStamina(DeltaTime);
    TickTemperature(DeltaTime);
    TickFear(DeltaTime);
    CheckCriticalStates();
}

// --- Tick helpers ---

void USurvivalComponent::TickHunger(float DeltaTime)
{
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - HungerDrainRate * DeltaTime);

    if (IsStarving())
    {
        // Starvation deals health damage
        float Damage = StarvationDamageRate * DeltaTime;
        Stats.Health = FMath::Max(0.0f, Stats.Health - Damage);
        BroadcastStatChange(ECore_SurvivalStat::Health, Stats.Health);

        if (Stats.Health <= 0.0f)
        {
            Die();
            return;
        }
    }

    BroadcastStatChange(ECore_SurvivalStat::Hunger, Stats.Hunger);
}

void USurvivalComponent::TickThirst(float DeltaTime)
{
    Stats.Thirst = FMath::Max(0.0f, Stats.Thirst - ThirstDrainRate * DeltaTime);

    if (IsDehydrated())
    {
        float Damage = DehydrationDamageRate * DeltaTime;
        Stats.Health = FMath::Max(0.0f, Stats.Health - Damage);
        BroadcastStatChange(ECore_SurvivalStat::Health, Stats.Health);

        if (Stats.Health <= 0.0f)
        {
            Die();
            return;
        }
    }

    BroadcastStatChange(ECore_SurvivalStat::Thirst, Stats.Thirst);
}

void USurvivalComponent::TickStamina(float DeltaTime)
{
    if (bIsSprinting)
    {
        Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - StaminaDrainRate * DeltaTime);
    }
    else
    {
        // Regen only when not sprinting — slower if hungry/thirsty
        float RegenMultiplier = 1.0f;
        if (IsStarving()) RegenMultiplier *= 0.3f;
        if (IsDehydrated()) RegenMultiplier *= 0.3f;

        Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + StaminaRegenRate * RegenMultiplier * DeltaTime);
    }

    BroadcastStatChange(ECore_SurvivalStat::Stamina, Stats.Stamina);
}

void USurvivalComponent::TickTemperature(float DeltaTime)
{
    // Body temperature moves toward ambient temperature (simplified thermoregulation)
    float TempDelta = AmbientTemperature - Stats.BodyTemperature;
    float TempChangeRate = 0.5f; // degrees/second — slow body temp change

    Stats.BodyTemperature += TempDelta * TempChangeRate * DeltaTime;
    Stats.BodyTemperature = FMath::Clamp(Stats.BodyTemperature, 20.0f, 45.0f);

    // Hypothermia damage
    if (IsHypothermic())
    {
        float Damage = HypothermiaDamageRate * DeltaTime;
        Stats.Health = FMath::Max(0.0f, Stats.Health - Damage);
        BroadcastStatChange(ECore_SurvivalStat::Health, Stats.Health);

        if (Stats.Health <= 0.0f)
        {
            Die();
            return;
        }
    }

    // Hyperthermia damage
    if (IsHyperthermic())
    {
        float Damage = HypothermiaDamageRate * 1.5f * DeltaTime; // heatstroke is faster
        Stats.Health = FMath::Max(0.0f, Stats.Health - Damage);
        BroadcastStatChange(ECore_SurvivalStat::Health, Stats.Health);

        if (Stats.Health <= 0.0f)
        {
            Die();
            return;
        }
    }

    BroadcastStatChange(ECore_SurvivalStat::Temperature, Stats.BodyTemperature);
}

void USurvivalComponent::TickFear(float DeltaTime)
{
    // Fear decays naturally over time
    if (Stats.Fear > 0.0f)
    {
        Stats.Fear = FMath::Max(0.0f, Stats.Fear - FearDecayRate * DeltaTime);
        BroadcastStatChange(ECore_SurvivalStat::Fear, Stats.Fear);
    }
}

void USurvivalComponent::CheckCriticalStates()
{
    // Fire critical events once per crossing of threshold (not every tick)
    if (Stats.Health <= CriticalThreshold && !bHealthCriticalFired)
    {
        bHealthCriticalFired = true;
        OnCriticalStat.Broadcast(ECore_SurvivalStat::Health);
    }
    else if (Stats.Health > CriticalThreshold)
    {
        bHealthCriticalFired = false;
    }

    if (Stats.Hunger <= CriticalThreshold && !bHungerCriticalFired)
    {
        bHungerCriticalFired = true;
        OnCriticalStat.Broadcast(ECore_SurvivalStat::Hunger);
    }
    else if (Stats.Hunger > CriticalThreshold)
    {
        bHungerCriticalFired = false;
    }

    if (Stats.Thirst <= CriticalThreshold && !bThirstCriticalFired)
    {
        bThirstCriticalFired = true;
        OnCriticalStat.Broadcast(ECore_SurvivalStat::Thirst);
    }
    else if (Stats.Thirst > CriticalThreshold)
    {
        bThirstCriticalFired = false;
    }
}

void USurvivalComponent::BroadcastStatChange(ECore_SurvivalStat Stat, float NewValue)
{
    OnStatChanged.Broadcast(Stat, NewValue);
}

void USurvivalComponent::Die()
{
    if (bIsDead) return;

    bIsDead = true;
    Stats.Health = 0.0f;

    OnPlayerDied.Broadcast();
}

// --- Public API ---

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (bIsDead || Amount <= 0.0f) return;

    Stats.Health = FMath::Max(0.0f, Stats.Health - Amount);
    BroadcastStatChange(ECore_SurvivalStat::Health, Stats.Health);

    if (Stats.Health <= 0.0f)
    {
        Die();
    }
}

void USurvivalComponent::Heal(float Amount)
{
    if (bIsDead || Amount <= 0.0f) return;

    Stats.Health = FMath::Min(Stats.MaxHealth, Stats.Health + Amount);
    BroadcastStatChange(ECore_SurvivalStat::Health, Stats.Health);
}

void USurvivalComponent::Eat(float NutritionValue)
{
    if (bIsDead || NutritionValue <= 0.0f) return;

    Stats.Hunger = FMath::Min(Stats.MaxHunger, Stats.Hunger + NutritionValue);
    BroadcastStatChange(ECore_SurvivalStat::Hunger, Stats.Hunger);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    if (bIsDead || HydrationValue <= 0.0f) return;

    Stats.Thirst = FMath::Min(Stats.MaxThirst, Stats.Thirst + HydrationValue);
    BroadcastStatChange(ECore_SurvivalStat::Thirst, Stats.Thirst);
}

void USurvivalComponent::SetSprinting(bool bSprinting)
{
    bIsSprinting = bSprinting;
}

void USurvivalComponent::AddFear(float Amount)
{
    if (bIsDead || Amount <= 0.0f) return;

    Stats.Fear = FMath::Min(Stats.MaxFear, Stats.Fear + Amount);
    BroadcastStatChange(ECore_SurvivalStat::Fear, Stats.Fear);
}

void USurvivalComponent::SetAmbientTemperature(float Celsius)
{
    AmbientTemperature = FMath::Clamp(Celsius, -50.0f, 60.0f);
}

// --- Queries ---

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

bool USurvivalComponent::IsStarving() const
{
    return Stats.Hunger <= 0.0f;
}

bool USurvivalComponent::IsDehydrated() const
{
    return Stats.Thirst <= 0.0f;
}

bool USurvivalComponent::IsHypothermic() const
{
    return Stats.BodyTemperature < HypothermiaThreshold;
}

bool USurvivalComponent::IsHyperthermic() const
{
    return Stats.BodyTemperature > HyperthermiaThreshold;
}

bool USurvivalComponent::IsInCriticalState() const
{
    return Stats.Health <= CriticalThreshold
        || Stats.Hunger <= CriticalThreshold
        || Stats.Thirst <= CriticalThreshold
        || IsHypothermic()
        || IsHyperthermic();
}
