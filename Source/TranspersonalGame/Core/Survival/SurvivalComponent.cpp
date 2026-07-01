// SurvivalComponent.cpp — Transpersonal Game Studio
// Core Systems Programmer — Agent #03
// Full implementation: hunger/thirst/stamina/fear drain + starvation damage
// Realistic prehistoric survival — no spiritual content.

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f;  // Every frame for smooth UI

    // Initialize stats to full
    Stats.Health    = 100.0f;
    Stats.Hunger    = 100.0f;
    Stats.Thirst    = 100.0f;
    Stats.Stamina   = 100.0f;
    Stats.Fear      = 0.0f;
    Stats.Temperature = 37.0f;

    CurrentStatus = ECore_SurvivalStatus::Healthy;
    bIsSprinting  = false;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    UpdateStatus();
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (IsDead()) return;

    TickHunger(DeltaTime);
    TickThirst(DeltaTime);
    TickStamina(DeltaTime);
    TickFear(DeltaTime);
    TickStarvationDamage(DeltaTime);
    ClampStats();
    UpdateStatus();
}

// ─── Hunger ──────────────────────────────────────────────────────────────────

void USurvivalComponent::TickHunger(float DeltaTime)
{
    float PrevHunger = Stats.Hunger;
    Stats.Hunger -= HungerDrainRate * DeltaTime;

    if (FMath::Abs(PrevHunger - Stats.Hunger) > 0.5f)
    {
        OnStatChanged.Broadcast(FName("Hunger"), Stats.Hunger);
    }
}

// ─── Thirst ───────────────────────────────────────────────────────────────────

void USurvivalComponent::TickThirst(float DeltaTime)
{
    float PrevThirst = Stats.Thirst;
    Stats.Thirst -= ThirstDrainRate * DeltaTime;

    if (FMath::Abs(PrevThirst - Stats.Thirst) > 0.5f)
    {
        OnStatChanged.Broadcast(FName("Thirst"), Stats.Thirst);
    }
}

// ─── Stamina ──────────────────────────────────────────────────────────────────

void USurvivalComponent::TickStamina(float DeltaTime)
{
    float PrevStamina = Stats.Stamina;

    if (bIsSprinting)
    {
        Stats.Stamina -= StaminaDrainRate * DeltaTime;
        // Force stop sprinting if exhausted
        if (Stats.Stamina <= 0.0f)
        {
            bIsSprinting = false;
            Stats.Stamina = 0.0f;
        }
    }
    else
    {
        // Regen is slower when hungry/thirsty
        float RegenMultiplier = 1.0f;
        if (Stats.Hunger < 30.0f) RegenMultiplier *= 0.5f;
        if (Stats.Thirst < 20.0f) RegenMultiplier *= 0.3f;

        Stats.Stamina += StaminaRegenRate * RegenMultiplier * DeltaTime;
    }

    if (FMath::Abs(PrevStamina - Stats.Stamina) > 0.5f)
    {
        OnStatChanged.Broadcast(FName("Stamina"), Stats.Stamina);
    }
}

// ─── Fear ─────────────────────────────────────────────────────────────────────

void USurvivalComponent::TickFear(float DeltaTime)
{
    if (Stats.Fear > 0.0f)
    {
        Stats.Fear -= FearDecayRate * DeltaTime;
        // High fear drains stamina faster
        if (Stats.Fear > 70.0f)
        {
            Stats.Stamina -= 0.3f * DeltaTime;
        }
    }
}

// ─── Starvation / Dehydration Damage ─────────────────────────────────────────

void USurvivalComponent::TickStarvationDamage(float DeltaTime)
{
    float DamageThisTick = 0.0f;

    if (Stats.Hunger <= 0.0f)
    {
        DamageThisTick += StarvationDamageRate * DeltaTime;
    }

    if (Stats.Thirst <= 0.0f)
    {
        DamageThisTick += DehydrationDamageRate * DeltaTime;
    }

    if (DamageThisTick > 0.0f)
    {
        Stats.Health -= DamageThisTick;
        OnStatChanged.Broadcast(FName("Health"), Stats.Health);
    }
}

// ─── Clamp ────────────────────────────────────────────────────────────────────

void USurvivalComponent::ClampStats()
{
    Stats.Health    = FMath::Clamp(Stats.Health,    0.0f, 100.0f);
    Stats.Hunger    = FMath::Clamp(Stats.Hunger,    0.0f, 100.0f);
    Stats.Thirst    = FMath::Clamp(Stats.Thirst,    0.0f, 100.0f);
    Stats.Stamina   = FMath::Clamp(Stats.Stamina,   0.0f, 100.0f);
    Stats.Fear      = FMath::Clamp(Stats.Fear,      0.0f, 100.0f);
    Stats.Temperature = FMath::Clamp(Stats.Temperature, 30.0f, 42.0f);
}

// ─── Status Update ────────────────────────────────────────────────────────────

void USurvivalComponent::UpdateStatus()
{
    ECore_SurvivalStatus NewStatus = ECore_SurvivalStatus::Healthy;

    if (Stats.Health <= 20.0f)
    {
        NewStatus = ECore_SurvivalStatus::Critical;
    }
    else if (Stats.Stamina <= 5.0f)
    {
        NewStatus = ECore_SurvivalStatus::Exhausted;
    }
    else if (Stats.Thirst <= 0.0f)
    {
        NewStatus = ECore_SurvivalStatus::Dehydrated;
    }
    else if (Stats.Hunger <= 0.0f)
    {
        NewStatus = ECore_SurvivalStatus::Starving;
    }
    else if (Stats.Thirst < 25.0f)
    {
        NewStatus = ECore_SurvivalStatus::Thirsty;
    }
    else if (Stats.Hunger < 25.0f)
    {
        NewStatus = ECore_SurvivalStatus::Hungry;
    }

    if (NewStatus != CurrentStatus)
    {
        CurrentStatus = NewStatus;
        OnStatusChanged.Broadcast(CurrentStatus);
    }
}

// ─── Public Modifiers ─────────────────────────────────────────────────────────

void USurvivalComponent::ConsumeFood(float Amount)
{
    if (Amount <= 0.0f) return;
    Stats.Hunger = FMath::Min(Stats.Hunger + Amount, 100.0f);
    OnStatChanged.Broadcast(FName("Hunger"), Stats.Hunger);
    UpdateStatus();
}

void USurvivalComponent::DrinkWater(float Amount)
{
    if (Amount <= 0.0f) return;
    Stats.Thirst = FMath::Min(Stats.Thirst + Amount, 100.0f);
    OnStatChanged.Broadcast(FName("Thirst"), Stats.Thirst);
    UpdateStatus();
}

void USurvivalComponent::TakeDamage_Survival(float Amount)
{
    if (Amount <= 0.0f) return;
    Stats.Health = FMath::Max(Stats.Health - Amount, 0.0f);
    OnStatChanged.Broadcast(FName("Health"), Stats.Health);
    UpdateStatus();
}

void USurvivalComponent::AddFear(float Amount)
{
    if (Amount <= 0.0f) return;
    Stats.Fear = FMath::Min(Stats.Fear + Amount, 100.0f);
    OnStatChanged.Broadcast(FName("Fear"), Stats.Fear);
}

void USurvivalComponent::SetSprinting(bool bNewSprinting)
{
    // Cannot sprint if exhausted or no stamina
    if (bNewSprinting && !CanSprint())
    {
        bIsSprinting = false;
        return;
    }
    bIsSprinting = bNewSprinting;
}
