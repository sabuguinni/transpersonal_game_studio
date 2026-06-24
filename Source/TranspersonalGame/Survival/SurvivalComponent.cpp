// SurvivalComponent.cpp
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260624_004
// Tick-based survival stats: hunger, thirst, temperature, stamina
// Designed for performance: uses FTimerHandle ticks (not per-frame), no allocations in hot path

#include "SurvivalComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

USurvivalComponent::USurvivalComponent()
{
    // Tick at 4Hz — survival stats don't need per-frame precision
    PrimaryComponentTick.bCanEverTick = false;

    // Initial stat values — full survival bars
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    MaxHunger = 100.0f;
    CurrentHunger = 100.0f;
    MaxThirst = 100.0f;
    CurrentThirst = 100.0f;
    MaxStamina = 100.0f;
    CurrentStamina = 100.0f;
    CurrentTemperature = 37.0f;   // Celsius — human body temp
    SafeTempMin = 10.0f;
    SafeTempMax = 45.0f;

    // Depletion rates per second
    HungerDepletionRate = 0.5f;    // Full hunger depletes in ~200s (~3.3 min)
    ThirstDepletionRate = 0.8f;    // Full thirst depletes in ~125s (~2 min)
    StaminaRegenRate = 12.0f;      // Regen when resting
    StaminaDrainRate = 20.0f;      // Drain when sprinting
    HealthRegenRate = 1.5f;        // Slow passive regen when fed/hydrated

    // Damage thresholds
    StarveHealthDrain = 2.0f;      // Health drain/s when starving
    DehydrateHealthDrain = 3.0f;   // Health drain/s when dehydrated
    HypothermiaHealthDrain = 4.0f; // Health drain/s in extreme cold
    HyperthermiaHealthDrain = 3.5f;// Health drain/s in extreme heat

    // State flags
    bIsSprinting = false;
    bIsResting = false;
    bIsStarving = false;
    bIsDehydrated = false;
    bIsHypothermic = false;
    bIsHyperthermic = false;

    // Timer interval — 0.25s (4Hz)
    SurvivalTickInterval = 0.25f;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start survival tick timer — 4Hz, not per-frame
    GetWorld()->GetTimerManager().SetTimer(
        SurvivalTickTimerHandle,
        this,
        &USurvivalComponent::SurvivalTick,
        SurvivalTickInterval,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Initialized on [%s] — 4Hz tick started"),
        *GetOwner()->GetActorLabel());
}

void USurvivalComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorld()->GetTimerManager().ClearTimer(SurvivalTickTimerHandle);
}

void USurvivalComponent::SurvivalTick()
{
    const float DeltaTime = SurvivalTickInterval;

    // --- HUNGER ---
    CurrentHunger = FMath::Max(0.0f, CurrentHunger - HungerDepletionRate * DeltaTime);
    bIsStarving = (CurrentHunger <= 0.0f);

    // --- THIRST ---
    CurrentThirst = FMath::Max(0.0f, CurrentThirst - ThirstDepletionRate * DeltaTime);
    bIsDehydrated = (CurrentThirst <= 0.0f);

    // --- STAMINA ---
    if (bIsSprinting)
    {
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - StaminaDrainRate * DeltaTime);
        if (CurrentStamina <= 0.0f)
        {
            // Force stop sprint — no stamina left
            SetSprinting(false);
            OnStaminaExhausted.Broadcast();
        }
    }
    else if (bIsResting)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * 2.0f * DeltaTime);
    }
    else
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
    }

    // --- TEMPERATURE EFFECTS ---
    bIsHypothermic = (CurrentTemperature < SafeTempMin);
    bIsHyperthermic = (CurrentTemperature > SafeTempMax);

    // --- HEALTH CONSEQUENCES ---
    float HealthDelta = 0.0f;

    if (bIsStarving)
    {
        HealthDelta -= StarveHealthDrain * DeltaTime;
    }
    if (bIsDehydrated)
    {
        HealthDelta -= DehydrateHealthDrain * DeltaTime;
    }
    if (bIsHypothermic)
    {
        HealthDelta -= HypothermiaHealthDrain * DeltaTime;
    }
    if (bIsHyperthermic)
    {
        HealthDelta -= HyperthermiaHealthDrain * DeltaTime;
    }

    // Passive health regen when all stats are good
    if (!bIsStarving && !bIsDehydrated && !bIsHypothermic && !bIsHyperthermic && CurrentHealth < MaxHealth)
    {
        HealthDelta += HealthRegenRate * DeltaTime;
    }

    if (HealthDelta != 0.0f)
    {
        ApplyHealthDelta(HealthDelta);
    }

    // Broadcast stat update event (for HUD)
    OnSurvivalStatsUpdated.Broadcast(CurrentHealth, CurrentHunger, CurrentThirst, CurrentStamina, CurrentTemperature);
}

void USurvivalComponent::ApplyHealthDelta(float Delta)
{
    const float PrevHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth + Delta, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f && PrevHealth > 0.0f)
    {
        OnDeath.Broadcast();
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent: [%s] has DIED from survival stat depletion"),
            *GetOwner()->GetActorLabel());
    }
}

void USurvivalComponent::ConsumeFood(float FoodValue)
{
    CurrentHunger = FMath::Min(MaxHunger, CurrentHunger + FoodValue);
    bIsStarving = false;
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: [%s] consumed food +%.1f — hunger now %.1f"),
        *GetOwner()->GetActorLabel(), FoodValue, CurrentHunger);
}

void USurvivalComponent::ConsumeWater(float WaterValue)
{
    CurrentThirst = FMath::Min(MaxThirst, CurrentThirst + WaterValue);
    bIsDehydrated = false;
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: [%s] consumed water +%.1f — thirst now %.1f"),
        *GetOwner()->GetActorLabel(), WaterValue, CurrentThirst);
}

void USurvivalComponent::SetAmbientTemperature(float NewTemperature)
{
    // Gradual temperature change — body adapts slowly
    const float TempChangeRate = 0.1f;
    CurrentTemperature = FMath::FInterpTo(CurrentTemperature, NewTemperature, SurvivalTickInterval, TempChangeRate);
}

void USurvivalComponent::SetSprinting(bool bSprinting)
{
    bIsSprinting = bSprinting;

    // Sprinting also increases thirst faster
    if (bSprinting)
    {
        ThirstDepletionRate = 1.6f;  // Double thirst drain while sprinting
    }
    else
    {
        ThirstDepletionRate = 0.8f;  // Reset to base rate
    }
}

void USurvivalComponent::SetResting(bool bResting)
{
    bIsResting = bResting;
    if (bResting)
    {
        // Resting slows hunger/thirst drain
        HungerDepletionRate = 0.2f;
        ThirstDepletionRate = 0.3f;
    }
    else
    {
        HungerDepletionRate = 0.5f;
        ThirstDepletionRate = 0.8f;
    }
}

float USurvivalComponent::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float USurvivalComponent::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (CurrentHunger / MaxHunger) : 0.0f;
}

float USurvivalComponent::GetThirstPercent() const
{
    return (MaxThirst > 0.0f) ? (CurrentThirst / MaxThirst) : 0.0f;
}

float USurvivalComponent::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}
