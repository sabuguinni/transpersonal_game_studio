// SurvivalComponent.cpp
// Core Systems Programmer #03 — Survival stat system for prehistoric survival game
// Handles: health, hunger, thirst, stamina, fear, body temperature decay/recovery

#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance

    // Initialize default stats
    Stats.Health = 100.0f;
    Stats.MaxHealth = 100.0f;
    Stats.Hunger = 100.0f;
    Stats.MaxHunger = 100.0f;
    Stats.Thirst = 100.0f;
    Stats.MaxThirst = 100.0f;
    Stats.Stamina = 100.0f;
    Stats.MaxStamina = 100.0f;
    Stats.BodyTemperature = 37.0f;
    Stats.Fear = 0.0f;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive())
    {
        return;
    }

    TickHunger(DeltaTime);
    TickThirst(DeltaTime);
    TickStamina(DeltaTime);
    TickFear(DeltaTime);
    TickStarvationDamage(DeltaTime);
}

// ============================================================
// Stat Modifiers
// ============================================================

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (Amount <= 0.0f) return;
    Stats.Health = FMath::Max(0.0f, Stats.Health - Amount);
}

void USurvivalComponent::Eat(float NutritionValue)
{
    if (NutritionValue <= 0.0f) return;
    Stats.Hunger = FMath::Min(Stats.MaxHunger, Stats.Hunger + NutritionValue);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    if (HydrationValue <= 0.0f) return;
    Stats.Thirst = FMath::Min(Stats.MaxThirst, Stats.Thirst + HydrationValue);
}

void USurvivalComponent::Rest(float StaminaRestored)
{
    if (StaminaRestored <= 0.0f) return;
    Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + StaminaRestored);
}

void USurvivalComponent::AddFear(float FearAmount)
{
    if (FearAmount <= 0.0f) return;
    Stats.Fear = FMath::Min(100.0f, Stats.Fear + FearAmount);
}

void USurvivalComponent::ReduceFear(float FearAmount)
{
    if (FearAmount <= 0.0f) return;
    Stats.Fear = FMath::Max(0.0f, Stats.Fear - FearAmount);
}

void USurvivalComponent::SetSprinting(bool bSprint)
{
    // Can only sprint if stamina is above 10%
    if (bSprint && Stats.Stamina < (Stats.MaxStamina * 0.1f))
    {
        bIsSprinting = false;
        return;
    }
    bIsSprinting = bSprint;
}

// ============================================================
// Status Query
// ============================================================

ECore_SurvivalStatus USurvivalComponent::GetSurvivalStatus() const
{
    if (Stats.Health <= 0.0f)
        return ECore_SurvivalStatus::Dead;

    if (Stats.Health < 20.0f || Stats.Hunger < 10.0f || Stats.Thirst < 10.0f)
        return ECore_SurvivalStatus::Critical;

    if (Stats.Stamina < 15.0f)
        return ECore_SurvivalStatus::Exhausted;

    if (Stats.Hunger < 30.0f)
        return ECore_SurvivalStatus::Hungry;

    if (Stats.Thirst < 30.0f)
        return ECore_SurvivalStatus::Thirsty;

    return ECore_SurvivalStatus::Healthy;
}

// ============================================================
// Private Tick Helpers
// ============================================================

void USurvivalComponent::TickHunger(float DeltaTime)
{
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - (HungerDecayRate * DeltaTime));
}

void USurvivalComponent::TickThirst(float DeltaTime)
{
    Stats.Thirst = FMath::Max(0.0f, Stats.Thirst - (ThirstDecayRate * DeltaTime));
}

void USurvivalComponent::TickStamina(float DeltaTime)
{
    if (bIsSprinting)
    {
        Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - (StaminaDrainRate * DeltaTime));
        // Auto-stop sprint when stamina depleted
        if (Stats.Stamina <= 0.0f)
        {
            bIsSprinting = false;
        }
    }
    else
    {
        // Recover stamina when not sprinting
        Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + (StaminaRecoveryRate * DeltaTime));
    }
}

void USurvivalComponent::TickFear(float DeltaTime)
{
    // Fear naturally decays over time when not threatened
    if (Stats.Fear > 0.0f)
    {
        Stats.Fear = FMath::Max(0.0f, Stats.Fear - (FearDecayRate * DeltaTime));
    }
}

void USurvivalComponent::TickStarvationDamage(float DeltaTime)
{
    // Take health damage when starving
    if (Stats.Hunger <= 0.0f)
    {
        ApplyDamage(StarvationDamageRate * DeltaTime);
    }

    // Take health damage when dehydrated (faster than starvation)
    if (Stats.Thirst <= 0.0f)
    {
        ApplyDamage(DehydrationDamageRate * DeltaTime);
    }
}
