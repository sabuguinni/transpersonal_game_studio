// SurvivalComponent.cpp
// Transpersonal Game Studio — Core Systems Programmer (Agent #03)
// Prehistoric survival stats: hunger, thirst, stamina, fear, temperature.
// Drains every second; applies starvation/dehydration damage at zero.

#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // We handle our own interval

    Stats.Health      = 100.f;
    Stats.Hunger      = 100.f;
    Stats.Thirst      = 100.f;
    Stats.Stamina     = 100.f;
    Stats.Fear        = 0.f;
    Stats.Temperature = 37.f;

    CurrentStatus = ECore_SurvivalStatus::Healthy;
    bIsDead       = false;
    AccumulatedTime = 0.f;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    AccumulatedTime += DeltaTime;
    if (AccumulatedTime >= TickInterval)
    {
        DrainStats(AccumulatedTime);
        AccumulatedTime = 0.f;
    }
}

// ── Stat modifiers ────────────────────────────────────────────────────────────

void USurvivalComponent::ConsumeFood(float Amount)
{
    Stats.Hunger = FMath::Clamp(Stats.Hunger + Amount, 0.f, 100.f);
    UpdateStatus();
}

void USurvivalComponent::DrinkWater(float Amount)
{
    Stats.Thirst = FMath::Clamp(Stats.Thirst + Amount, 0.f, 100.f);
    UpdateStatus();
}

void USurvivalComponent::RestoreStamina(float Amount)
{
    Stats.Stamina = FMath::Clamp(Stats.Stamina + Amount, 0.f, 100.f);
    UpdateStatus();
}

void USurvivalComponent::ApplyFear(float Amount)
{
    Stats.Fear = FMath::Clamp(Stats.Fear + Amount, 0.f, 100.f);
    if (Stats.Fear >= FearPanicThreshold)
    {
        OnFearPanic(Stats.Fear);
    }
    UpdateStatus();
}

void USurvivalComponent::ReduceFear(float Amount)
{
    Stats.Fear = FMath::Clamp(Stats.Fear - Amount, 0.f, 100.f);
    UpdateStatus();
}

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (bIsDead) return;
    Stats.Health = FMath::Clamp(Stats.Health - Amount, 0.f, 100.f);

    if (Stats.Health <= CriticalHealthThreshold && Stats.Health > 0.f)
    {
        OnCriticalHealth(Stats.Health);
    }

    if (Stats.Health <= 0.f)
    {
        bIsDead = true;
        OnDeath();
    }

    UpdateStatus();
}

void USurvivalComponent::HealHealth(float Amount)
{
    if (bIsDead) return;
    Stats.Health = FMath::Clamp(Stats.Health + Amount, 0.f, 100.f);
    UpdateStatus();
}

void USurvivalComponent::UseStamina(float Amount)
{
    Stats.Stamina = FMath::Clamp(Stats.Stamina - Amount, 0.f, 100.f);
    if (Stats.Stamina <= StaminaExhaustionThreshold)
    {
        OnStaminaExhausted();
    }
    UpdateStatus();
}

// ── Internal drain logic ──────────────────────────────────────────────────────

void USurvivalComponent::DrainStats(float DeltaSeconds)
{
    // Hunger drain
    Stats.Hunger = FMath::Clamp(Stats.Hunger - (HungerDrainPerSecond * DeltaSeconds), 0.f, 100.f);
    if (Stats.Hunger <= HungerWarningThreshold)
    {
        OnHungerWarning(Stats.Hunger);
    }

    // Thirst drain (faster than hunger — dehydration kills quicker)
    Stats.Thirst = FMath::Clamp(Stats.Thirst - (ThirstDrainPerSecond * DeltaSeconds), 0.f, 100.f);
    if (Stats.Thirst <= ThirstWarningThreshold)
    {
        OnThirstWarning(Stats.Thirst);
    }

    // Stamina regen when not running
    if (!bIsRunning)
    {
        Stats.Stamina = FMath::Clamp(Stats.Stamina + (StaminaRegenPerSecond * DeltaSeconds), 0.f, 100.f);
    }

    // Fear decay (calm down over time when not threatened)
    Stats.Fear = FMath::Clamp(Stats.Fear - (FearDecayPerSecond * DeltaSeconds), 0.f, 100.f);

    // Apply starvation / dehydration damage
    ApplyStarvationDamage();
    ApplyDehydrationDamage();

    ClampStats();
    UpdateStatus();
}

