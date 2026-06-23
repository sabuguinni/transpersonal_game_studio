// SurvivalComponent.cpp
// Core Systems Programmer — Agent #3
// P3 Character System: Survival stats (health, hunger, thirst, stamina, fear)
// Integrates into TranspersonalCharacter via CreateDefaultSubobject

#include "SurvivalComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for stat drain

    // Default stat values — full health at spawn
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

    // Drain rates per second (realistic prehistoric survival pacing)
    HungerDrainRate = 0.05f;   // ~33 min to starve at rest
    ThirstDrainRate = 0.08f;   // ~21 min to dehydrate at rest
    StaminaDrainRate = 5.0f;   // Drains fast during exertion (managed externally)
    StaminaRegenRate = 3.0f;   // Regens when resting
    FearDecayRate = 0.5f;      // Fear decays slowly when safe

    bIsExerting = false;
    bIsDead = false;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Initialized on %s — Health:%.0f Hunger:%.0f Thirst:%.0f"),
        *GetOwner()->GetName(), Health, Hunger, Thirst);
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead)
    {
        return;
    }

    // Drain hunger and thirst over time
    Hunger = FMath::Max(0.0f, Hunger - HungerDrainRate * DeltaTime);
    Thirst = FMath::Max(0.0f, Thirst - ThirstDrainRate * DeltaTime);

    // Stamina: drain if exerting, regen if resting
    if (bIsExerting)
    {
        Stamina = FMath::Max(0.0f, Stamina - StaminaDrainRate * DeltaTime);
    }
    else
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenRate * DeltaTime);
    }

    // Fear decays passively when not threatened
    Fear = FMath::Max(0.0f, Fear - FearDecayRate * DeltaTime);

    // Starvation/dehydration damage
    if (Hunger <= 0.0f)
    {
        ApplyDamage(0.02f * DeltaTime); // Slow starvation damage
    }
    if (Thirst <= 0.0f)
    {
        ApplyDamage(0.04f * DeltaTime); // Dehydration kills faster than starvation
    }

    // Death check
    if (Health <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        OnDeath.Broadcast();
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent: %s has died."), *GetOwner()->GetName());
    }
}

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (bIsDead || Amount <= 0.0f)
    {
        return;
    }
    Health = FMath::Max(0.0f, Health - Amount);
    OnHealthChanged.Broadcast(Health, MaxHealth);
}

void USurvivalComponent::Heal(float Amount)
{
    if (bIsDead || Amount <= 0.0f)
    {
        return;
    }
    Health = FMath::Min(MaxHealth, Health + Amount);
    OnHealthChanged.Broadcast(Health, MaxHealth);
}

void USurvivalComponent::ConsumeFood(float NutritionValue)
{
    if (NutritionValue <= 0.0f)
    {
        return;
    }
    Hunger = FMath::Min(MaxHunger, Hunger + NutritionValue);
    OnHungerChanged.Broadcast(Hunger, MaxHunger);
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Ate food +%.1f hunger. Now: %.1f/%.1f"), NutritionValue, Hunger, MaxHunger);
}

void USurvivalComponent::ConsumeWater(float HydrationValue)
{
    if (HydrationValue <= 0.0f)
    {
        return;
    }
    Thirst = FMath::Min(MaxThirst, Thirst + HydrationValue);
    OnThirstChanged.Broadcast(Thirst, MaxThirst);
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Drank water +%.1f thirst. Now: %.1f/%.1f"), HydrationValue, Thirst, MaxThirst);
}

void USurvivalComponent::AddFear(float FearAmount)
{
    if (FearAmount <= 0.0f)
    {
        return;
    }
    Fear = FMath::Min(MaxFear, Fear + FearAmount);
    OnFearChanged.Broadcast(Fear, MaxFear);
}

void USurvivalComponent::SetExerting(bool bExerting)
{
    bIsExerting = bExerting;
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

bool USurvivalComponent::IsCriticalHealth() const
{
    return GetHealthPercent() < 0.25f;
}

bool USurvivalComponent::IsStarving() const
{
    return GetHungerPercent() < 0.15f;
}

bool USurvivalComponent::IsDehydrated() const
{
    return GetThirstPercent() < 0.15f;
}

bool USurvivalComponent::IsExhausted() const
{
    return GetStaminaPercent() < 0.1f;
}

bool USurvivalComponent::IsPanicking() const
{
    return GetFearPercent() > 0.8f;
}
