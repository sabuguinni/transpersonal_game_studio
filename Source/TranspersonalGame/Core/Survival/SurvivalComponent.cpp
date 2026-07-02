// SurvivalComponent.cpp — Transpersonal Game Studio
// Core Systems Programmer — Agent #03
// Full implementation of survival stat decay, status evaluation, and modifier methods

#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms — not every frame

    // Default stats
    Stats.Health    = 100.0f;
    Stats.Hunger    = 100.0f;
    Stats.Thirst    = 100.0f;
    Stats.Stamina   = 100.0f;
    Stats.Fear      = 0.0f;
    Stats.Temperature = 37.0f;

    CurrentStatus = ECore_SurvivalStatus::Healthy;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    UpdateStatus();
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive()) return;

    ApplyDecay(DeltaTime);
    UpdateStatus();
}

void USurvivalComponent::ApplyDecay(float DeltaTime)
{
    // Hunger decay — slower, represents metabolic rate
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - HungerDecayRate * DeltaTime);

    // Thirst decay — faster, critical for survival
    Stats.Thirst = FMath::Max(0.0f, Stats.Thirst - ThirstDecayRate * DeltaTime);

    // Stamina: drain while sprinting, recover while idle
    if (bIsSprinting)
    {
        Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - StaminaDecayRate * DeltaTime);
        // Force stop sprinting if stamina depleted
        if (Stats.Stamina <= 0.0f)
        {
            bIsSprinting = false;
        }
    }
    else
    {
        Stats.Stamina = FMath::Min(100.0f, Stats.Stamina + StaminaRecoveryRate * DeltaTime);
    }

    // Fear naturally decays when not threatened
    Stats.Fear = FMath::Max(0.0f, Stats.Fear - FearDecayRate * DeltaTime);

    // Health damage from starvation
    if (Stats.Hunger <= 0.0f)
    {
        Stats.Health = FMath::Max(0.0f, Stats.Health - HealthDecayFromStarvation * DeltaTime);
    }

    // Health damage from dehydration (more severe than starvation)
    if (Stats.Thirst <= 0.0f)
    {
        Stats.Health = FMath::Max(0.0f, Stats.Health - HealthDecayFromDehydration * DeltaTime);
    }

    // Death check
    if (Stats.Health <= 0.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Dead;
    }
}

void USurvivalComponent::UpdateStatus()
{
    if (Stats.Health <= 0.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Dead;
        return;
    }

    // Priority: Critical > Panicked > Exhausted > Thirsty > Hungry > Healthy
    if (Stats.Health < 20.0f || Stats.Thirst <= 0.0f || Stats.Hunger <= 0.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Critical;
    }
    else if (Stats.Fear > 70.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Panicked;
    }
    else if (Stats.Stamina < 10.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Exhausted;
    }
    else if (Stats.Thirst < 30.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Thirsty;
    }
    else if (Stats.Hunger < 30.0f)
    {
        CurrentStatus = ECore_SurvivalStatus::Hungry;
    }
    else
    {
        CurrentStatus = ECore_SurvivalStatus::Healthy;
    }
}

// --- Modifiers ---

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (!IsAlive()) return;
    Stats.Health = FMath::Max(0.0f, Stats.Health - FMath::Abs(Amount));
    UpdateStatus();
}

void USurvivalComponent::ConsumeFood(float NutritionValue)
{
    Stats.Hunger = FMath::Min(100.0f, Stats.Hunger + FMath::Abs(NutritionValue));
    UpdateStatus();
}

void USurvivalComponent::DrinkWater(float HydrationValue)
{
    Stats.Thirst = FMath::Min(100.0f, Stats.Thirst + FMath::Abs(HydrationValue));
    UpdateStatus();
}

void USurvivalComponent::AddFear(float FearAmount)
{
    Stats.Fear = FMath::Min(100.0f, Stats.Fear + FMath::Abs(FearAmount));
    UpdateStatus();
}

void USurvivalComponent::ReduceFear(float FearAmount)
{
    Stats.Fear = FMath::Max(0.0f, Stats.Fear - FMath::Abs(FearAmount));
    UpdateStatus();
}

void USurvivalComponent::SpendStamina(float Amount)
{
    Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - FMath::Abs(Amount));
    if (Stats.Stamina <= 0.0f)
    {
        bIsSprinting = false;
    }
    UpdateStatus();
}

void USurvivalComponent::RecoverStamina(float Amount)
{
    Stats.Stamina = FMath::Min(100.0f, Stats.Stamina + FMath::Abs(Amount));
    UpdateStatus();
}
