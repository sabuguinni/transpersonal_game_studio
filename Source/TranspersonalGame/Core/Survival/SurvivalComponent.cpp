// SurvivalComponent.cpp — Transpersonal Game Studio
// Core Systems Programmer — Agent #3
// Prehistoric survival stats: health, hunger, thirst, stamina, fear

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for performance

    // Defaults
    Health    = 100.0f;
    MaxHealth = 100.0f;
    Hunger    = 100.0f;
    MaxHunger = 100.0f;
    Thirst    = 100.0f;
    MaxThirst = 100.0f;
    Stamina   = 100.0f;
    MaxStamina= 100.0f;
    Fear      = 0.0f;
    MaxFear   = 100.0f;
    Temperature = 37.0f; // Celsius — normal body temp

    // Drain rates per second
    HungerDrainRate  = 0.05f;
    ThirstDrainRate  = 0.08f;
    StaminaRegenRate = 5.0f;
    FearDecayRate    = 2.0f;

    bIsAlive = true;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAlive) return;

    // Drain hunger and thirst over time
    Hunger = FMath::Max(0.0f, Hunger - HungerDrainRate * DeltaTime);
    Thirst = FMath::Max(0.0f, Thirst - ThirstDrainRate * DeltaTime);

    // Hunger/thirst damage
    if (Hunger <= 0.0f)
    {
        ApplyDamage(0.02f * DeltaTime, ECore_DamageType::Starvation);
    }
    if (Thirst <= 0.0f)
    {
        ApplyDamage(0.04f * DeltaTime, ECore_DamageType::Dehydration);
    }

    // Fear decays naturally
    Fear = FMath::Max(0.0f, Fear - FearDecayRate * DeltaTime);

    // Stamina regenerates when not sprinting
    if (Stamina < MaxStamina)
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenRate * DeltaTime);
    }

    // Death check
    if (Health <= 0.0f && bIsAlive)
    {
        bIsAlive = false;
        OnDeath.Broadcast();
    }
}

void USurvivalComponent::ApplyDamage(float Amount, ECore_DamageType DamageType)
{
    if (!bIsAlive || Amount <= 0.0f) return;
    Health = FMath::Max(0.0f, Health - Amount);
    OnHealthChanged.Broadcast(Health, MaxHealth);
}

void USurvivalComponent::Heal(float Amount)
{
    if (!bIsAlive || Amount <= 0.0f) return;
    Health = FMath::Min(MaxHealth, Health + Amount);
    OnHealthChanged.Broadcast(Health, MaxHealth);
}

void USurvivalComponent::Eat(float NutritionValue)
{
    Hunger = FMath::Min(MaxHunger, Hunger + NutritionValue);
    OnHungerChanged.Broadcast(Hunger, MaxHunger);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    Thirst = FMath::Min(MaxThirst, Thirst + HydrationValue);
    OnThirstChanged.Broadcast(Thirst, MaxThirst);
}

void USurvivalComponent::AddFear(float FearAmount)
{
    Fear = FMath::Min(MaxFear, Fear + FearAmount);
    OnFearChanged.Broadcast(Fear, MaxFear);
}

void USurvivalComponent::DrainStamina(float Amount)
{
    Stamina = FMath::Max(0.0f, Stamina - Amount);
    OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

bool USurvivalComponent::HasEnoughStamina(float Required) const
{
    return Stamina >= Required;
}

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
