// SurvivalComponent.cpp
// Core Systems Programmer — Agent #3
// Full implementation of survival stats: hunger/thirst drain, stamina recovery,
// fear decay, starvation/dehydration damage, death trigger.

#include "Core/Survival/SurvivalComponent.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // We use a timer, not per-frame tick
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start survival timer — fires every TickInterval seconds
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            SurvivalTimerHandle,
            this,
            &USurvivalComponent::SurvivalTick,
            TickInterval,
            true // looping
        );
    }
}

void USurvivalComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SurvivalTimerHandle);
    }
    Super::EndPlay(EndPlayReason);
}

// --- Core survival tick (called every TickInterval seconds) ---
void USurvivalComponent::SurvivalTick()
{
    if (!bIsAlive) return;

    // Drain hunger and thirst
    Hunger -= HungerDrainRate * TickInterval;
    Thirst -= ThirstDrainRate * TickInterval;
    ClampStat(Hunger);
    ClampStat(Thirst);

    OnHungerChanged.Broadcast(Hunger);
    OnThirstChanged.Broadcast(Thirst);

    // Starvation damage
    if (Hunger <= 0.0f)
    {
        ApplyDamage(StarvationDamageRate * TickInterval);
    }

    // Dehydration damage (worse than starvation)
    if (Thirst <= 0.0f)
    {
        ApplyDamage(DehydrationDamageRate * TickInterval);
    }

    // Stamina: drain if sprinting, restore if not
    if (bIsSprinting)
    {
        Stamina -= SprintStaminaDrainRate * TickInterval;
        ClampStat(Stamina);
        if (Stamina <= 0.0f)
        {
            // Auto-stop sprint when exhausted
            bIsSprinting = false;
        }
    }
    else
    {
        Stamina += StaminaRestoreRate * TickInterval;
        ClampStat(Stamina);
    }
    OnStaminaChanged.Broadcast(Stamina);

    // Fear decay — fear naturally reduces over time when safe
    if (Fear > 0.0f)
    {
        Fear -= FearDecayRate * TickInterval;
        ClampStat(Fear);
        OnFearChanged.Broadcast(Fear);
    }
}

// --- Stat modifiers ---

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (!bIsAlive || Amount <= 0.0f) return;

    Health -= Amount;
    ClampStat(Health);
    OnHealthChanged.Broadcast(Health);

    if (Health <= 0.0f)
    {
        TriggerDeath();
    }
}

void USurvivalComponent::Heal(float Amount)
{
    if (!bIsAlive || Amount <= 0.0f) return;

    Health += Amount;
    ClampStat(Health);
    OnHealthChanged.Broadcast(Health);
}

void USurvivalComponent::ConsumeFood(float Amount)
{
    if (Amount <= 0.0f) return;

    Hunger += Amount;
    ClampStat(Hunger);
    OnHungerChanged.Broadcast(Hunger);
}

void USurvivalComponent::ConsumeWater(float Amount)
{
    if (Amount <= 0.0f) return;

    Thirst += Amount;
    ClampStat(Thirst);
    OnThirstChanged.Broadcast(Thirst);
}

void USurvivalComponent::DrainStamina(float Amount)
{
    if (Amount <= 0.0f) return;

    Stamina -= Amount;
    ClampStat(Stamina);
    OnStaminaChanged.Broadcast(Stamina);
}

void USurvivalComponent::RestoreStamina(float Amount)
{
    if (Amount <= 0.0f) return;

    Stamina += Amount;
    ClampStat(Stamina);
    OnStaminaChanged.Broadcast(Stamina);
}

void USurvivalComponent::AddFear(float Amount)
{
    if (Amount <= 0.0f) return;

    Fear += Amount;
    ClampStat(Fear);
    OnFearChanged.Broadcast(Fear);

    // High fear drains stamina faster (panic response)
    if (Fear > 75.0f)
    {
        DrainStamina(Amount * 0.2f);
    }
}

void USurvivalComponent::ReduceFear(float Amount)
{
    if (Amount <= 0.0f) return;

    Fear -= Amount;
    ClampStat(Fear);
    OnFearChanged.Broadcast(Fear);
}

void USurvivalComponent::SetSprinting(bool bSprinting)
{
    // Can only start sprinting if stamina > 5 and alive
    if (bSprinting && !CanSprint())
    {
        bIsSprinting = false;
        return;
    }
    bIsSprinting = bSprinting;
}

// --- Internal helpers ---

void USurvivalComponent::ClampStat(float& Stat)
{
    Stat = FMath::Clamp(Stat, 0.0f, 100.0f);
}

void USurvivalComponent::TriggerDeath()
{
    if (!bIsAlive) return; // Guard against double-death

    bIsAlive = false;
    bIsSprinting = false;

    // Stop the survival timer — no more ticking after death
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SurvivalTimerHandle);
    }

    // Broadcast death event — TranspersonalCharacter listens and triggers ragdoll
    OnPlayerDied.Broadcast();

    UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent: Player has died."));
}
