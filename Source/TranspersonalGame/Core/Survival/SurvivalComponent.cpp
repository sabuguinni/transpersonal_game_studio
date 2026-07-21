// SurvivalComponent.cpp
// Core Systems Programmer — Agent #03
// Cycle 007: Survival stat drain wired to BiomeManager temperature/danger modifiers
// Prehistoric survival game — realistic hunger/thirst/stamina/health drain

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // 2 Hz — performance-conscious

    // Default survival stats (full bars)
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
    Temperature  = 37.0f; // Celsius — human body temp baseline

    // Default drain rates (per second, baseline biome = Plains)
    HungerDrainRate  = 0.5f;
    ThirstDrainRate  = 0.8f;
    StaminaDrainRate = 5.0f;
    StaminaRegenRate = 3.0f;

    // Biome modifiers (set by BiomeManager on zone transition)
    BiomeThirstMultiplier  = 1.0f;
    BiomeHealthDrainPerSec = 0.0f;
    BiomeDangerLevel       = 0.3f;
    BiomeTemperature       = 22.0f; // Celsius — Plains default

    bIsExhausted = false;
    bIsCritical  = false;
    AccumulatedTime = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    // Stats start full — no timer needed, using Tick with interval
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent — drain stats at 2 Hz
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AccumulatedTime += DeltaTime;

    // Process every 0.5s (matches TickInterval)
    // Drain is scaled to per-second rates × DeltaTime
    DrainHunger(DeltaTime);
    DrainThirst(DeltaTime);
    ApplyBiomeHealthDrain(DeltaTime);
    UpdateTemperatureEffect(DeltaTime);
    UpdateCriticalState();
}

// ─────────────────────────────────────────────────────────────────────────────
// Drain Hunger
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::DrainHunger(float DeltaTime)
{
    if (Hunger <= 0.0f) return;

    float drain = HungerDrainRate * DeltaTime;

    // Strenuous activity bonus (fear increases hunger drain — fight-or-flight)
    if (Fear > 50.0f)
    {
        drain *= 1.3f;
    }

    Hunger = FMath::Max(0.0f, Hunger - drain);

    // Starvation: health drain when hunger hits zero
    if (Hunger <= 0.0f && Health > 0.0f)
    {
        Health = FMath::Max(0.0f, Health - (0.2f * DeltaTime));
    }

    OnHungerChanged.Broadcast(Hunger, MaxHunger);
}

// ─────────────────────────────────────────────────────────────────────────────
// Drain Thirst — BiomeThirstMultiplier applied here
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::DrainThirst(float DeltaTime)
{
    if (Thirst <= 0.0f) return;

    // Temperature above 30°C accelerates thirst (Volcanic biome)
    float tempFactor = 1.0f;
    if (BiomeTemperature > 30.0f)
    {
        tempFactor = 1.0f + ((BiomeTemperature - 30.0f) * 0.05f); // +5% per degree above 30°C
    }

    float drain = ThirstDrainRate * BiomeThirstMultiplier * tempFactor * DeltaTime;
    Thirst = FMath::Max(0.0f, Thirst - drain);

    // Dehydration: accelerated health drain when thirst hits zero
    if (Thirst <= 0.0f && Health > 0.0f)
    {
        Health = FMath::Max(0.0f, Health - (0.5f * DeltaTime)); // Dehydration kills faster than starvation
    }

    OnThirstChanged.Broadcast(Thirst, MaxThirst);
}

// ─────────────────────────────────────────────────────────────────────────────
// Apply Biome Health Drain (Volcanic heat, toxic gas, etc.)
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::ApplyBiomeHealthDrain(float DeltaTime)
{
    if (BiomeHealthDrainPerSec <= 0.0f) return;

    Health = FMath::Max(0.0f, Health - (BiomeHealthDrainPerSec * DeltaTime));
    OnHealthChanged.Broadcast(Health, MaxHealth);
}

