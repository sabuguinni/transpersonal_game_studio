// SurvivalComponent.cpp — Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260702_005
// Implements hunger, thirst, temperature, stamina, and fear survival stats.
// Ticks every second; integrates with BiomeManager for environmental modifiers.
// Realistic prehistoric survival — no spiritual/mystical content.

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // Uses timer instead of per-frame tick

    // Default survival stats — full at spawn
    Health       = 100.0f;
    MaxHealth    = 100.0f;
    Hunger       = 100.0f;
    MaxHunger    = 100.0f;
    Thirst       = 100.0f;
    MaxThirst    = 100.0f;
    Stamina      = 100.0f;
    MaxStamina   = 100.0f;
    Fear         = 0.0f;
    MaxFear      = 100.0f;
    Temperature  = 37.0f; // Normal body temperature (Celsius)

    // Drain rates per second (realistic prehistoric survival pacing)
    HungerDrainRate    = 0.05f;  // ~33 min to starve at rest
    ThirstDrainRate    = 0.08f;  // ~21 min to dehydrate at rest
    StaminaDrainRate   = 1.0f;   // Drains fast during exertion (managed externally)
    StaminaRegenRate   = 0.5f;   // Regens when resting
    FearDecayRate      = 0.2f;   // Fear fades over time when safe

    // Damage thresholds
    StarvationDamageRate    = 1.0f;   // HP/s when Hunger <= 0
    DehydrationDamageRate   = 2.0f;   // HP/s when Thirst <= 0
    HypothermiaDamageRate   = 1.5f;   // HP/s when Temperature < 30
    HyperthermiaThreshold   = 42.0f;  // Celsius — above this causes damage

    // State flags
    bIsExerting    = false;
    bIsResting     = false;
    bIsSprinting   = false;
    bIsStarving    = false;
    bIsDehydrated  = false;

    // Biome modifier defaults (overridden by BiomeManager queries)
    CurrentBiomeTemperature  = 25.0f;
    CurrentBiomeDanger       = 0.0f;
    CurrentBiomeHumidity     = 0.5f;

    TickIntervalSeconds = 1.0f;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start survival tick timer — 1 second intervals
    GetWorld()->GetTimerManager().SetTimer(
        SurvivalTickHandle,
        this,
        &USurvivalComponent::SurvivalTick,
        TickIntervalSeconds,
        true  // looping
    );

    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: BeginPlay — survival tick started (interval=%.1fs)"), TickIntervalSeconds);
}

void USurvivalComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(SurvivalTickHandle);
    Super::EndPlay(EndPlayReason);
}

// ─── Core Survival Tick ───────────────────────────────────────────────────────

void USurvivalComponent::SurvivalTick()
{
    const float DeltaTime = TickIntervalSeconds;

    // --- Hunger drain ---
    float HungerDrain = HungerDrainRate * DeltaTime;
    if (bIsExerting) HungerDrain *= 2.0f;
    if (bIsSprinting) HungerDrain *= 3.0f;
    Hunger = FMath::Max(0.0f, Hunger - HungerDrain);
    bIsStarving = (Hunger <= 0.0f);

    // --- Thirst drain ---
    // Higher temperature and humidity increase thirst drain
    float ThirstMultiplier = 1.0f + FMath::Clamp((CurrentBiomeTemperature - 25.0f) / 20.0f, 0.0f, 1.5f);
    float ThirstDrain = ThirstDrainRate * DeltaTime * ThirstMultiplier;
    if (bIsExerting) ThirstDrain *= 1.5f;
    Thirst = FMath::Max(0.0f, Thirst - ThirstDrain);
    bIsDehydrated = (Thirst <= 0.0f);

    // --- Stamina ---
    if (bIsExerting || bIsSprinting)
    {
        float StaminaDrain = StaminaDrainRate * DeltaTime;
        if (bIsSprinting) StaminaDrain *= 2.0f;
        Stamina = FMath::Max(0.0f, Stamina - StaminaDrain);
    }
    else if (bIsResting)
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenRate * DeltaTime * 2.0f);
    }
    else
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenRate * DeltaTime);
    }

    // --- Temperature regulation ---
    // Body temperature drifts toward biome temperature
    float TempDiff = CurrentBiomeTemperature - Temperature;
    Temperature += TempDiff * 0.01f * DeltaTime;  // Slow drift
    Temperature = FMath::Clamp(Temperature, 20.0f, 45.0f);

    // --- Fear decay ---
    if (CurrentBiomeDanger <= 0.0f)
    {
        Fear = FMath::Max(0.0f, Fear - FearDecayRate * DeltaTime);
    }

    // --- Health damage from critical states ---
    float HealthDamage = 0.0f;

    if (bIsStarving)
    {
        HealthDamage += StarvationDamageRate * DeltaTime;
    }
    if (bIsDehydrated)
    {
        HealthDamage += DehydrationDamageRate * DeltaTime;
    }
    if (Temperature < 30.0f)
    {
        float HypothermiaFactor = (30.0f - Temperature) / 10.0f;
        HealthDamage += HypothermiaDamageRate * HypothermiaFactor * DeltaTime;
    }
    if (Temperature > HyperthermiaThreshold)
    {
        float HyperthermiaFactor = (Temperature - HyperthermiaThreshold) / 5.0f;
        HealthDamage += HypothermiaDamageRate * HyperthermiaFactor * DeltaTime;
    }

    if (HealthDamage > 0.0f)
    {
        ApplyHealthDamage(HealthDamage);
    }

    // --- Broadcast stat update ---
    OnSurvivalStatsUpdated.Broadcast(Health, Hunger, Thirst, Stamina, Fear, Temperature);
}

