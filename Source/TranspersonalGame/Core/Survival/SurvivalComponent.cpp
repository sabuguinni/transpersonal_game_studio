// SurvivalComponent.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Full implementation of survival stats: hunger, thirst, stamina, temperature, fear, health

#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick at 10Hz — survival stats don't need per-frame precision
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize stats to full
    Health = 100.0f;
    Hunger = 100.0f;
    Thirst = 100.0f;
    Stamina = 100.0f;
    BodyTemperature = 37.0f;
    Fear = 0.0f;
    bIsAlive = true;
    LastDeathCause = ECore_DeathCause::None;
    AmbientTemperature = 20.0f;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAlive)
    {
        return;
    }

    TickHunger(DeltaTime);
    TickThirst(DeltaTime);
    TickStamina(DeltaTime);
    TickTemperature(DeltaTime);
    TickFear(DeltaTime);
    CheckCriticalStats();
}

// ── Stat Modifiers ───────────────────────────────────────────────────────────

void USurvivalComponent::ApplyHealthDelta(float Delta, ECore_DeathCause Cause)
{
    if (!bIsAlive) return;

    Health = FMath::Clamp(Health - Delta, 0.0f, 100.0f);

    if (Health <= 0.0f)
    {
        TriggerDeath(Cause);
    }
}

void USurvivalComponent::Eat(float Amount)
{
    Hunger = FMath::Clamp(Hunger + Amount, 0.0f, 100.0f);
}

void USurvivalComponent::Drink(float Amount)
{
    Thirst = FMath::Clamp(Thirst + Amount, 0.0f, 100.0f);
}

void USurvivalComponent::ConsumeStamina(float Amount)
{
    Stamina = FMath::Clamp(Stamina - Amount, 0.0f, 100.0f);
}

void USurvivalComponent::RestoreStamina(float Amount)
{
    Stamina = FMath::Clamp(Stamina + Amount, 0.0f, 100.0f);
}

void USurvivalComponent::SetAmbientTemperature(float AmbientCelsius)
{
    AmbientTemperature = AmbientCelsius;
}

void USurvivalComponent::AddFear(float Amount)
{
    Fear = FMath::Clamp(Fear + Amount, 0.0f, 100.0f);
}

void USurvivalComponent::ReduceFear(float Amount)
{
    Fear = FMath::Clamp(Fear - Amount, 0.0f, 100.0f);
}

FCore_SurvivalSnapshot USurvivalComponent::GetSnapshot() const
{
    FCore_SurvivalSnapshot Snap;
    Snap.Health = Health;
    Snap.Hunger = Hunger;
    Snap.Thirst = Thirst;
    Snap.Stamina = Stamina;
    Snap.Temperature = BodyTemperature;
    Snap.Fear = Fear;
    Snap.bIsAlive = bIsAlive;
    Snap.DeathCause = LastDeathCause;
    return Snap;
}

// ── Internal Tick Helpers ────────────────────────────────────────────────────

void USurvivalComponent::TickHunger(float DeltaTime)
{
    // Drain hunger over time — faster in hot biomes (HungerDrainMultiplier set by BiomeManager)
    float Drain = BaseHungerDrainPerSecond * HungerDrainMultiplier * DeltaTime;
    Hunger = FMath::Clamp(Hunger - Drain, 0.0f, 100.0f);

    // Starvation damage when hunger is empty
    if (Hunger <= 0.0f)
    {
        float StarveDamage = StarvationDamagePerSecond * DeltaTime;
        ApplyHealthDelta(StarveDamage, ECore_DeathCause::Starvation);
    }
}

void USurvivalComponent::TickThirst(float DeltaTime)
{
    // Drain thirst over time — faster in hot/volcanic biomes
    float Drain = BaseThirstDrainPerSecond * ThirstDrainMultiplier * DeltaTime;
    Thirst = FMath::Clamp(Thirst - Drain, 0.0f, 100.0f);

    // Dehydration damage when thirst is empty
    if (Thirst <= 0.0f)
    {
        float DehydDamage = DehydrationDamagePerSecond * DeltaTime;
        ApplyHealthDelta(DehydDamage, ECore_DeathCause::Dehydration);
    }
}