void USurvivalComponent::ApplyStarvationDamage()
{
    if (Stats.Hunger <= 0.f && !bIsDead)
    {
        Stats.Health = FMath::Clamp(Stats.Health - StarvationDamagePerSecond, 0.f, 100.f);
        if (Stats.Health <= 0.f)
        {
            bIsDead = true;
            OnDeath();
        }
    }
}

void USurvivalComponent::ApplyDehydrationDamage()
{
    if (Stats.Thirst <= 0.f && !bIsDead)
    {
        Stats.Health = FMath::Clamp(Stats.Health - DehydrationDamagePerSecond, 0.f, 100.f);
        if (Stats.Health <= 0.f)
        {
            bIsDead = true;
            OnDeath();
        }
    }
}

void USurvivalComponent::UpdateStatus()
{
    if (bIsDead)
    {
        CurrentStatus = ECore_SurvivalStatus::Critical;
        return;
    }

    if (Stats.Health <= CriticalHealthThreshold)
    {
        CurrentStatus = ECore_SurvivalStatus::Critical;
    }
    else if (Stats.Fear >= FearPanicThreshold)
    {
        CurrentStatus = ECore_SurvivalStatus::Panicking;
    }
    else if (Stats.Stamina <= StaminaExhaustionThreshold)
    {
        CurrentStatus = ECore_SurvivalStatus::Exhausted;
    }
    else if (Stats.Thirst <= 0.f)
    {
        CurrentStatus = ECore_SurvivalStatus::Dehydrated;
    }
    else if (Stats.Hunger <= 0.f)
    {
        CurrentStatus = ECore_SurvivalStatus::Starving;
    }
    else if (Stats.Thirst <= ThirstWarningThreshold)
    {
        CurrentStatus = ECore_SurvivalStatus::Thirsty;
    }
    else if (Stats.Hunger <= HungerWarningThreshold)
    {
        CurrentStatus = ECore_SurvivalStatus::Hungry;
    }
    else
    {
        CurrentStatus = ECore_SurvivalStatus::Healthy;
    }
}

void USurvivalComponent::ClampStats()
{
    Stats.Health      = FMath::Clamp(Stats.Health,      0.f, 100.f);
    Stats.Hunger      = FMath::Clamp(Stats.Hunger,      0.f, 100.f);
    Stats.Thirst      = FMath::Clamp(Stats.Thirst,      0.f, 100.f);
    Stats.Stamina     = FMath::Clamp(Stats.Stamina,     0.f, 100.f);
    Stats.Fear        = FMath::Clamp(Stats.Fear,        0.f, 100.f);
    Stats.Temperature = FMath::Clamp(Stats.Temperature, -50.f, 60.f);
}

void USurvivalComponent::PrintSurvivalStats() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== SurvivalComponent Stats ==="));
    UE_LOG(LogTemp, Warning, TEXT("  Health:  %.1f"), Stats.Health);
    UE_LOG(LogTemp, Warning, TEXT("  Hunger:  %.1f"), Stats.Hunger);
    UE_LOG(LogTemp, Warning, TEXT("  Thirst:  %.1f"), Stats.Thirst);
    UE_LOG(LogTemp, Warning, TEXT("  Stamina: %.1f"), Stats.Stamina);
    UE_LOG(LogTemp, Warning, TEXT("  Fear:    %.1f"), Stats.Fear);
    UE_LOG(LogTemp, Warning, TEXT("  Temp:    %.1f C"), Stats.Temperature);
    UE_LOG(LogTemp, Warning, TEXT("  Status:  %d"), (int32)CurrentStatus);
}
