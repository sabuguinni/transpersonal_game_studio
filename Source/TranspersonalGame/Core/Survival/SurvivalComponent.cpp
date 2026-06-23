// SurvivalComponent.cpp
// Transpersonal Game Studio — Core Systems Programmer (#03)
// Prehistoric survival stats: health, hunger, thirst, stamina, fear.
// Ticks at 1 Hz (configurable) to drain stats passively.

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // Tick at 1 Hz — stat drain is slow, no need for per-frame updates
    PrimaryComponentTick.TickInterval = 1.0f;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialise all stats to max
    Health  = MaxHealth;
    Hunger  = MaxHunger;
    Thirst  = MaxThirst;
    Stamina = MaxStamina;
    Fear    = 0.0f;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAlive) return;

    // Passive drain per second (TickInterval = 1.0s so DeltaTime ≈ 1.0)
    ModifyHunger (-HungerDrainRate  * DeltaTime);
    ModifyThirst (-ThirstDrainRate  * DeltaTime);
    ModifyStamina( StaminaRegenRate * DeltaTime);  // regen when not sprinting

    // Fear decays toward 0 naturally
    if (Fear > 0.0f)
    {
        Fear = FMath::Max(0.0f, Fear - FearDecayRate * DeltaTime);
    }

    // Starvation / dehydration damage
    if (Hunger <= 0.0f)
    {
        ModifyHealth(-StarvationDamageRate * DeltaTime);
    }
    if (Thirst <= 0.0f)
    {
        ModifyHealth(-DehydrationDamageRate * DeltaTime);
    }
}

// ── Modifiers ──────────────────────────────────────────────────────────────

void USurvivalComponent::ModifyHealth(float Delta)
{
    Health = FMath::Clamp(Health + Delta, 0.0f, MaxHealth);
    if (Health <= 0.0f && bIsAlive)
    {
        bIsAlive = false;
        OnDeath.Broadcast();
    }
}

void USurvivalComponent::ModifyHunger(float Delta)
{
    Hunger = FMath::Clamp(Hunger + Delta, 0.0f, MaxHunger);
}

void USurvivalComponent::ModifyThirst(float Delta)
{
    Thirst = FMath::Clamp(Thirst + Delta, 0.0f, MaxThirst);
}

void USurvivalComponent::ModifyStamina(float Delta)
{
    Stamina = FMath::Clamp(Stamina + Delta, 0.0f, MaxStamina);
}

void USurvivalComponent::ModifyFear(float Delta)
{
    Fear = FMath::Clamp(Fear + Delta, 0.0f, MaxFear);
}

// ── Getters ────────────────────────────────────────────────────────────────

float USurvivalComponent::GetHealth()  const { return Health;  }
float USurvivalComponent::GetHunger()  const { return Hunger;  }
float USurvivalComponent::GetThirst()  const { return Thirst;  }
float USurvivalComponent::GetStamina() const { return Stamina; }
float USurvivalComponent::GetFear()    const { return Fear;    }
bool  USurvivalComponent::IsAlive()    const { return bIsAlive; }

float USurvivalComponent::GetHealthPercent()  const { return MaxHealth  > 0.0f ? Health  / MaxHealth  : 0.0f; }
float USurvivalComponent::GetHungerPercent()  const { return MaxHunger  > 0.0f ? Hunger  / MaxHunger  : 0.0f; }
float USurvivalComponent::GetThirstPercent()  const { return MaxThirst  > 0.0f ? Thirst  / MaxThirst  : 0.0f; }
float USurvivalComponent::GetStaminaPercent() const { return MaxStamina > 0.0f ? Stamina / MaxStamina : 0.0f; }

// ── Convenience ────────────────────────────────────────────────────────────

void USurvivalComponent::Eat(float NutritionValue)
{
    ModifyHunger(NutritionValue);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    ModifyThirst(HydrationValue);
}

void USurvivalComponent::TakeDamage_Survival(float DamageAmount)
{
    ModifyHealth(-FMath::Abs(DamageAmount));
}

void USurvivalComponent::Heal(float HealAmount)
{
    if (bIsAlive)
    {
        ModifyHealth(FMath::Abs(HealAmount));
    }
}

void USurvivalComponent::AddFearFromDinosaur(float FearAmount)
{
    ModifyFear(FearAmount);
}

bool USurvivalComponent::IsStarving()     const { return Hunger < (MaxHunger * 0.15f); }
bool USurvivalComponent::IsDehydrated()   const { return Thirst < (MaxThirst * 0.15f); }
bool USurvivalComponent::IsExhausted()    const { return Stamina < (MaxStamina * 0.10f); }
bool USurvivalComponent::IsPanicking()    const { return Fear > (MaxFear * 0.75f); }