// ─── Public API ──────────────────────────────────────────────────────────────

void USurvivalComponent::ApplyHealthDamage(float Amount)
{
    if (Amount <= 0.0f) return;
    Health = FMath::Max(0.0f, Health - Amount);

    if (Health <= 0.0f)
    {
        OnPlayerDied.Broadcast();
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent: Player has died (Health=0)"));
    }
}

void USurvivalComponent::HealHealth(float Amount)
{
    if (Amount <= 0.0f) return;
    Health = FMath::Min(MaxHealth, Health + Amount);
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Healed %.1f HP — Health=%.1f"), Amount, Health);
}

void USurvivalComponent::ConsumeFood(float NutritionValue)
{
    if (NutritionValue <= 0.0f) return;
    Hunger = FMath::Min(MaxHunger, Hunger + NutritionValue);
    bIsStarving = false;
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Consumed food +%.1f — Hunger=%.1f"), NutritionValue, Hunger);
}

void USurvivalComponent::ConsumeWater(float HydrationValue)
{
    if (HydrationValue <= 0.0f) return;
    Thirst = FMath::Min(MaxThirst, Thirst + HydrationValue);
    bIsDehydrated = false;
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Consumed water +%.1f — Thirst=%.1f"), HydrationValue, Thirst);
}

void USurvivalComponent::AddFear(float FearAmount)
{
    if (FearAmount <= 0.0f) return;
    Fear = FMath::Min(MaxFear, Fear + FearAmount);
}

void USurvivalComponent::ReduceFear(float FearAmount)
{
    if (FearAmount <= 0.0f) return;
    Fear = FMath::Max(0.0f, Fear - FearAmount);
}

void USurvivalComponent::SetExerting(bool bExerting)
{
    bIsExerting = bExerting;
}

void USurvivalComponent::SetSprinting(bool bSprinting)
{
    bIsSprinting = bSprinting;
    bIsExerting = bSprinting; // Sprinting implies exertion
}

void USurvivalComponent::SetResting(bool bResting)
{
    bIsResting = bResting;
    if (bResting)
    {
        bIsExerting = false;
        bIsSprinting = false;
    }
}

void USurvivalComponent::UpdateBiomeConditions(float BiomeTemperature, float BiomeDanger, float BiomeHumidity)
{
    CurrentBiomeTemperature = BiomeTemperature;
    CurrentBiomeDanger      = BiomeDanger;
    CurrentBiomeHumidity    = BiomeHumidity;
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Biome updated — Temp=%.1f, Danger=%.2f, Humidity=%.2f"),
        BiomeTemperature, BiomeDanger, BiomeHumidity);
}

float USurvivalComponent::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
}

float USurvivalComponent::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (Hunger / MaxHunger) : 0.0f;
}

float USurvivalComponent::GetThirstPercent() const
{
    return (MaxThirst > 0.0f) ? (Thirst / MaxThirst) : 0.0f;
}

float USurvivalComponent::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f;
}

float USurvivalComponent::GetFearPercent() const
{
    return (MaxFear > 0.0f) ? (Fear / MaxFear) : 0.0f;
}

bool USurvivalComponent::IsInCriticalState() const
{
    return (Health < 20.0f || bIsStarving || bIsDehydrated || Temperature < 32.0f || Temperature > 41.0f);
}
