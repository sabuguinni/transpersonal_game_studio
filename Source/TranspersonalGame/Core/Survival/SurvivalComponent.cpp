// SurvivalComponent.cpp
// Agent #4 — Performance Optimizer — Cycle 011
// Full implementation of USurvivalComponent:
//   - Hunger/Thirst drain per tick
//   - Starvation/Dehydration health damage
//   - Fear decay
//   - Biome environment multipliers
//   - OnDeath / OnHealthChanged delegates

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for survival stats, saves CPU

    // Default stat values
    Health  = 100.0f;
    Hunger  = 100.0f;
    Thirst  = 100.0f;
    Stamina = 100.0f;
    Fear    = 0.0f;

    // Default drain rates (per second)
    HungerDrainPerSecond        = 0.5f;   // Full hunger depletes in ~200s (~3.3 min)
    ThirstDrainPerSecond        = 0.8f;   // Full thirst depletes in ~125s (~2 min)
    FearDecayPerSecond          = 2.0f;   // Fear decays when no threat nearby
    StarvationDamagePerSecond   = 1.0f;   // 1 HP/s when starving
    DehydrationDamagePerSecond  = 2.0f;   // 2 HP/s when dehydrated (more urgent)

    // Environment multipliers (neutral by default)
    EnvThirstMult  = 1.0f;
    EnvStaminaMult = 1.0f;
    EnvFearMult    = 1.0f;

    bDeathFired = false;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USurvivalComponent::TickComponent(float DeltaTime,
                                        ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive()) return;

    // ── Hunger drain ─────────────────────────────────────────────────────────
    float hungerDrain = HungerDrainPerSecond * DeltaTime;
    Hunger = ClampStat(Hunger - hungerDrain);

    // ── Thirst drain (biome-multiplied) ──────────────────────────────────────
    float thirstDrain = ThirstDrainPerSecond * EnvThirstMult * DeltaTime;
    Thirst = ClampStat(Thirst - thirstDrain);

    // ── Fear decay when below max (passive calm) ──────────────────────────────
    if (Fear > 0.0f)
    {
        float fearDecay = FearDecayPerSecond * DeltaTime;
        Fear = ClampStat(Fear - fearDecay);
    }

    // ── Starvation damage ─────────────────────────────────────────────────────
    if (Hunger <= 0.0f)
    {
        float starvDmg = StarvationDamagePerSecond * DeltaTime;
        ModifyHealth(-starvDmg);
    }

    // ── Dehydration damage ────────────────────────────────────────────────────
    if (Thirst <= 0.0f)
    {
        float dehydDmg = DehydrationDamagePerSecond * DeltaTime;
        ModifyHealth(-dehydDmg);
    }
}

// ── Stat modifiers ────────────────────────────────────────────────────────────

void USurvivalComponent::ModifyHealth(float Delta)
{
    if (!IsAlive() && Delta < 0.0f) return; // Already dead, ignore further damage

    float OldHealth = Health;
    Health = ClampStat(Health + Delta);

    if (Health != OldHealth)
    {
        OnHealthChanged.Broadcast(Health);
    }

    // Death check
    if (Health <= 0.0f && !bDeathFired)
    {
        bDeathFired = true;
        OnDeath.Broadcast();
    }
}

void USurvivalComponent::ModifyHunger(float Delta)
{
    Hunger = ClampStat(Hunger + Delta);
}

void USurvivalComponent::ModifyThirst(float Delta)
{
    Thirst = ClampStat(Thirst + Delta);
}

void USurvivalComponent::ModifyStamina(float Delta)
{
    Stamina = ClampStat(Stamina + Delta);
}

void USurvivalComponent::ModifyFear(float Delta)
{
    Fear = ClampStat(Fear + Delta);
}

// ── Environment multipliers ───────────────────────────────────────────────────

void USurvivalComponent::SetEnvironmentMultipliers(float ThirstMult, float StaminaMult, float FearMult)
{
    // Clamp multipliers to sane range [0.1, 5.0] — prevents runaway drain
    EnvThirstMult  = FMath::Clamp(ThirstMult,  0.1f, 5.0f);
    EnvStaminaMult = FMath::Clamp(StaminaMult, 0.1f, 5.0f);
    EnvFearMult    = FMath::Clamp(FearMult,    0.1f, 5.0f);
}

// ── Eat / Drink ───────────────────────────────────────────────────────────────

void USurvivalComponent::Eat(float NutritionValue)
{
    ModifyHunger(NutritionValue);
    // Eating also slightly restores health if starving
    if (Hunger < 20.0f)
    {
        ModifyHealth(NutritionValue * 0.1f);
    }
}

void USurvivalComponent::Drink(float HydrationValue)
{
    ModifyThirst(HydrationValue);
    // Drinking also slightly restores health if dehydrated
    if (Thirst < 20.0f)
    {
        ModifyHealth(HydrationValue * 0.15f);
    }
}

// ── Private helpers ───────────────────────────────────────────────────────────

float USurvivalComponent::ClampStat(float Value) const
{
    return FMath::Clamp(Value, MinStat, MaxStat);
}
