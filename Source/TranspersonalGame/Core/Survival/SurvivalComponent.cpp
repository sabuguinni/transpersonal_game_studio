// SurvivalComponent.cpp
// Transpersonal Game Studio — Core Systems Programmer (#03)
// Full implementation of survival stats: drain, regen, status evaluation.

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // We handle our own interval via accumulator

    Stats = FCore_SurvivalStats();
    CurrentStatus = ECore_SurvivalStatus::Healthy;
    AmbientTemperature = 25.0f;
    bIsSprinting = false;
    AccumulatedTime = 0.0f;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    UpdateStatus();
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AccumulatedTime += DeltaTime;
    if (AccumulatedTime >= TickInterval)
    {
        SurvivalTick(AccumulatedTime);
        AccumulatedTime = 0.0f;
    }
}

void USurvivalComponent::SurvivalTick(float DeltaSeconds)
{
    if (!IsAlive()) return;

    // Drain hunger
    Stats.Hunger -= HungerDrainPerSecond * DeltaSeconds;

    // Drain thirst
    Stats.Thirst -= ThirstDrainPerSecond * DeltaSeconds;

    // Stamina: drain while sprinting, regen otherwise
    if (bIsSprinting)
    {
        Stats.Stamina -= StaminaDrainWhileSprinting * DeltaSeconds;
        if (Stats.Stamina <= 0.0f)
        {
            Stats.Stamina = 0.0f;
            bIsSprinting = false; // Force stop sprint
        }
    }
    else
    {
        Stats.Stamina += StaminaRegenPerSecond * DeltaSeconds;
    }

    // Health drain from starvation
    if (Stats.Hunger <= 0.0f)
    {
        Stats.Health -= HealthDrainWhenStarving * DeltaSeconds;
    }

    // Health drain from dehydration
    if (Stats.Thirst <= 0.0f)
    {
        Stats.Health -= HealthDrainWhenDehydrated * DeltaSeconds;
    }

    // Temperature regulation: body tries to return to 37°C
    float TempDiff = AmbientTemperature - Stats.Temperature;
    Stats.Temperature += TempDiff * 0.01f * DeltaSeconds;

    // Hypothermia / hyperthermia damage
    if (Stats.Temperature < 32.0f)
    {
        Stats.Health -= (32.0f - Stats.Temperature) * 0.5f * DeltaSeconds;
    }
    else if (Stats.Temperature > 40.0f)
    {
        Stats.Health -= (Stats.Temperature - 40.0f) * 0.5f * DeltaSeconds;
    }

    // Fear decay
    Stats.Fear -= FearDecayPerSecond * DeltaSeconds;

    ClampStats();
    UpdateStatus();
}

void USurvivalComponent::UpdateStatus()
{
    if (Stats.Health <= 0.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Dead;
        return;
    }
    if (Stats.Health < 20.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Dying;
        return;
    }
    if (Stats.Temperature < 32.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Hypothermic;
        return;
    }
    if (Stats.Temperature > 40.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Hyperthermic;
        return;
    }
    if (Stats.Stamina < 10.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Exhausted;
        return;
    }
    if (Stats.Thirst < 20.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Thirsty;
        return;
    }
    if (Stats.Hunger < 20.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Hungry;
        return;
    }
    CurrentStatus = ECore_SurvivalStatus::Healthy;
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

// --- Public Modifiers ---

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (!IsAlive()) return;
    Stats.Health -= FMath::Abs(Amount);
    ClampStats();
    UpdateStatus();
}

void USurvivalComponent::Heal(float Amount)
{
    Stats.Health += FMath::Abs(Amount);
    ClampStats();
    UpdateStatus();
}

void USurvivalComponent::Eat(float NutritionValue)
{
    Stats.Hunger += FMath::Abs(NutritionValue);
    ClampStats();
    UpdateStatus();
}

void USurvivalComponent::Drink(float HydrationValue)
{
    Stats.Thirst += FMath::Abs(HydrationValue);
    ClampStats();
    UpdateStatus();
}

void USurvivalComponent::Rest(float StaminaRestored)
{
    Stats.Stamina += FMath::Abs(StaminaRestored);
    ClampStats();
    UpdateStatus();
}

void USurvivalComponent::AddFear(float FearAmount)
{
    Stats.Fear += FMath::Abs(FearAmount);
    ClampStats();
}

void USurvivalComponent::ReduceFear(float FearAmount)
{
    Stats.Fear -= FMath::Abs(FearAmount);
    ClampStats();
}

void USurvivalComponent::SetAmbientTemperature(float AmbientCelsius)
{
    AmbientTemperature = FMath::Clamp(AmbientCelsius, -50.0f, 80.0f);
}

void USurvivalComponent::SetSprinting(bool bSprint)
{
    if (bSprint && Stats.Stamina <= 0.0f)
    {
        bIsSprinting = false;
        return;
    }
    bIsSprinting = bSprint;
}
