// SurvivalComponent.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Prehistoric survival stats: health, hunger, thirst, stamina, temperature, fear
// CYCLE: PROD_CYCLE_AUTO_20260626_001

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for performance

    // Default survival stats
    Health          = 100.0f;
    MaxHealth       = 100.0f;
    Hunger          = 100.0f;
    MaxHunger       = 100.0f;
    Thirst          = 100.0f;
    MaxThirst       = 100.0f;
    Stamina         = 100.0f;
    MaxStamina      = 100.0f;
    Temperature     = 37.0f; // Celsius — human core body temp
    Fear            = 0.0f;
    MaxFear         = 100.0f;

    // Drain rates per second
    HungerDrainRate     = 0.05f;  // ~33 min to starve
    ThirstDrainRate     = 0.08f;  // ~21 min to die of thirst
    StaminaDrainRate    = 2.0f;   // Drains fast during sprint
    StaminaRegenRate    = 1.5f;   // Regens when resting
    FearDecayRate       = 0.3f;   // Fear fades over time

    bIsStarving         = false;
    bIsDehydrated       = false;
    bIsExhausted        = false;
    bIsFrozen           = false;
    bIsOverheating      = false;
    bIsDead             = false;

    CurrentBiome        = ECore_BiomeType::Savanna;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    // Survival tick is handled in TickComponent
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    // --- Drain hunger and thirst ---
    Hunger = FMath::Clamp(Hunger - HungerDrainRate * DeltaTime, 0.0f, MaxHunger);
    Thirst = FMath::Clamp(Thirst - ThirstDrainRate * DeltaTime, 0.0f, MaxThirst);

    // --- Fear decay ---
    Fear = FMath::Clamp(Fear - FearDecayRate * DeltaTime, 0.0f, MaxFear);

    // --- Status flags ---
    bIsStarving    = (Hunger <= 10.0f);
    bIsDehydrated  = (Thirst <= 10.0f);
    bIsExhausted   = (Stamina <= 5.0f);

    // --- Temperature extremes ---
    bIsFrozen      = (Temperature < 20.0f);
    bIsOverheating = (Temperature > 42.0f);

    // --- Health damage from critical states ---
    float HealthDrain = 0.0f;
    if (bIsStarving)    HealthDrain += 0.02f;
    if (bIsDehydrated)  HealthDrain += 0.04f;
    if (bIsFrozen)      HealthDrain += 0.03f;
    if (bIsOverheating) HealthDrain += 0.05f;

    if (HealthDrain > 0.0f)
    {
        ApplyDamage(HealthDrain * DeltaTime);
    }

    // --- Stamina regen when not exhausted ---
    if (!bIsExhausted && Stamina < MaxStamina)
    {
        Stamina = FMath::Clamp(Stamina + StaminaRegenRate * DeltaTime, 0.0f, MaxStamina);
    }

    // --- Death check ---
    if (Health <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        OnDeath.Broadcast();
    }
}

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (bIsDead) return;
    Health = FMath::Clamp(Health - Amount, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(Health, MaxHealth);
}

void USurvivalComponent::Heal(float Amount)
{
    Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(Health, MaxHealth);
}

void USurvivalComponent::Eat(float NutritionValue)
{
    Hunger = FMath::Clamp(Hunger + NutritionValue, 0.0f, MaxHunger);
    OnHungerChanged.Broadcast(Hunger, MaxHunger);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    Thirst = FMath::Clamp(Thirst + HydrationValue, 0.0f, MaxThirst);
    OnThirstChanged.Broadcast(Thirst, MaxThirst);
}

void USurvivalComponent::DrainStamina(float Amount)
{
    if (bIsExhausted) return;
    Stamina = FMath::Clamp(Stamina - Amount, 0.0f, MaxStamina);
    OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

void USurvivalComponent::AddFear(float Amount)
{
    Fear = FMath::Clamp(Fear + Amount, 0.0f, MaxFear);
    OnFearChanged.Broadcast(Fear, MaxFear);
}

void USurvivalComponent::SetBiome(ECore_BiomeType NewBiome)
{
    CurrentBiome = NewBiome;

    // Biome affects temperature and drain rates
    switch (NewBiome)
    {
        case ECore_BiomeType::Jungle:
            Temperature = 35.0f;
            ThirstDrainRate = 0.10f; // Humid but hot — sweat more
            HungerDrainRate = 0.05f;
            break;
        case ECore_BiomeType::Desert:
            Temperature = 45.0f;
            ThirstDrainRate = 0.15f; // Extreme dehydration
            HungerDrainRate = 0.06f;
            break;
        case ECore_BiomeType::Tundra:
            Temperature = 5.0f;
            ThirstDrainRate = 0.06f;
            HungerDrainRate = 0.08f; // Body burns more calories to stay warm
            break;
        case ECore_BiomeType::Swamp:
            Temperature = 30.0f;
            ThirstDrainRate = 0.07f;
            HungerDrainRate = 0.05f;
            break;
        case ECore_BiomeType::Savanna:
        default:
            Temperature = 37.0f;
            ThirstDrainRate = 0.08f;
            HungerDrainRate = 0.05f;
            break;
    }
}

FCore_SurvivalStats USurvivalComponent::GetCurrentStats() const
{
    FCore_SurvivalStats Stats;
    Stats.Health        = Health;
    Stats.MaxHealth     = MaxHealth;
    Stats.Hunger        = Hunger;
    Stats.MaxHunger     = MaxHunger;
    Stats.Thirst        = Thirst;
    Stats.MaxThirst     = MaxThirst;
    Stats.Stamina       = Stamina;
    Stats.MaxStamina    = MaxStamina;
    Stats.Temperature   = Temperature;
    Stats.Fear          = Fear;
    Stats.bIsStarving   = bIsStarving;
    Stats.bIsDehydrated = bIsDehydrated;
    Stats.bIsExhausted  = bIsExhausted;
    Stats.bIsDead       = bIsDead;
    return Stats;
}

bool USurvivalComponent::CanSprint() const
{
    return !bIsExhausted && !bIsDead && Stamina > 10.0f;
}

bool USurvivalComponent::IsAlive() const
{
    return !bIsDead && Health > 0.0f;
}