// ─────────────────────────────────────────────────────────────────────────────
// Temperature Effect — hypothermia/hyperthermia
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::UpdateTemperatureEffect(float DeltaTime)
{
    // Body temperature drifts toward biome temperature (slow thermal equilibrium)
    float tempDelta = (BiomeTemperature - Temperature) * 0.01f * DeltaTime;
    Temperature = FMath::Clamp(Temperature + tempDelta, 20.0f, 45.0f);

    // Hypothermia: below 35°C
    if (Temperature < 35.0f)
    {
        float severity = (35.0f - Temperature) / 15.0f; // 0-1 scale
        Health = FMath::Max(0.0f, Health - (severity * 0.3f * DeltaTime));
        OnHealthChanged.Broadcast(Health, MaxHealth);
    }

    // Hyperthermia: above 40°C (Volcanic biome extreme)
    if (Temperature > 40.0f)
    {
        float severity = (Temperature - 40.0f) / 5.0f;
        Health = FMath::Max(0.0f, Health - (severity * 0.5f * DeltaTime));
        // Hyperthermia also accelerates thirst loss
        Thirst = FMath::Max(0.0f, Thirst - (severity * 1.0f * DeltaTime));
        OnHealthChanged.Broadcast(Health, MaxHealth);
        OnThirstChanged.Broadcast(Thirst, MaxThirst);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Update Critical State flags
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::UpdateCriticalState()
{
    bool bWasCritical = bIsCritical;
    bIsCritical = (Health < 25.0f || Hunger < 10.0f || Thirst < 10.0f);
    bIsExhausted = (Stamina < 5.0f);

    if (bIsCritical && !bWasCritical)
    {
        OnCriticalState.Broadcast(true);
    }
    else if (!bIsCritical && bWasCritical)
    {
        OnCriticalState.Broadcast(false);
    }

    // Death check
    if (Health <= 0.0f)
    {
        OnDeath.Broadcast();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API — Consume food/water
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::ConsumeFood(float Amount)
{
    Hunger = FMath::Min(MaxHunger, Hunger + Amount);
    OnHungerChanged.Broadcast(Hunger, MaxHunger);
}

void USurvivalComponent::ConsumeWater(float Amount)
{
    Thirst = FMath::Min(MaxThirst, Thirst + Amount);
    OnThirstChanged.Broadcast(Thirst, MaxThirst);
}

void USurvivalComponent::ApplyDamage(float DamageAmount)
{
    Health = FMath::Max(0.0f, Health - DamageAmount);
    OnHealthChanged.Broadcast(Health, MaxHealth);
    UpdateCriticalState();
}

void USurvivalComponent::HealDamage(float HealAmount)
{
    Health = FMath::Min(MaxHealth, Health + HealAmount);
    OnHealthChanged.Broadcast(Health, MaxHealth);
}

void USurvivalComponent::DrainStamina(float Amount)
{
    Stamina = FMath::Max(0.0f, Stamina - Amount);
    bIsExhausted = (Stamina < 5.0f);
    OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

void USurvivalComponent::RegenStamina(float DeltaTime)
{
    if (bIsExhausted) return; // Exhausted: no regen until threshold recovered
    Stamina = FMath::Min(MaxStamina, Stamina + (StaminaRegenRate * DeltaTime));
    OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

// ─────────────────────────────────────────────────────────────────────────────
// BiomeManager integration — called on biome zone transition
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::SetBiomeModifiers(float ThirstMultiplier, float HealthDrainPerSec, float DangerLevel, float AmbientTemperatureCelsius)
{
    BiomeThirstMultiplier  = FMath::Max(0.1f, ThirstMultiplier);
    BiomeHealthDrainPerSec = FMath::Max(0.0f, HealthDrainPerSec);
    BiomeDangerLevel       = FMath::Clamp(DangerLevel, 0.0f, 1.0f);
    BiomeTemperature       = FMath::Clamp(AmbientTemperatureCelsius, -20.0f, 80.0f);

    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Biome modifiers updated — ThirstMult=%.2f, HealthDrain=%.2f/s, Danger=%.2f, Temp=%.1f°C"),
        BiomeThirstMultiplier, BiomeHealthDrainPerSec, BiomeDangerLevel, BiomeTemperature);
}

// ─────────────────────────────────────────────────────────────────────────────
// Fear system — increases with nearby predator proximity
// ─────────────────────────────────────────────────────────────────────────────
void USurvivalComponent::AddFear(float Amount)
{
    Fear = FMath::Min(MaxFear, Fear + Amount);
    OnFearChanged.Broadcast(Fear, MaxFear);
}

void USurvivalComponent::ReduceFear(float Amount)
{
    Fear = FMath::Max(0.0f, Fear - Amount);
    OnFearChanged.Broadcast(Fear, MaxFear);
}

// ─────────────────────────────────────────────────────────────────────────────
// Getters
// ─────────────────────────────────────────────────────────────────────────────
float USurvivalComponent::GetHealthPercent() const   { return (MaxHealth  > 0.0f) ? Health  / MaxHealth  : 0.0f; }
float USurvivalComponent::GetHungerPercent() const   { return (MaxHunger  > 0.0f) ? Hunger  / MaxHunger  : 0.0f; }
float USurvivalComponent::GetThirstPercent() const   { return (MaxThirst  > 0.0f) ? Thirst  / MaxThirst  : 0.0f; }
float USurvivalComponent::GetStaminaPercent() const  { return (MaxStamina > 0.0f) ? Stamina / MaxStamina : 0.0f; }
float USurvivalComponent::GetFearPercent() const     { return (MaxFear    > 0.0f) ? Fear    / MaxFear    : 0.0f; }
