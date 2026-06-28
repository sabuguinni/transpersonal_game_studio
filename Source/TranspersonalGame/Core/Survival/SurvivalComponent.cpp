// SurvivalComponent.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Full implementation of survival stats: hunger/thirst drain, stamina regen,
// fear decay, starvation damage, and state machine transitions.

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;  // Tick every 100ms — sufficient for survival stats
}

// ---------------------------------------------------------------------------
// BeginPlay
// ---------------------------------------------------------------------------

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    UpdateSurvivalState();
}

// ---------------------------------------------------------------------------
// TickComponent
// ---------------------------------------------------------------------------

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead)
    {
        return;
    }

    TickHunger(DeltaTime);
    TickThirst(DeltaTime);
    TickStamina(DeltaTime);
    TickFear(DeltaTime);
    TickStarvationDamage(DeltaTime);
    UpdateSurvivalState();
}

// ---------------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------------

void USurvivalComponent::ApplyDamage(float DamageAmount)
{
    if (bIsDead || DamageAmount <= 0.0f)
    {
        return;
    }

    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
    BroadcastStatChange(ECore_SurvivalStat::Health, Health);

    if (Health <= 0.0f)
    {
        Die();
    }
}

void USurvivalComponent::Heal(float HealAmount)
{
    if (bIsDead || HealAmount <= 0.0f)
    {
        return;
    }

    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
    BroadcastStatChange(ECore_SurvivalStat::Health, Health);
}

void USurvivalComponent::Eat(float NutritionValue)
{
    if (bIsDead || NutritionValue <= 0.0f)
    {
        return;
    }

    Hunger = FMath::Clamp(Hunger + NutritionValue, 0.0f, 100.0f);
    BroadcastStatChange(ECore_SurvivalStat::Hunger, Hunger);
    AccumulatedStarvationTime = 0.0f;
}

void USurvivalComponent::Drink(float HydrationValue)
{
    if (bIsDead || HydrationValue <= 0.0f)
    {
        return;
    }

    Thirst = FMath::Clamp(Thirst + HydrationValue, 0.0f, 100.0f);
    BroadcastStatChange(ECore_SurvivalStat::Thirst, Thirst);
    AccumulatedDehydrationTime = 0.0f;
}

void USurvivalComponent::AddFear(float FearAmount)
{
    if (bIsDead || FearAmount <= 0.0f)
    {
        return;
    }

    Fear = FMath::Clamp(Fear + FearAmount, 0.0f, 100.0f);
    BroadcastStatChange(ECore_SurvivalStat::Fear, Fear);
}

void USurvivalComponent::SetSprinting(bool bSprinting)
{
    bIsSprinting = bSprinting;
}

float USurvivalComponent::GetStatValue(ECore_SurvivalStat Stat) const
{
    switch (Stat)
    {
        case ECore_SurvivalStat::Health:        return Health;
        case ECore_SurvivalStat::Hunger:        return Hunger;
        case ECore_SurvivalStat::Thirst:        return Thirst;
        case ECore_SurvivalStat::Stamina:       return Stamina;
        case ECore_SurvivalStat::Temperature:   return BodyTemperature;
        case ECore_SurvivalStat::Fear:          return Fear;
        default:                                return 0.0f;
    }
}

float USurvivalComponent::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
}

float USurvivalComponent::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f;
}

// ---------------------------------------------------------------------------
// Private Tick Helpers
// ---------------------------------------------------------------------------

void USurvivalComponent::TickHunger(float DeltaTime)
{
    // Hunger drains faster when sprinting
    float DrainMultiplier = bIsSprinting ? 2.0f : 1.0f;
    Hunger = FMath::Clamp(Hunger - (HungerDrainRate * DrainMultiplier * DeltaTime), 0.0f, 100.0f);
    BroadcastStatChange(ECore_SurvivalStat::Hunger, Hunger);

    if (Hunger <= 0.0f)
    {
        AccumulatedStarvationTime += DeltaTime;
    }
}

