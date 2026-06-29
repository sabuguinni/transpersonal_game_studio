#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — sufficient for survival stats
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Reset all stats to full on spawn
    Health          = 100.f;
    Hunger          = 100.f;
    Thirst          = 100.f;
    Stamina         = 100.f;
    BodyTemperature = 37.f;
    Fear            = 0.f;
    bIsAlive        = true;

    bHungerCriticalFired  = false;
    bThirstCriticalFired  = false;
    bHealthCriticalFired  = false;
    bStaminaCriticalFired = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAlive)
    {
        return;
    }

    TickHunger(DeltaTime);
    TickThirst(DeltaTime);
    TickStamina(DeltaTime);
    TickFear(DeltaTime);
    TickTemperature(DeltaTime);
    CheckCriticalThresholds();
}

// ─────────────────────────────────────────────────────────────────────────────
// Stat Accessors
// ─────────────────────────────────────────────────────────────────────────────

FCore_SurvivalSnapshot USurvivalComponent::GetSnapshot() const
{
    FCore_SurvivalSnapshot Snap;
    Snap.Health      = Health;
    Snap.Hunger      = Hunger;
    Snap.Thirst      = Thirst;
    Snap.Stamina     = Stamina;
    Snap.Temperature = BodyTemperature;
    Snap.Fear        = Fear;
    Snap.bIsAlive    = bIsAlive;
    return Snap;
}

float USurvivalComponent::GetStatNormalised(ECore_SurvivalStat Stat) const
{
    switch (Stat)
    {
        case ECore_SurvivalStat::Health:      return Health      / 100.f;
        case ECore_SurvivalStat::Hunger:      return Hunger      / 100.f;
        case ECore_SurvivalStat::Thirst:      return Thirst      / 100.f;
        case ECore_SurvivalStat::Stamina:     return Stamina     / 100.f;
        case ECore_SurvivalStat::Temperature: return FMath::Clamp((BodyTemperature - 30.f) / 15.f, 0.f, 1.f);
        case ECore_SurvivalStat::Fear:        return Fear        / 100.f;
        default:                              return 0.f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Stat Modifiers
// ─────────────────────────────────────────────────────────────────────────────

float USurvivalComponent::ApplyDamage(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return 0.f;
    }

    const float Before = Health;
    Health = ClampStat(Health - Amount);
    const float Applied = Before - Health;

    OnStatChanged.Broadcast(ECore_SurvivalStat::Health);

    if (Health <= 0.f)
    {
        TriggerDeath();
    }

    return Applied;
}

float USurvivalComponent::Heal(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return 0.f;
    }

    const float Before = Health;
    Health = ClampStat(Health + Amount);
    const float Applied = Health - Before;

    if (Applied > 0.f)
    {
        OnStatChanged.Broadcast(ECore_SurvivalStat::Health);
        bHealthCriticalFired = false; // Reset critical flag on recovery
    }

    return Applied;
}

float USurvivalComponent::Eat(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return 0.f;
    }

    const float Before = Hunger;
    Hunger = ClampStat(Hunger + Amount);
    const float Applied = Hunger - Before;

    if (Applied > 0.f)
    {
        OnStatChanged.Broadcast(ECore_SurvivalStat::Hunger);
        bHungerCriticalFired = false;
    }

    return Applied;
}

float USurvivalComponent::Drink(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return 0.f;
    }

    const float Before = Thirst;
    Thirst = ClampStat(Thirst + Amount);
    const float Applied = Thirst - Before;

    if (Applied > 0.f)
    {
        OnStatChanged.Broadcast(ECore_SurvivalStat::Thirst);
        bThirstCriticalFired = false;
    }

    return Applied;
}

float USurvivalComponent::SpendStamina(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return 0.f;
    }

    const float Before = Stamina;
    Stamina = ClampStat(Stamina - Amount);
    const float Applied = Before - Stamina;

    if (Applied > 0.f)
    {
        OnStatChanged.Broadcast(ECore_SurvivalStat::Stamina);
    }

    return Applied;
}

void USurvivalComponent::AddFear(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return;
    }

    Fear = ClampStat(Fear + Amount);
    OnStatChanged.Broadcast(ECore_SurvivalStat::Fear);
}

