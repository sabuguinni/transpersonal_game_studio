#include "SurvivalComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for survival stats

    // Health: no passive drain; damaged by starvation/dehydration/combat
    HealthStat.Current    = 100.0f;
    HealthStat.Max        = 100.0f;
    HealthStat.DrainRatePerSecond = 0.0f;
    HealthStat.CriticalThreshold  = 25.0f;

    // Hunger: drains slowly; empty = starvation damage
    HungerStat.Current    = 100.0f;
    HungerStat.Max        = 100.0f;
    HungerStat.DrainRatePerSecond = 0.3f;
    HungerStat.CriticalThreshold  = 20.0f;

    // Thirst: drains faster than hunger; empty = dehydration damage
    ThirstStat.Current    = 100.0f;
    ThirstStat.Max        = 100.0f;
    ThirstStat.DrainRatePerSecond = 0.5f;
    ThirstStat.CriticalThreshold  = 20.0f;

    // Stamina: drains on sprint/combat; recovers at rest
    StaminaStat.Current   = 100.0f;
    StaminaStat.Max       = 100.0f;
    StaminaStat.DrainRatePerSecond = 0.0f; // managed manually via sprint state
    StaminaStat.CriticalThreshold  = 10.0f;

    // Temperature: neutral = 50; extreme cold/heat causes health drain
    TemperatureStat.Current   = 50.0f;
    TemperatureStat.Min       = 0.0f;
    TemperatureStat.Max       = 100.0f;
    TemperatureStat.DrainRatePerSecond = 0.0f;
    TemperatureStat.CriticalThreshold  = 15.0f; // hypothermia threshold

    // Fear: spikes near predators; decays passively
    FearStat.Current      = 0.0f;
    FearStat.Max          = 100.0f;
    FearStat.DrainRatePerSecond = 0.0f;
    FearStat.CriticalThreshold  = 75.0f; // panic threshold
}

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    // Stats are initialised in constructor — nothing extra needed at BeginPlay
    // Future: load saved stats from SaveGame here
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (IsDead()) return;

    TickPassiveDrain(DeltaTime);
    TickStaminaRecovery(DeltaTime);
    TickFearDecay(DeltaTime);
    TickStarvationAndDehydration(DeltaTime);
    TickTemperatureEffect(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
// Stat Access
// ─────────────────────────────────────────────────────────────────────────────

float USurvivalComponent::GetStatNormalized(ECore_SurvivalStat Stat) const
{
    return GetStatDataConst(Stat).NormalizedValue();
}

float USurvivalComponent::GetStatCurrent(ECore_SurvivalStat Stat) const
{
    return GetStatDataConst(Stat).Current;
}

bool USurvivalComponent::IsStatCritical(ECore_SurvivalStat Stat) const
{
    return GetStatDataConst(Stat).IsCritical();
}

// ─────────────────────────────────────────────────────────────────────────────
// Stat Modification
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::ModifyStat(ECore_SurvivalStat Stat, float Delta)
{
    FCore_SurvivalStatData& Data = GetStatData(Stat);
    Data.Current = FMath::Clamp(Data.Current + Delta, Data.Min, Data.Max);
}

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (Amount <= 0.0f) return;
    HealthStat.Current = FMath::Max(HealthStat.Min, HealthStat.Current - Amount);
}

void USurvivalComponent::ConsumeFood(float NutritionValue)
{
    if (NutritionValue <= 0.0f) return;
    HungerStat.Current = FMath::Min(HungerStat.Max, HungerStat.Current + NutritionValue);
    // Eating dry food costs a little thirst
    ThirstStat.Current = FMath::Max(ThirstStat.Min, ThirstStat.Current - (NutritionValue * 0.1f));
}

void USurvivalComponent::DrinkWater(float HydrationValue)
{
    if (HydrationValue <= 0.0f) return;
    ThirstStat.Current = FMath::Min(ThirstStat.Max, ThirstStat.Current + HydrationValue);
}

void USurvivalComponent::SetAmbientTemperatureModifier(float Modifier)
{
    AmbientTemperatureModifier = FMath::Clamp(Modifier, -1.0f, 1.0f);
}

void USurvivalComponent::AddFear(float Amount)
{
    if (Amount <= 0.0f) return;
    FearStat.Current = FMath::Min(FearStat.Max, FearStat.Current + Amount);
}

void USurvivalComponent::ReduceFear(float Amount)
{
    if (Amount <= 0.0f) return;
    FearStat.Current = FMath::Max(FearStat.Min, FearStat.Current - Amount);
}

// ─────────────────────────────────────────────────────────────────────────────
// State Queries
// ─────────────────────────────────────────────────────────────────────────────