void USurvivalComponent::TickThirst(float DeltaTime)
{
    // Thirst drains faster in hot environments (simplified: always drain)
    float DrainMultiplier = bIsSprinting ? 1.5f : 1.0f;
    Thirst = FMath::Clamp(Thirst - (ThirstDrainRate * DrainMultiplier * DeltaTime), 0.0f, 100.0f);
    BroadcastStatChange(ECore_SurvivalStat::Thirst, Thirst);

    if (Thirst <= 0.0f)
    {
        AccumulatedDehydrationTime += DeltaTime;
    }
}

void USurvivalComponent::TickStamina(float DeltaTime)
{
    if (bIsSprinting)
    {
        // Drain stamina while sprinting
        Stamina = FMath::Clamp(Stamina - (StaminaDrainRate * DeltaTime), 0.0f, MaxStamina);

        // Force stop sprinting if stamina depleted
        if (Stamina <= 0.0f)
        {
            bIsSprinting = false;
        }
    }
    else
    {
        // Regen stamina when not sprinting — slower if hungry/thirsty
        float RegenMultiplier = 1.0f;
        if (Hunger < 20.0f) RegenMultiplier *= 0.5f;
        if (Thirst < 20.0f) RegenMultiplier *= 0.5f;

        Stamina = FMath::Clamp(Stamina + (StaminaRegenRate * RegenMultiplier * DeltaTime), 0.0f, MaxStamina);
    }

    BroadcastStatChange(ECore_SurvivalStat::Stamina, Stamina);
}

void USurvivalComponent::TickFear(float DeltaTime)
{
    if (Fear > 0.0f)
    {
        Fear = FMath::Clamp(Fear - (FearDecayRate * DeltaTime), 0.0f, 100.0f);
        BroadcastStatChange(ECore_SurvivalStat::Fear, Fear);
    }
}

void USurvivalComponent::TickStarvationDamage(float DeltaTime)
{
    // Starvation: deal 1 HP damage every 10 seconds of zero hunger
    if (AccumulatedStarvationTime >= 10.0f)
    {
        ApplyDamage(1.0f);
        AccumulatedStarvationTime = 0.0f;
    }

    // Dehydration: deal 2 HP damage every 8 seconds of zero thirst (more severe)
    if (AccumulatedDehydrationTime >= 8.0f)
    {
        ApplyDamage(2.0f);
        AccumulatedDehydrationTime = 0.0f;
    }
}

// ---------------------------------------------------------------------------
// State Machine
// ---------------------------------------------------------------------------

void USurvivalComponent::UpdateSurvivalState()
{
    ECore_SurvivalState NewState = ECore_SurvivalState::Healthy;

    if (bIsDead)
    {
        NewState = ECore_SurvivalState::Dead;
    }
    else if (Fear >= 80.0f)
    {
        NewState = ECore_SurvivalState::Panicked;
    }
    else if (Thirst <= 0.0f)
    {
        NewState = ECore_SurvivalState::Dehydrated;
    }
    else if (Hunger <= 0.0f)
    {
        NewState = ECore_SurvivalState::Starving;
    }
    else if (Hunger < 25.0f)
    {
        NewState = ECore_SurvivalState::Hungry;
    }
    else if (Stamina <= 5.0f)
    {
        NewState = ECore_SurvivalState::Exhausted;
    }
    else if (BodyTemperature < 35.0f)
    {
        NewState = ECore_SurvivalState::Hypothermic;
    }
    else if (BodyTemperature > 40.0f)
    {
        NewState = ECore_SurvivalState::Hyperthermic;
    }

    if (NewState != CurrentState)
    {
        CurrentState = NewState;
        OnSurvivalStateChanged.Broadcast(CurrentState);
    }
}

void USurvivalComponent::BroadcastStatChange(ECore_SurvivalStat Stat, float NewValue)
{
    OnStatChanged.Broadcast(Stat, NewValue);
}

void USurvivalComponent::Die()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;
    bIsSprinting = false;
    CurrentState = ECore_SurvivalState::Dead;
    OnSurvivalStateChanged.Broadcast(ECore_SurvivalState::Dead);
    OnCharacterDied.Broadcast();
}