void USurvivalComponent::ReduceFear(float Amount)
{
    if (!bIsAlive || Amount <= 0.f)
    {
        return;
    }

    Fear = ClampStat(Fear - Amount);
    OnStatChanged.Broadcast(ECore_SurvivalStat::Fear);
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal Tick Helpers
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::TickHunger(float DeltaTime)
{
    // Biome heat increases hunger decay (hot environments burn more calories)
    const float BiomeHeatBonus = FMath::Max(0.f, BiomeTemperatureModifier * 0.01f);
    const float EffectiveDecay = HungerDecayRate * (1.f + BiomeHeatBonus);

    Hunger = ClampStat(Hunger - EffectiveDecay * DeltaTime);

    // Starvation damage when hunger is empty
    if (Hunger <= 0.f)
    {
        ApplyDamage(StarvationDamageRate * DeltaTime);
    }
}

void USurvivalComponent::TickThirst(float DeltaTime)
{
    // Biome humidity reduces thirst decay (humid air = less water loss)
    const float HumidityReduction = FMath::Clamp(BiomeHumidityModifier * 0.005f, 0.f, 0.5f);
    const float EffectiveDecay    = ThirstDecayRate * (1.f - HumidityReduction);

    Thirst = ClampStat(Thirst - EffectiveDecay * DeltaTime);

    // Dehydration damage when thirst is empty
    if (Thirst <= 0.f)
    {
        ApplyDamage(DehydrationDamageRate * DeltaTime);
    }
}

void USurvivalComponent::TickStamina(float DeltaTime)
{
    // Stamina regenerates passively when not being spent externally
    // Hunger/thirst penalties reduce regen rate
    const float HungerPenalty  = (Hunger  < 25.f) ? 0.5f : 1.f;
    const float ThirstPenalty  = (Thirst  < 25.f) ? 0.5f : 1.f;
    const float EffectiveRegen = StaminaRegenRate * HungerPenalty * ThirstPenalty;

    Stamina = ClampStat(Stamina + EffectiveRegen * DeltaTime);
}

void USurvivalComponent::TickFear(float DeltaTime)
{
    // Fear decays naturally when no external AddFear calls are made
    if (Fear > 0.f)
    {
        Fear = ClampStat(Fear - FearDecayRate * DeltaTime);
        if (Fear < 0.f)
        {
            Fear = 0.f;
        }
    }
}

void USurvivalComponent::TickTemperature(float DeltaTime)
{
    // Body temperature drifts toward ambient biome temperature
    // Normal body temp = 37°C. Biome modifier shifts the ambient.
    const float AmbientTemp  = 25.f + BiomeTemperatureModifier; // base ambient 25°C
    const float TempDelta    = AmbientTemp - BodyTemperature;
    const float TempRate     = 0.1f; // degrees per second drift

    BodyTemperature += TempDelta * TempRate * DeltaTime;

    // Hypothermia/hyperthermia damage
    if (BodyTemperature < 35.f)
    {
        // Hypothermia — damage scales with severity
        const float Severity = (35.f - BodyTemperature) * 0.5f;
        ApplyDamage(Severity * DeltaTime);
    }
    else if (BodyTemperature > 40.f)
    {
        // Hyperthermia
        const float Severity = (BodyTemperature - 40.f) * 0.5f;
        ApplyDamage(Severity * DeltaTime);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Critical Threshold Checks
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::CheckCriticalThresholds()
{
    constexpr float CriticalThreshold = 20.f;

    if (!bHungerCriticalFired && Hunger < CriticalThreshold)
    {
        bHungerCriticalFired = true;
        OnLowStat.Broadcast(ECore_SurvivalStat::Hunger);
    }

    if (!bThirstCriticalFired && Thirst < CriticalThreshold)
    {
        bThirstCriticalFired = true;
        OnLowStat.Broadcast(ECore_SurvivalStat::Thirst);
    }

    if (!bHealthCriticalFired && Health < CriticalThreshold)
    {
        bHealthCriticalFired = true;
        OnLowStat.Broadcast(ECore_SurvivalStat::Health);
    }

    if (!bStaminaCriticalFired && Stamina < CriticalThreshold)
    {
        bStaminaCriticalFired = true;
        OnLowStat.Broadcast(ECore_SurvivalStat::Stamina);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Death
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::TriggerDeath()
{
    if (!bIsAlive)
    {
        return;
    }

    bIsAlive = false;
    Health   = 0.f;

    OnDeath.Broadcast();

    // Disable tick — dead characters don't need survival updates
    SetComponentTickEnabled(false);

    UE_LOG(LogTemp, Warning, TEXT("USurvivalComponent: Owner '%s' has died."),
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Utility
// ─────────────────────────────────────────────────────────────────────────────

float USurvivalComponent::ClampStat(float Value, float Max)
{
    return FMath::Clamp(Value, 0.f, Max);
}
