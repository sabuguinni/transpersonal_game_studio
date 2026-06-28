// SurvivalComponent.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Prehistoric survival stats: health, hunger, thirst, stamina, temperature, fear.
// Ticks every frame, applies drain/damage logic, fires delegates on state change.

#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // tick every frame, we gate internally
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    // Reset to full stats on spawn
    Health      = 100.0f;
    Hunger      = 100.0f;
    Thirst      = 100.0f;
    Stamina     = 100.0f;
    Temperature = 20.0f;
    Fear        = 0.0f;
    bIsDead     = false;
    AccumulatedTime = 0.0f;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead)
    {
        return;
    }

    TickSurvivalStats(DeltaTime);
}

// ── Stat Modifiers ──────────────────────────────────────────────────────────

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (bIsDead || Amount <= 0.0f)
    {
        return;
    }

    Health -= Amount;
    ClampStat(Health);
    OnHealthChanged.Broadcast(Health);

    if (Health <= 0.0f)
    {
        HandleDeath();
    }
}

void USurvivalComponent::Eat(float NutritionValue)
{
    if (bIsDead || NutritionValue <= 0.0f)
    {
        return;
    }

    Hunger += NutritionValue;
    ClampStat(Hunger);
    OnHungerChanged.Broadcast(Hunger);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    if (bIsDead || HydrationValue <= 0.0f)
    {
        return;
    }

    Thirst += HydrationValue;
    ClampStat(Thirst);
    OnThirstChanged.Broadcast(Thirst);
}

void USurvivalComponent::ConsumeStamina(float Amount)
{
    if (bIsDead || Amount <= 0.0f)
    {
        return;
    }

    Stamina -= Amount;
    ClampStat(Stamina);
    OnStaminaChanged.Broadcast(Stamina);
}

void USurvivalComponent::RecoverStamina(float Amount)
{
    if (bIsDead || Amount <= 0.0f)
    {
        return;
    }

    Stamina += Amount;
    ClampStat(Stamina);
    OnStaminaChanged.Broadcast(Stamina);
}

void USurvivalComponent::AddFear(float Amount)
{
    if (bIsDead || Amount <= 0.0f)
    {
        return;
    }

    Fear += Amount;
    ClampStat(Fear);
}

void USurvivalComponent::ReduceFear(float Amount)
{
    if (bIsDead || Amount <= 0.0f)
    {
        return;
    }

    Fear -= Amount;
    ClampStat(Fear);
}

void USurvivalComponent::SetAmbientTemperature(float AmbientTemp)
{
    // Gradually move body temperature toward ambient
    // Called by environment systems (biome manager, weather system)
    Temperature = FMath::FInterpTo(Temperature, AmbientTemp, GetWorld()->GetDeltaSeconds(), 0.1f);
}

// ── Internal Helpers ────────────────────────────────────────────────────────

void USurvivalComponent::TickSurvivalStats(float DeltaTime)
{
    // ── Hunger drain ────────────────────────────────────────────────────────
    const float PrevHunger = Hunger;
    Hunger -= HungerDrainPerSecond * DeltaTime;
    ClampStat(Hunger);
    if (FMath::Abs(Hunger - PrevHunger) > 0.01f)
    {
        OnHungerChanged.Broadcast(Hunger);
    }

    // ── Thirst drain ────────────────────────────────────────────────────────
    const float PrevThirst = Thirst;
    Thirst -= ThirstDrainPerSecond * DeltaTime;
    ClampStat(Thirst);
    if (FMath::Abs(Thirst - PrevThirst) > 0.01f)
    {
        OnThirstChanged.Broadcast(Thirst);
    }

    // ── Stamina passive recovery (when not sprinting) ────────────────────────
    const float PrevStamina = Stamina;
    Stamina += StaminaRecoveryPerSecond * DeltaTime;
    ClampStat(Stamina);
    if (FMath::Abs(Stamina - PrevStamina) > 0.01f)
    {
        OnStaminaChanged.Broadcast(Stamina);
    }

    // ── Fear passive decay ───────────────────────────────────────────────────
    Fear -= FearDecayPerSecond * DeltaTime;
    ClampStat(Fear);

    // ── Starvation damage ────────────────────────────────────────────────────
    if (Hunger <= 0.0f)
    {
        ApplyDamage(StarvationDamagePerSecond * DeltaTime);
    }

    // ── Dehydration damage ───────────────────────────────────────────────────
    if (Thirst <= 0.0f)
    {
        ApplyDamage(DehydrationDamagePerSecond * DeltaTime);
    }

    // ── Temperature damage ───────────────────────────────────────────────────
    if (Temperature >= HyperthermiaThreshold)
    {
        // Hyperthermia — overheating
        const float Severity = (Temperature - HyperthermiaThreshold) / 10.0f;
        ApplyDamage(HypothermiaDamagePerSecond * Severity * DeltaTime);
    }
    else if (Temperature <= HypothermiaThreshold)
    {
        // Hypothermia — freezing
        const float Severity = (HypothermiaThreshold - Temperature) / 10.0f;
        ApplyDamage(HypothermiaDamagePerSecond * Severity * DeltaTime);
    }
}

void USurvivalComponent::ClampStat(float& Stat, float Min, float Max)
{
    Stat = FMath::Clamp(Stat, Min, Max);
}

void USurvivalComponent::HandleDeath()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;
    Health  = 0.0f;
    OnHealthChanged.Broadcast(0.0f);
    OnDeath.Broadcast();

    UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent: Character died. Owner: %s"),
        GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}
