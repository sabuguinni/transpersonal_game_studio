// SurvivalComponent.cpp — Implementation of survival stats system
// Agent #3 — Core Systems Programmer | Cycle PROD_AUTO_20260627_005

#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second for performance
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize stats to full
    Stats.Health        = Stats.MaxHealth;
    Stats.Hunger        = 100.f;
    Stats.Thirst        = 100.f;
    Stats.Stamina       = Stats.MaxStamina;
    Stats.BodyTemperature = 37.f;
    Stats.Fear          = 0.f;

    CurrentState = ECore_SurvivalState::Healthy;
    bIsDead = false;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    DrainStats(DeltaTime);
    ApplyPassiveEffects(DeltaTime);
    UpdateSurvivalState();
}

// ── Stat Drains ──────────────────────────────────────────────────────────────

void USurvivalComponent::DrainStats(float DeltaTime)
{
    // Hunger and thirst drain over time
    Stats.Hunger = FMath::Max(0.f, Stats.Hunger - HungerDrainPerSecond * DeltaTime);
    Stats.Thirst = FMath::Max(0.f, Stats.Thirst - ThirstDrainPerSecond * DeltaTime);

    // Fear decays naturally when not threatened
    Stats.Fear = FMath::Max(0.f, Stats.Fear - FearDecayPerSecond * DeltaTime);

    // Stamina regenerates when not exhausted (handled separately via ConsumeStamina)
    if (Stats.Stamina < Stats.MaxStamina)
    {
        Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + StaminaRegenPerSecond * DeltaTime);
    }
}

void USurvivalComponent::ApplyPassiveEffects(float DeltaTime)
{
    // Starvation damage
    if (Stats.Hunger <= 0.f)
    {
        Stats.Health = FMath::Max(0.f, Stats.Health - StarvationDamagePerSecond * DeltaTime);
    }

    // Dehydration damage (more severe than starvation)
    if (Stats.Thirst <= 0.f)
    {
        Stats.Health = FMath::Max(0.f, Stats.Health - DehydrationDamagePerSecond * DeltaTime);
    }

    // Hypothermia damage (body temp below 35°C)
    if (Stats.BodyTemperature < 35.f)
    {
        Stats.Health = FMath::Max(0.f, Stats.Health - 0.5f * DeltaTime);
    }

    // Hyperthermia damage (body temp above 40°C)
    if (Stats.BodyTemperature > 40.f)
    {
        Stats.Health = FMath::Max(0.f, Stats.Health - 0.5f * DeltaTime);
    }

    // Death check
    if (Stats.Health <= 0.f && !bIsDead)
    {
        bIsDead = true;
        CurrentState = ECore_SurvivalState::Dead;
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent: %s has died."), *GetOwner()->GetName());
    }
}

void USurvivalComponent::UpdateSurvivalState()
{
    if (bIsDead)
    {
        CurrentState = ECore_SurvivalState::Dead;
        return;
    }

    if (Stats.Fear >= 80.f)
    {
        CurrentState = ECore_SurvivalState::Panicking;
    }
    else if (Stats.BodyTemperature < 35.f)
    {
        CurrentState = ECore_SurvivalState::Hypothermic;
    }
    else if (Stats.BodyTemperature > 40.f)
    {
        CurrentState = ECore_SurvivalState::Hyperthermic;
    }
    else if (Stats.Stamina <= 0.f)
    {
        CurrentState = ECore_SurvivalState::Exhausted;
    }
    else if (Stats.Thirst <= 20.f)
    {
        CurrentState = ECore_SurvivalState::Thirsty;
    }
    else if (Stats.Hunger <= 20.f)
    {
        CurrentState = ECore_SurvivalState::Hungry;
    }
    else
    {
        CurrentState = ECore_SurvivalState::Healthy;
    }
}

// ── Public Modifiers ─────────────────────────────────────────────────────────

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (bIsDead) return;
    Stats.Health = FMath::Max(0.f, Stats.Health - Amount);

    // Damage causes fear spike
    AddFear(Amount * 0.5f);

    if (Stats.Health <= 0.f && !bIsDead)
    {
        bIsDead = true;
        CurrentState = ECore_SurvivalState::Dead;
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent: %s killed by damage (%.1f)."),
               *GetOwner()->GetName(), Amount);
    }
}

void USurvivalComponent::Eat(float NutritionValue)
{
    Stats.Hunger = FMath::Min(100.f, Stats.Hunger + NutritionValue);
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: %s ate — Hunger now %.1f"),
           *GetOwner()->GetName(), Stats.Hunger);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    Stats.Thirst = FMath::Min(100.f, Stats.Thirst + HydrationValue);
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: %s drank — Thirst now %.1f"),
           *GetOwner()->GetName(), Stats.Thirst);
}

void USurvivalComponent::ConsumeStamina(float Amount)
{
    Stats.Stamina = FMath::Max(0.f, Stats.Stamina - Amount);
}

void USurvivalComponent::RestoreStamina(float Amount)
{
    Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + Amount);
}

void USurvivalComponent::AddFear(float Amount)
{
    Stats.Fear = FMath::Min(100.f, Stats.Fear + Amount);
}

void USurvivalComponent::ReduceFear(float Amount)
{
    Stats.Fear = FMath::Max(0.f, Stats.Fear - Amount);
}
