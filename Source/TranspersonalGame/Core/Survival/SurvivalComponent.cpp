// SurvivalComponent.cpp
// Core Systems Programmer #03 — P3 Character System
// Implements survival stats: health, hunger, thirst, stamina, temperature, fear
// Ticks every second; broadcasts delegates on stat changes and death.

#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // We use a timer instead of tick

    // Default stat values
    Health       = 100.0f;
    MaxHealth    = 100.0f;
    Hunger       = 100.0f;
    MaxHunger    = 100.0f;
    Thirst       = 100.0f;
    MaxThirst    = 100.0f;
    Stamina      = 100.0f;
    MaxStamina   = 100.0f;
    Temperature  = 37.0f; // Normal body temp °C
    Fear         = 0.0f;
    MaxFear      = 100.0f;

    // Drain rates per second
    HungerDrainRate     = 0.5f;
    ThirstDrainRate     = 0.8f;
    StaminaRegenRate    = 5.0f;
    StaminaDrainRate    = 10.0f;
    FearDecayRate       = 2.0f;

    bIsSprinting        = false;
    bIsDead             = false;
    SurvivalTickInterval = 1.0f;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start survival tick timer
    GetWorld()->GetTimerManager().SetTimer(
        SurvivalTickHandle,
        this,
        &USurvivalComponent::SurvivalTick,
        SurvivalTickInterval,
        true  // looping
    );
}

void USurvivalComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(SurvivalTickHandle);
    Super::EndPlay(EndPlayReason);
}

// ─────────────────────────────────────────────────────────────────────────────
// Core survival tick — called every SurvivalTickInterval seconds
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::SurvivalTick()
{
    if (bIsDead) return;

    const float DeltaTime = SurvivalTickInterval;

    // Drain hunger
    ModifyHunger(-HungerDrainRate * DeltaTime);

    // Drain thirst (faster than hunger)
    ModifyThirst(-ThirstDrainRate * DeltaTime);

    // Stamina: drain if sprinting, regen otherwise
    if (bIsSprinting)
    {
        ModifyStamina(-StaminaDrainRate * DeltaTime);
    }
    else
    {
        ModifyStamina(StaminaRegenRate * DeltaTime);
    }

    // Fear decay over time
    if (Fear > 0.0f)
    {
        Fear = FMath::Max(0.0f, Fear - FearDecayRate * DeltaTime);
        OnFearChanged.Broadcast(Fear);
    }

    // Starvation / dehydration damage
    if (Hunger <= 0.0f)
    {
        ModifyHealth(-2.0f * DeltaTime); // Starvation damage
    }
    if (Thirst <= 0.0f)
    {
        ModifyHealth(-4.0f * DeltaTime); // Dehydration damage (faster)
    }

    // Temperature extremes cause damage
    if (Temperature < 20.0f)
    {
        ModifyHealth(-(20.0f - Temperature) * 0.5f * DeltaTime); // Hypothermia
    }
    else if (Temperature > 42.0f)
    {
        ModifyHealth(-(Temperature - 42.0f) * 1.0f * DeltaTime); // Hyperthermia
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Stat modifiers
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::ModifyHealth(float Delta)
{
    if (bIsDead) return;

    Health = FMath::Clamp(Health + Delta, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(Health);

    if (Health <= 0.0f)
    {
        bIsDead = true;
        OnDeath.Broadcast();
    }
}

void USurvivalComponent::ModifyHunger(float Delta)
{
    Hunger = FMath::Clamp(Hunger + Delta, 0.0f, MaxHunger);
    OnHungerChanged.Broadcast(Hunger);
}

void USurvivalComponent::ModifyThirst(float Delta)
{
    Thirst = FMath::Clamp(Thirst + Delta, 0.0f, MaxThirst);
    OnThirstChanged.Broadcast(Thirst);
}

void USurvivalComponent::ModifyStamina(float Delta)
{
    Stamina = FMath::Clamp(Stamina + Delta, 0.0f, MaxStamina);
    OnStaminaChanged.Broadcast(Stamina);
}

void USurvivalComponent::ModifyFear(float Delta)
{
    Fear = FMath::Clamp(Fear + Delta, 0.0f, MaxFear);
    OnFearChanged.Broadcast(Fear);
}

void USurvivalComponent::SetTemperature(float NewTemp)
{
    Temperature = NewTemp;
    OnTemperatureChanged.Broadcast(Temperature);
}

// ─────────────────────────────────────────────────────────────────────────────
// Gameplay actions
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::Eat(float NutritionValue)
{
    ModifyHunger(NutritionValue);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    ModifyThirst(HydrationValue);
}

void USurvivalComponent::SetSprinting(bool bSprinting)
{
    bIsSprinting = bSprinting;
}

bool USurvivalComponent::CanSprint() const
{
    return Stamina > 10.0f && !bIsDead;
}

bool USurvivalComponent::IsAlive() const
{
    return !bIsDead && Health > 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Dinosaur encounter — spike fear, optionally deal damage
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::OnDinosaurEncounter(float FearAmount, float DamageAmount)
{
    ModifyFear(FearAmount);
    if (DamageAmount > 0.0f)
    {
        ModifyHealth(-DamageAmount);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Stat accessors (Blueprint-callable)
// ─────────────────────────────────────────────────────────────────────────────
float USurvivalComponent::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
}

float USurvivalComponent::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (Hunger / MaxHunger) : 0.0f;
}

float USurvivalComponent::GetThirstPercent() const
{
    return (MaxThirst > 0.0f) ? (Thirst / MaxThirst) : 0.0f;
}

float USurvivalComponent::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f;
}

float USurvivalComponent::GetFearPercent() const
{
    return (MaxFear > 0.0f) ? (Fear / MaxFear) : 0.0f;
}
