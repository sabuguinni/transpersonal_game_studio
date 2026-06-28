// SurvivalComponent.cpp
// Core Systems Programmer — Agent #03
// Prehistoric survival stats: health, hunger, thirst, stamina, temperature, fear

#include "SurvivalComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // Timer-driven, not tick-driven

    // Survival stat defaults — prehistoric human baseline
    Health = 100.f;
    MaxHealth = 100.f;
    Hunger = 100.f;
    MaxHunger = 100.f;
    Thirst = 100.f;
    MaxThirst = 100.f;
    Stamina = 100.f;
    MaxStamina = 100.f;
    BodyTemperature = 37.0f;  // Normal human temperature in Celsius
    Fear = 0.f;
    MaxFear = 100.f;

    bIsAlive = true;
    bIsStarving = false;
    bIsDehydrated = false;
    bIsHypothermic = false;
    bIsHyperthermic = false;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
}

// ============================================================
// HEALTH
// ============================================================

void USurvivalComponent::ModifyHealth(float Delta)
{
    if (!bIsAlive) return;

    Health = FMath::Clamp(Health + Delta, 0.f, MaxHealth);

    if (Health <= 0.f)
    {
        bIsAlive = false;
        OnDeath.Broadcast();
    }
    else if (Delta < 0.f)
    {
        OnHealthChanged.Broadcast(Health, MaxHealth);
        OnDamageTaken.Broadcast(-Delta);
    }
    else
    {
        OnHealthChanged.Broadcast(Health, MaxHealth);
    }
}

float USurvivalComponent::GetHealth() const
{
    return Health;
}

float USurvivalComponent::GetMaxHealth() const
{
    return MaxHealth;
}

// ============================================================
// HUNGER
// ============================================================

void USurvivalComponent::ModifyHunger(float Delta)
{
    Hunger = FMath::Clamp(Hunger + Delta, 0.f, MaxHunger);
    bIsStarving = (Hunger <= 10.f);
    OnHungerChanged.Broadcast(Hunger, MaxHunger);
}

float USurvivalComponent::GetHunger() const
{
    return Hunger;
}

float USurvivalComponent::GetMaxHunger() const
{
    return MaxHunger;
}

// ============================================================
// THIRST
// ============================================================

void USurvivalComponent::ModifyThirst(float Delta)
{
    Thirst = FMath::Clamp(Thirst + Delta, 0.f, MaxThirst);
    bIsDehydrated = (Thirst <= 10.f);
    OnThirstChanged.Broadcast(Thirst, MaxThirst);
}

float USurvivalComponent::GetThirst() const
{
    return Thirst;
}

float USurvivalComponent::GetMaxThirst() const
{
    return MaxThirst;
}

// ============================================================
// STAMINA
// ============================================================

void USurvivalComponent::ModifyStamina(float Delta)
{
    Stamina = FMath::Clamp(Stamina + Delta, 0.f, MaxStamina);
    OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

float USurvivalComponent::GetStamina() const
{
    return Stamina;
}

float USurvivalComponent::GetMaxStamina() const
{
    return MaxStamina;
}

// ============================================================
// TEMPERATURE
// ============================================================

void USurvivalComponent::SetBodyTemperature(float NewTemp)
{
    BodyTemperature = FMath::Clamp(NewTemp, 25.f, 45.f);

    bIsHypothermic = (BodyTemperature < 35.f);
    bIsHyperthermic = (BodyTemperature > 40.f);

    OnTemperatureChanged.Broadcast(BodyTemperature);
}

float USurvivalComponent::GetBodyTemperature() const
{
    return BodyTemperature;
}

// ============================================================
// FEAR
// ============================================================

void USurvivalComponent::ModifyFear(float Delta)
{
    Fear = FMath::Clamp(Fear + Delta, 0.f, MaxFear);
    OnFearChanged.Broadcast(Fear, MaxFear);

    // High fear reduces stamina recovery — prehistoric stress response
    if (Fear > 75.f)
    {
        // Adrenaline burst — temporary stamina boost when terrified
        ModifyStamina(5.f);
    }
}

float USurvivalComponent::GetFear() const
{
    return Fear;
}

// ============================================================
// CONSUME FOOD/WATER
// ============================================================

void USurvivalComponent::ConsumeFood(float NutritionValue)
{
    ModifyHunger(NutritionValue);

    // Food also provides minor health recovery if not injured
    if (Health < MaxHealth && NutritionValue > 20.f)
    {
        ModifyHealth(NutritionValue * 0.1f);
    }
}

void USurvivalComponent::ConsumeWater(float HydrationValue)
{
    ModifyThirst(HydrationValue);

    // Water also reduces fear slightly — calming effect
    if (Fear > 0.f)
    {
        ModifyFear(-HydrationValue * 0.05f);
    }
}

// ============================================================
// STATUS QUERIES
// ============================================================

bool USurvivalComponent::IsAlive() const
{
    return bIsAlive;
}

bool USurvivalComponent::IsStarving() const
{
    return bIsStarving;
}

bool USurvivalComponent::IsDehydrated() const
{
    return bIsDehydrated;
}

bool USurvivalComponent::IsHypothermic() const
{
    return bIsHypothermic;
}

bool USurvivalComponent::IsHyperthermic() const
{
    return bIsHyperthermic;
}

bool USurvivalComponent::IsCritical() const
{
    return (Health < 25.f || bIsStarving || bIsDehydrated || bIsHypothermic || bIsHyperthermic);
}

// ============================================================
// APPLY ENVIRONMENT EFFECTS
// ============================================================

void USurvivalComponent::ApplyEnvironmentTemperature(float AmbientTemp, float DeltaTime)
{
    // Body temperature slowly converges toward ambient temperature
    // Prehistoric humans had no insulation — temperature matters
    float TempDiff = AmbientTemp - BodyTemperature;
    float TempChange = TempDiff * 0.01f * DeltaTime;  // Very slow change
    SetBodyTemperature(BodyTemperature + TempChange);
}

void USurvivalComponent::ApplyRainEffect(float RainIntensity, float DeltaTime)
{
    // Rain lowers body temperature — dangerous in cold environments
    float CoolingEffect = RainIntensity * 0.5f * DeltaTime;
    SetBodyTemperature(BodyTemperature - CoolingEffect);

    // Rain also provides water — can drink from rain in extremis
    if (RainIntensity > 0.8f)
    {
        ModifyThirst(RainIntensity * 0.2f * DeltaTime);
    }
}
