// SurvivalComponent.cpp
// Core Systems — Survival Statistics for Prehistoric Human Character
// Agent #03 — Core Systems Programmer

#include "Core/Survival/SurvivalComponent.h"
#include "Math/UnrealMathUtility.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // tick every frame for smooth stamina recovery
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialise stats to full — world/save system can override after BeginPlay
    Stats.Health    = 100.f;
    Stats.Hunger    = 100.f;
    Stats.Thirst    = 100.f;
    Stats.Stamina   = 100.f;
    Stats.Fear      = 0.f;
    Stats.BodyTemperature = 37.0f;
    Stats.State     = ECore_SurvivalState::Healthy;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive())
    {
        return;
    }

    // Drain hunger/thirst on a 1-second accumulator to avoid float precision drift
    AccumulatedTime += DeltaTime;
    if (AccumulatedTime >= TickInterval)
    {
        DrainStats(AccumulatedTime);
        AccumulatedTime = 0.f;
    }

    // Stamina recovery is frame-rate dependent for smoothness
    if (Stats.Stamina < 100.f)
    {
        SetStat(Stats.Stamina, Stats.Stamina + StaminaRecoveryRate * DeltaTime);
    }

    // Fear decays passively
    if (Stats.Fear > 0.f)
    {
        SetStat(Stats.Fear, Stats.Fear - FearDecayRate * DeltaTime);
    }

    // Body temperature drifts toward ambient
    UpdateBodyTemperature(DeltaTime);

    // Apply starvation/dehydration damage every frame (scaled)
    ApplyStarvationDamage(DeltaTime);

    // Recalculate overall state
    RecalculateState();
}

// ── Stat modifiers ────────────────────────────────────────────────────────────

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (!IsAlive())
    {
        return;
    }

    const float OldHealth = Stats.Health;
    SetStat(Stats.Health, Stats.Health - Amount);
    OnHealthChanged.Broadcast(Stats.Health);

    if (Stats.Health <= 0.f && OldHealth > 0.f)
    {
        Stats.State = ECore_SurvivalState::Dead;
        OnStateChanged.Broadcast(ECore_SurvivalState::Dead);
        OnDeath.Broadcast();
    }
}

void USurvivalComponent::ConsumeFood(float Amount)
{
    SetStat(Stats.Hunger, Stats.Hunger + Amount);
}

void USurvivalComponent::DrinkWater(float Amount)
{
    SetStat(Stats.Thirst, Stats.Thirst + Amount);
}

void USurvivalComponent::ModifyStamina(float Delta)
{
    SetStat(Stats.Stamina, Stats.Stamina + Delta);
}

void USurvivalComponent::AddFear(float Amount)
{
    SetStat(Stats.Fear, Stats.Fear + Amount);
}

void USurvivalComponent::ReduceFear(float Amount)
{
    SetStat(Stats.Fear, Stats.Fear - Amount);
}

void USurvivalComponent::SetAmbientTemperature(float Celsius)
{
    AmbientTemperature = Celsius;
}

// ── Private helpers ───────────────────────────────────────────────────────────

void USurvivalComponent::DrainStats(float DeltaSeconds)
{
    SetStat(Stats.Hunger, Stats.Hunger - HungerDrainRate * DeltaSeconds);
    SetStat(Stats.Thirst, Stats.Thirst - ThirstDrainRate * DeltaSeconds);
}

void USurvivalComponent::UpdateBodyTemperature(float DeltaSeconds)
{
    // Body temperature drifts toward ambient at 0.1°C per second
    const float Drift = 0.1f * DeltaSeconds;
    if (Stats.BodyTemperature < AmbientTemperature)
    {
        Stats.BodyTemperature = FMath::Min(Stats.BodyTemperature + Drift, AmbientTemperature);
    }
    else if (Stats.BodyTemperature > AmbientTemperature)
    {
        Stats.BodyTemperature = FMath::Max(Stats.BodyTemperature - Drift, AmbientTemperature);
    }
}

void USurvivalComponent::ApplyStarvationDamage(float DeltaSeconds)
{
    const bool bStarving    = Stats.Hunger <= 0.f;
    const bool bDehydrated  = Stats.Thirst <= 0.f;

    if (bStarving || bDehydrated)
    {
        // Dehydration is more lethal than starvation — double rate if both
        float DamageMultiplier = 1.0f;
        if (bStarving && bDehydrated)
        {
            DamageMultiplier = 2.5f;
        }
        else if (bDehydrated)
        {
            DamageMultiplier = 1.8f;
        }

        ApplyDamage(StarvationDamageRate * DamageMultiplier * DeltaSeconds);
    }
}

void USurvivalComponent::RecalculateState()
{
    if (Stats.Health <= 0.f)
    {
        if (Stats.State != ECore_SurvivalState::Dead)
        {
            Stats.State = ECore_SurvivalState::Dead;
            OnStateChanged.Broadcast(ECore_SurvivalState::Dead);
            OnDeath.Broadcast();
        }
        return;
    }

    ECore_SurvivalState NewState = ECore_SurvivalState::Healthy;

    if (Stats.Health <= CriticalHealthThreshold)
    {
        NewState = ECore_SurvivalState::Critical;
    }
    else if (Stats.Stamina <= 10.f)
    {
        NewState = ECore_SurvivalState::Exhausted;
    }
    else if (Stats.Thirst <= 20.f)
    {
        NewState = ECore_SurvivalState::Thirsty;
    }
    else if (Stats.Hunger <= 20.f)
    {
        NewState = ECore_SurvivalState::Hungry;
    }

    if (NewState != Stats.State)
    {
        Stats.State = NewState;
        OnStateChanged.Broadcast(NewState);
    }
}

void USurvivalComponent::SetStat(float& Stat, float NewValue)
{
    Stat = FMath::Clamp(NewValue, 0.f, 100.f);
}
