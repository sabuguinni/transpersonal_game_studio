// SurvivalComponent.cpp
// Core Systems Programmer #03 — P3 Character System
// Full implementation of survival stat drain, damage, food/water, temperature effects.
#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.25f; // Tick 4x/sec for smooth drain
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    bIsDead = false;
    AccumulatedTime = 0.f;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    TickSurvivalDrain(DeltaTime);
}

// ── Drain logic ────────────────────────────────────────────────────────────────

void USurvivalComponent::TickSurvivalDrain(float DeltaSeconds)
{
    // Hunger drain
    if (Stats.Hunger > 0.f)
    {
        Stats.Hunger = FMath::Max(0.f, Stats.Hunger - HungerDrainPerSecond * DeltaSeconds);
        BroadcastStat(ECore_SurvivalStat::Hunger, Stats.Hunger);
    }

    // Thirst drain
    if (Stats.Thirst > 0.f)
    {
        Stats.Thirst = FMath::Max(0.f, Stats.Thirst - ThirstDrainPerSecond * DeltaSeconds);
        BroadcastStat(ECore_SurvivalStat::Thirst, Stats.Thirst);
    }

    // Health drain when starving
    if (Stats.Hunger <= 0.f)
    {
        Stats.Health = FMath::Max(0.f, Stats.Health - HealthDrainWhenStarving * DeltaSeconds);
        BroadcastStat(ECore_SurvivalStat::Health, Stats.Health);
    }

    // Health drain when dehydrated
    if (Stats.Thirst <= 0.f)
    {
        Stats.Health = FMath::Max(0.f, Stats.Health - HealthDrainWhenDehydrated * DeltaSeconds);
        BroadcastStat(ECore_SurvivalStat::Health, Stats.Health);
    }

    // Temperature damage — hypothermia or hyperthermia
    if (Stats.Temperature < MinTemperatureSafe)
    {
        float severity = (MinTemperatureSafe - Stats.Temperature) / MinTemperatureSafe;
        Stats.Health = FMath::Max(0.f, Stats.Health - severity * 0.05f * DeltaSeconds);
        BroadcastStat(ECore_SurvivalStat::Health, Stats.Health);
    }
    else if (Stats.Temperature > MaxTemperatureSafe)
    {
        float severity = (Stats.Temperature - MaxTemperatureSafe) / MaxTemperatureSafe;
        Stats.Health = FMath::Max(0.f, Stats.Health - severity * 0.08f * DeltaSeconds);
        BroadcastStat(ECore_SurvivalStat::Health, Stats.Health);
    }

    // Fear natural decay
    if (Stats.Fear > 0.f)
    {
        Stats.Fear = FMath::Max(0.f, Stats.Fear - FearDecayPerSecond * DeltaSeconds);
        BroadcastStat(ECore_SurvivalStat::Fear, Stats.Fear);
    }

    // Body temperature slowly moves toward ambient
    float TempDelta = AmbientTemperatureCelsius - Stats.Temperature;
    Stats.Temperature += TempDelta * 0.01f * DeltaSeconds; // Slow drift
    BroadcastStat(ECore_SurvivalStat::Temperature, Stats.Temperature);

    CheckDeath();
}

// ── Query ──────────────────────────────────────────────────────────────────────

float USurvivalComponent::GetStat(ECore_SurvivalStat Stat) const
{
    switch (Stat)
    {
        case ECore_SurvivalStat::Health:      return Stats.Health;
        case ECore_SurvivalStat::Hunger:      return Stats.Hunger;
        case ECore_SurvivalStat::Thirst:      return Stats.Thirst;
        case ECore_SurvivalStat::Stamina:     return Stats.Stamina;
        case ECore_SurvivalStat::Fear:        return Stats.Fear;
        case ECore_SurvivalStat::Temperature: return Stats.Temperature;
        default: return 0.f;
    }
}

