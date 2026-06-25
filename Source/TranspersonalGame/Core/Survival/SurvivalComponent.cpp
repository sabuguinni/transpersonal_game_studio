// SurvivalComponent.cpp
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260625_005
// Prehistoric survival game — USurvivalComponent implementation

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    // Tick is enabled — but we throttle internally to StatTickInterval (1s)
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // check every 0.1s, drain every 1s
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    TickAccumulator = 0.f;
    bIsDead = false;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    TickAccumulator += DeltaTime;
    if (TickAccumulator >= StatTickInterval)
    {
        UpdateStats(TickAccumulator);
        TickAccumulator = 0.f;
    }
}

// ─── Internal Stat Update ─────────────────────────────────────────────────────

void USurvivalComponent::UpdateStats(float DeltaSeconds)
{
    // Hunger drain
    SetStat(Hunger, Hunger - HungerDrainRate * DeltaSeconds, FName("Hunger"));

    // Thirst drain (faster than hunger)
    SetStat(Thirst, Thirst - ThirstDrainRate * DeltaSeconds, FName("Thirst"));

    // Stamina: drain while sprinting, recover otherwise
    if (bIsSprinting)
    {
        SetStat(Stamina, Stamina - StaminaSprintDrainRate * DeltaSeconds, FName("Stamina"));
    }
    else
    {
        SetStat(Stamina, Stamina + StaminaRecoveryRate * DeltaSeconds, FName("Stamina"));
    }

    // Fear decay when no active threat
    if (Fear > 0.f)
    {
        SetStat(Fear, Fear - FearDecayRate * DeltaSeconds, FName("Fear"));
    }

    // Starvation: drain health if hunger or thirst is depleted
    if (Hunger <= 0.f || Thirst <= 0.f)
    {
        float HealthDrain = StarvationHealthDrain * DeltaSeconds;
        // Double drain if BOTH are depleted
        if (Hunger <= 0.f && Thirst <= 0.f)
        {
            HealthDrain *= 2.f;
        }
        SetStat(Health, Health - HealthDrain, FName("Health"));
    }

    // Temperature stress: drain health at high stress levels (>70)
    if (TemperatureStress > 70.f)
    {
        float TempDrain = (TemperatureStress - 70.f) * 0.01f * DeltaSeconds;
        SetStat(Health, Health - TempDrain, FName("Health"));
    }

    // Death check
    if (Health <= 0.f && !bIsDead)
    {
        bIsDead = true;
        Health = 0.f;
        OnOwnerDied.Broadcast();
    }
}

// ─── Helper ───────────────────────────────────────────────────────────────────

void USurvivalComponent::SetStat(float& Stat, float NewValue, FName StatName)
{
    float OldValue = Stat;
    Stat = FMath::Clamp(NewValue, 0.f, 100.f);

    // Broadcast if change is significant (>5 points) to avoid event spam
    if (FMath::Abs(Stat - OldValue) > 5.f)
    {
        OnSurvivalStatChanged.Broadcast(StatName, Stat);
    }
}

// ─── Public API ───────────────────────────────────────────────────────────────

float USurvivalComponent::ApplyDamage(float DamageAmount)
{
    if (bIsDead || DamageAmount <= 0.f) return 0.f;

    float OldHealth = Health;
    SetStat(Health, Health - DamageAmount, FName("Health"));
    float ActualDamage = OldHealth - Health;

    if (Health <= 0.f && !bIsDead)
    {
        bIsDead = true;
        OnOwnerDied.Broadcast();
    }

    return ActualDamage;
}

void USurvivalComponent::ConsumeFood(float Amount)
{
    if (bIsDead || Amount <= 0.f) return;
    SetStat(Hunger, Hunger + Amount, FName("Hunger"));
}

void USurvivalComponent::DrinkWater(float Amount)
{
    if (bIsDead || Amount <= 0.f) return;
    SetStat(Thirst, Thirst + Amount, FName("Thirst"));
}

void USurvivalComponent::AddFear(float Amount)
{
    if (bIsDead || Amount <= 0.f) return;
    SetStat(Fear, Fear + Amount, FName("Fear"));
}

void USurvivalComponent::SetTemperatureStress(float Stress)
{
    SetStat(TemperatureStress, Stress, FName("TemperatureStress"));
}

bool USurvivalComponent::IsInCriticalState() const
{
    return Health < 15.f || Hunger < 15.f || Thirst < 15.f || Stamina < 15.f;
}

float USurvivalComponent::GetOverallDangerScore() const
{
    // Weighted danger: health most important, then thirst, hunger, fear
    float HealthDanger   = (100.f - Health)   * 0.40f;
    float ThirstDanger   = (100.f - Thirst)   * 0.25f;
    float HungerDanger   = (100.f - Hunger)   * 0.20f;
    float FearDanger     = Fear                * 0.10f;
    float TempDanger     = TemperatureStress   * 0.05f;

    float Total = (HealthDanger + ThirstDanger + HungerDanger + FearDanger + TempDanger) / 100.f;
    return FMath::Clamp(Total, 0.f, 1.f);
}