void USurvivalComponent::TickStamina(float DeltaTime)
{
    // Restore stamina passively when not sprinting
    // Sprinting stamina drain is handled externally via ConsumeStamina()
    // Here we only do passive restore
    if (Stamina < 100.0f)
    {
        float Restore = StaminaRestorePerSecond * DeltaTime;
        // Slower restore when hungry/thirsty
        if (Hunger < 20.0f) Restore *= 0.5f;
        if (Thirst < 20.0f) Restore *= 0.5f;
        Stamina = FMath::Clamp(Stamina + Restore, 0.0f, 100.0f);
    }
}

void USurvivalComponent::TickTemperature(float DeltaTime)
{
    // Body temperature drifts toward ambient temperature
    // Normal body temp is 37°C — ambient affects how fast it changes
    float TempDiff = AmbientTemperature - BodyTemperature;
    float TempChangeRate = 0.5f * DeltaTime; // Slow drift — body regulates temperature
    BodyTemperature += TempDiff * TempChangeRate;

    // Hypothermia damage
    if (BodyTemperature < HypothermiaDangerTemp)
    {
        float Severity = (HypothermiaDangerTemp - BodyTemperature) / HypothermiaDangerTemp;
        float Damage = Severity * 2.0f * DeltaTime;
        ApplyHealthDelta(Damage, ECore_DeathCause::Hypothermia);
    }

    // Hyperthermia damage
    if (BodyTemperature > HyperthermiaDangerTemp)
    {
        float Severity = (BodyTemperature - HyperthermiaDangerTemp) / 10.0f;
        float Damage = Severity * 3.0f * DeltaTime;
        ApplyHealthDelta(Damage, ECore_DeathCause::Hyperthermia);
    }
}

void USurvivalComponent::TickFear(float DeltaTime)
{
    // Fear decays naturally over time (player calms down)
    if (Fear > 0.0f)
    {
        Fear = FMath::Clamp(Fear - FearDecayPerSecond * DeltaTime, 0.0f, 100.0f);
    }

    // High fear drains stamina faster (adrenaline cost)
    if (Fear > 70.0f)
    {
        float FearStaminaDrain = (Fear - 70.0f) * 0.05f * DeltaTime;
        ConsumeStamina(FearStaminaDrain);
    }
}

void USurvivalComponent::CheckCriticalStats()
{
    // Fire critical events when stats drop below 20%
    const float CriticalThreshold = 20.0f;

    if (Health < CriticalThreshold)   OnStatCritical.Broadcast(ECore_SurvivalStat::Health);
    if (Hunger < CriticalThreshold)   OnStatCritical.Broadcast(ECore_SurvivalStat::Hunger);
    if (Thirst < CriticalThreshold)   OnStatCritical.Broadcast(ECore_SurvivalStat::Thirst);
    if (Stamina < CriticalThreshold)  OnStatCritical.Broadcast(ECore_SurvivalStat::Stamina);
    if (Fear > 80.0f)                 OnStatCritical.Broadcast(ECore_SurvivalStat::Fear);
}

void USurvivalComponent::TriggerDeath(ECore_DeathCause Cause)
{
    if (!bIsAlive) return; // Prevent double-death

    bIsAlive = false;
    LastDeathCause = Cause;
    Health = 0.0f;

    AActor* Owner = GetOwner();
    OnDeath.Broadcast(Owner, Cause);

    UE_LOG(LogTemp, Warning, TEXT("USurvivalComponent: %s died — Cause: %d"),
        Owner ? *Owner->GetName() : TEXT("Unknown"),
        static_cast<int32>(Cause));
}