float USurvivalComponent::GetStatNormalized(ECore_SurvivalStat Stat) const
{
    // Temperature normalized differently (37 = 1.0, range 30-45)
    if (Stat == ECore_SurvivalStat::Temperature)
    {
        return FMath::Clamp((Stats.Temperature - 30.f) / 15.f, 0.f, 1.f);
    }
    return FMath::Clamp(GetStat(Stat) / 100.f, 0.f, 1.f);
}

bool USurvivalComponent::IsAlive() const
{
    return !bIsDead && Stats.Health > 0.f;
}

bool USurvivalComponent::IsCritical(ECore_SurvivalStat Stat) const
{
    if (Stat == ECore_SurvivalStat::Temperature)
    {
        return Stats.Temperature < MinTemperatureSafe || Stats.Temperature > MaxTemperatureSafe;
    }
    return GetStat(Stat) <= CriticalThreshold;
}

// ── Modification ───────────────────────────────────────────────────────────────

void USurvivalComponent::ModifyStat(ECore_SurvivalStat Stat, float Delta)
{
    if (bIsDead) return;

    switch (Stat)
    {
        case ECore_SurvivalStat::Health:
            Stats.Health = FMath::Clamp(Stats.Health + Delta, 0.f, 100.f);
            BroadcastStat(Stat, Stats.Health);
            break;
        case ECore_SurvivalStat::Hunger:
            Stats.Hunger = FMath::Clamp(Stats.Hunger + Delta, 0.f, 100.f);
            BroadcastStat(Stat, Stats.Hunger);
            break;
        case ECore_SurvivalStat::Thirst:
            Stats.Thirst = FMath::Clamp(Stats.Thirst + Delta, 0.f, 100.f);
            BroadcastStat(Stat, Stats.Thirst);
            break;
        case ECore_SurvivalStat::Stamina:
            Stats.Stamina = FMath::Clamp(Stats.Stamina + Delta, 0.f, 100.f);
            BroadcastStat(Stat, Stats.Stamina);
            break;
        case ECore_SurvivalStat::Fear:
            Stats.Fear = FMath::Clamp(Stats.Fear + Delta, 0.f, 100.f);
            BroadcastStat(Stat, Stats.Fear);
            break;
        case ECore_SurvivalStat::Temperature:
            Stats.Temperature = FMath::Clamp(Stats.Temperature + Delta, 20.f, 50.f);
            BroadcastStat(Stat, Stats.Temperature);
            break;
        default: break;
    }

    CheckDeath();
}

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (bIsDead || Amount <= 0.f) return;
    ModifyStat(ECore_SurvivalStat::Health, -Amount);
}

void USurvivalComponent::Eat(float NutritionValue)
{
    if (bIsDead) return;
    ModifyStat(ECore_SurvivalStat::Hunger, NutritionValue);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    if (bIsDead) return;
    ModifyStat(ECore_SurvivalStat::Thirst, HydrationValue);
}

void USurvivalComponent::Rest(float StaminaRestored)
{
    if (bIsDead) return;
    ModifyStat(ECore_SurvivalStat::Stamina, StaminaRestored);
}

void USurvivalComponent::SetFear(float FearLevel)
{
    Stats.Fear = FMath::Clamp(FearLevel, 0.f, 100.f);
    BroadcastStat(ECore_SurvivalStat::Fear, Stats.Fear);
}

void USurvivalComponent::SetAmbientTemperature(float CelsiusTemp)
{
    AmbientTemperatureCelsius = FMath::Clamp(CelsiusTemp, -20.f, 60.f);
}

// ── Internal ───────────────────────────────────────────────────────────────────

void USurvivalComponent::BroadcastStat(ECore_SurvivalStat Stat, float Value)
{
    OnStatChanged.Broadcast(Stat, Value);
}

void USurvivalComponent::CheckDeath()
{
    if (!bIsDead && Stats.Health <= 0.f)
    {
        bIsDead = true;
        OnPlayerDied.Broadcast();
    }
}
