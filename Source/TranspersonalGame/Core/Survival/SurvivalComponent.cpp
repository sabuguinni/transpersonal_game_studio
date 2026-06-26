// SurvivalComponent.cpp
// Core Systems Programmer — Agent #03
// Full implementation of survival stats: health, hunger, thirst, stamina, fear

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Engine/World.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for performance

    // Default survival stats
    Health = 100.0f;
    MaxHealth = 100.0f;
    Hunger = 100.0f;
    MaxHunger = 100.0f;
    Thirst = 100.0f;
    MaxThirst = 100.0f;
    Stamina = 100.0f;
    MaxStamina = 100.0f;
    Fear = 0.0f;
    MaxFear = 100.0f;
    Temperature = 37.0f; // Normal body temperature in Celsius

    // Decay rates per second
    HungerDecayRate = 0.5f;   // Lose 0.5 hunger/sec = ~3.3 min to empty
    ThirstDecayRate = 0.8f;   // Lose 0.8 thirst/sec = ~2 min to empty
    StaminaRegenRate = 5.0f;  // Regain 5 stamina/sec when resting
    StaminaDrainRate = 10.0f; // Drain 10 stamina/sec when sprinting
    FearDecayRate = 2.0f;     // Fear decays 2/sec when safe

    bIsSprinting = false;
    bIsExhausted = false;
    bIsDead = false;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    // Stats start at full
    Health = MaxHealth;
    Hunger = MaxHunger;
    Thirst = MaxThirst;
    Stamina = MaxStamina;
    Fear = 0.0f;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    // Hunger decay
    ModifyStat(Hunger, -HungerDecayRate * DeltaTime, MaxHunger);

    // Thirst decay
    ModifyStat(Thirst, -ThirstDecayRate * DeltaTime, MaxThirst);

    // Stamina: drain when sprinting, regen when resting
    if (bIsSprinting && !bIsExhausted)
    {
        ModifyStat(Stamina, -StaminaDrainRate * DeltaTime, MaxStamina);
        if (Stamina <= 0.0f)
        {
            bIsExhausted = true;
            bIsSprinting = false;
            OnExhausted.Broadcast();
        }
    }
    else if (!bIsSprinting && Stamina < MaxStamina)
    {
        ModifyStat(Stamina, StaminaRegenRate * DeltaTime, MaxStamina);
        if (bIsExhausted && Stamina >= 25.0f)
        {
            bIsExhausted = false; // Recover from exhaustion at 25% stamina
        }
    }

    // Fear decay when not in danger
    if (Fear > 0.0f)
    {
        ModifyStat(Fear, -FearDecayRate * DeltaTime, MaxFear);
    }

    // Starvation damage: if hunger or thirst at 0, drain health
    if (Hunger <= 0.0f)
    {
        ApplyDamage(2.0f * DeltaTime); // 2 HP/sec starvation
    }
    if (Thirst <= 0.0f)
    {
        ApplyDamage(3.0f * DeltaTime); // 3 HP/sec dehydration (faster)
    }
}

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (bIsDead || Amount <= 0.0f) return;

    Health = FMath::Max(0.0f, Health - Amount);
    OnHealthChanged.Broadcast(Health, MaxHealth);

    if (Health <= 0.0f)
    {
        bIsDead = true;
        OnDeath.Broadcast();
    }
}

void USurvivalComponent::Heal(float Amount)
{
    if (bIsDead || Amount <= 0.0f) return;
    ModifyStat(Health, Amount, MaxHealth);
    OnHealthChanged.Broadcast(Health, MaxHealth);
}

void USurvivalComponent::Eat(float NutritionValue)
{
    if (bIsDead) return;
    ModifyStat(Hunger, NutritionValue, MaxHunger);
    OnHungerChanged.Broadcast(Hunger, MaxHunger);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    if (bIsDead) return;
    ModifyStat(Thirst, HydrationValue, MaxThirst);
    OnThirstChanged.Broadcast(Thirst, MaxThirst);
}

void USurvivalComponent::AddFear(float FearAmount)
{
    if (bIsDead) return;
    ModifyStat(Fear, FearAmount, MaxFear);
    OnFearChanged.Broadcast(Fear, MaxFear);
}

void USurvivalComponent::SetSprinting(bool bSprinting)
{
    if (bIsExhausted && bSprinting) return; // Can't sprint while exhausted
    bIsSprinting = bSprinting;
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

bool USurvivalComponent::IsAlive() const
{
    return !bIsDead && Health > 0.0f;
}

bool USurvivalComponent::IsCritical() const
{
    return Health < (MaxHealth * 0.25f) || Hunger < (MaxHunger * 0.1f) || Thirst < (MaxThirst * 0.1f);
}

void USurvivalComponent::ModifyStat(float& Stat, float Delta, float Max)
{
    Stat = FMath::Clamp(Stat + Delta, 0.0f, Max);
}
