#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // tick at 10 Hz — sufficient for survival drain
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Reset all stats to full on spawn
    Health      = 100.f;
    Hunger      = 100.f;
    Thirst      = 100.f;
    Stamina     = 100.f;
    Temperature = 37.f;
    Fear        = 0.f;
    bSprinting  = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive())
    {
        return;
    }

    TickHungerThirst(DeltaTime);
    TickStamina(DeltaTime);
    TickHealthDrain(DeltaTime);
    TickFear(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
// Private tick helpers
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::TickHungerThirst(float DeltaTime)
{
    Hunger = ClampStat(Hunger - HungerDrainPerSecond * DeltaTime);
    Thirst = ClampStat(Thirst - ThirstDrainPerSecond * DeltaTime);
}

void USurvivalComponent::TickStamina(float DeltaTime)
{
    if (bSprinting)
    {
        Stamina = ClampStat(Stamina - StaminaDrainSprintPerSecond * DeltaTime);

        // Force stop sprint if stamina depleted
        if (Stamina <= 0.f)
        {
            bSprinting = false;
        }
    }
    else
    {
        Stamina = ClampStat(Stamina + StaminaRegenPerSecond * DeltaTime);
    }
}

void USurvivalComponent::TickHealthDrain(float DeltaTime)
{
    // Starvation: drain health when hunger is critical (< 10)
    if (Hunger < 10.f)
    {
        Health = ClampStat(Health - HealthDrainStarvationPerSecond * DeltaTime);
    }

    // Dehydration: drain health faster when thirst is critical (< 10)
    if (Thirst < 10.f)
    {
        Health = ClampStat(Health - HealthDrainStarvationPerSecond * 1.5f * DeltaTime);
    }
}

void USurvivalComponent::TickFear(float DeltaTime)
{
    if (Fear > 0.f)
    {
        Fear = ClampStat(Fear - FearDecayPerSecond * DeltaTime);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Accessors
// ─────────────────────────────────────────────────────────────────────────────

FCore_SurvivalStats USurvivalComponent::GetSurvivalStats() const
{
    FCore_SurvivalStats Stats;
    Stats.Health      = Health;
    Stats.Hunger      = Hunger;
    Stats.Thirst      = Thirst;
    Stats.Stamina     = Stamina;
    Stats.Temperature = Temperature;
    Stats.Fear        = Fear;
    return Stats;
}

ECore_SurvivalStatLevel USurvivalComponent::GetStatLevel(float Value)
{
    if (Value < 10.f)  return ECore_SurvivalStatLevel::Critical;
    if (Value < 30.f)  return ECore_SurvivalStatLevel::Low;
    if (Value < 70.f)  return ECore_SurvivalStatLevel::Normal;
    if (Value < 90.f)  return ECore_SurvivalStatLevel::High;
    return ECore_SurvivalStatLevel::Full;
}

// ─────────────────────────────────────────────────────────────────────────────
// Modifiers
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::ConsumeFood(float Amount)
{
    if (Amount <= 0.f) return;
    Hunger = ClampStat(Hunger + Amount);
}

void USurvivalComponent::DrinkWater(float Amount)
{
    if (Amount <= 0.f) return;
    Thirst = ClampStat(Thirst + Amount);
}

void USurvivalComponent::ApplyHealthDelta(float Delta)
{
    Health = ClampStat(Health - Delta); // positive Delta = damage
}

void USurvivalComponent::AddFear(float Amount)
{
    if (Amount <= 0.f) return;
    Fear = ClampStat(Fear + Amount);
}

void USurvivalComponent::SetSprinting(bool bIsSprinting)
{
    // Cannot sprint with no stamina
    if (bIsSprinting && Stamina <= 0.f)
    {
        bSprinting = false;
        return;
    }
    bSprinting = bIsSprinting;
}

bool USurvivalComponent::IsAlive() const
{
    return Health > 0.f;
}