bool USurvivalComponent::IsDead() const
{
    return HealthStat.Current <= HealthStat.Min;
}

bool USurvivalComponent::IsExhausted() const
{
    return StaminaStat.IsCritical();
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal Tick Helpers
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::TickPassiveDrain(float DeltaTime)
{
    // Hunger drains passively
    HungerStat.Current = FMath::Max(HungerStat.Min,
        HungerStat.Current - HungerDrainRate * DeltaTime);

    // Thirst drains passively (faster than hunger)
    ThirstStat.Current = FMath::Max(ThirstStat.Min,
        ThirstStat.Current - ThirstDrainRate * DeltaTime);
}

void USurvivalComponent::TickStaminaRecovery(float DeltaTime)
{
    // Stamina recovers when not sprinting
    // Sprint state is set externally by the character movement system
    if (!bIsSprinting && StaminaStat.Current < StaminaStat.Max)
    {
        StaminaStat.Current = FMath::Min(StaminaStat.Max,
            StaminaStat.Current + StaminaRecoveryRate * DeltaTime);
    }
}

void USurvivalComponent::TickFearDecay(float DeltaTime)
{
    if (FearStat.Current > FearStat.Min)
    {
        FearStat.Current = FMath::Max(FearStat.Min,
            FearStat.Current - FearDecayRate * DeltaTime);
    }
}

void USurvivalComponent::TickStarvationAndDehydration(float DeltaTime)
{
    // Starvation: health drain when hunger is empty
    if (HungerStat.IsEmpty())
    {
        HealthStat.Current = FMath::Max(HealthStat.Min,
            HealthStat.Current - StarvationDamageRate * DeltaTime);
    }

    // Dehydration: health drain when thirst is empty (faster than starvation)
    if (ThirstStat.IsEmpty())
    {
        HealthStat.Current = FMath::Max(HealthStat.Min,
            HealthStat.Current - DehydrationDamageRate * DeltaTime);
    }
}

void USurvivalComponent::TickTemperatureEffect(float DeltaTime)
{
    // Ambient temperature modifier pushes temperature stat toward extremes
    if (FMath::Abs(AmbientTemperatureModifier) > 0.01f)
    {
        const float TempDelta = AmbientTemperatureModifier * 2.0f * DeltaTime;
        TemperatureStat.Current = FMath::Clamp(
            TemperatureStat.Current + TempDelta,
            TemperatureStat.Min, TemperatureStat.Max);
    }
    else
    {
        // Passive return toward neutral (50)
        const float Neutral = 50.0f;
        const float RecoveryRate = 1.0f;
        if (TemperatureStat.Current < Neutral)
            TemperatureStat.Current = FMath::Min(Neutral, TemperatureStat.Current + RecoveryRate * DeltaTime);
        else if (TemperatureStat.Current > Neutral)
            TemperatureStat.Current = FMath::Max(Neutral, TemperatureStat.Current - RecoveryRate * DeltaTime);
    }

    // Hypothermia / hyperthermia: health drain at extremes
    if (TemperatureStat.Current <= TemperatureStat.CriticalThreshold ||
        TemperatureStat.Current >= (TemperatureStat.Max - TemperatureStat.CriticalThreshold))
    {
        HealthStat.Current = FMath::Max(HealthStat.Min,
            HealthStat.Current - 0.5f * DeltaTime);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal Helpers — stat data access by enum
// ─────────────────────────────────────────────────────────────────────────────

FCore_SurvivalStatData& USurvivalComponent::GetStatData(ECore_SurvivalStat Stat)
{
    switch (Stat)
    {
        case ECore_SurvivalStat::Health:      return HealthStat;
        case ECore_SurvivalStat::Hunger:      return HungerStat;
        case ECore_SurvivalStat::Thirst:      return ThirstStat;
        case ECore_SurvivalStat::Stamina:     return StaminaStat;
        case ECore_SurvivalStat::Temperature: return TemperatureStat;
        case ECore_SurvivalStat::Fear:        return FearStat;
        default:                              return HealthStat;
    }
}

const FCore_SurvivalStatData& USurvivalComponent::GetStatDataConst(ECore_SurvivalStat Stat) const
{
    switch (Stat)
    {
        case ECore_SurvivalStat::Health:      return HealthStat;
        case ECore_SurvivalStat::Hunger:      return HungerStat;
        case ECore_SurvivalStat::Thirst:      return ThirstStat;
        case ECore_SurvivalStat::Stamina:     return StaminaStat;
        case ECore_SurvivalStat::Temperature: return TemperatureStat;
        case ECore_SurvivalStat::Fear:        return FearStat;
        default:                              return HealthStat;
    }
}
