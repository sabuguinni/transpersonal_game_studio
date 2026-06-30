// SurvivalComponent.cpp — Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260630_002
// Survival stats component: health, hunger, thirst, stamina, fear
// Attached to ATranspersonalCharacter in constructor

#include "SurvivalComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // Tick handled by owning character

    // Default survival stat values
    Health    = 100.f;
    MaxHealth = 100.f;
    Hunger    = 100.f;
    MaxHunger = 100.f;
    Thirst    = 100.f;
    MaxThirst = 100.f;
    Stamina   = 100.f;
    MaxStamina = 100.f;
    FearLevel = 0.f;
    MaxFear   = 100.f;

    bIsAlive  = true;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("USurvivalComponent: Initialized — Health=%.1f Hunger=%.1f Thirst=%.1f Stamina=%.1f Fear=%.1f"),
        Health, Hunger, Thirst, Stamina, FearLevel);
}

// ── Health ────────────────────────────────────────────────────────────────────

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (!bIsAlive || Amount <= 0.f) return;

    Health = FMath::Clamp(Health - Amount, 0.f, MaxHealth);
    UE_LOG(LogTemp, Warning, TEXT("USurvivalComponent: Damage %.1f applied — Health=%.1f"), Amount, Health);

    if (Health <= 0.f)
    {
        bIsAlive = false;
        OnDeath();
    }
}

void USurvivalComponent::RestoreHealth(float Amount)
{
    if (!bIsAlive || Amount <= 0.f) return;
    Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
    UE_LOG(LogTemp, Log, TEXT("USurvivalComponent: Health restored %.1f — Health=%.1f"), Amount, Health);
}

// ── Hunger ────────────────────────────────────────────────────────────────────

void USurvivalComponent::ConsumeFood(float NutritionValue)
{
    if (NutritionValue <= 0.f) return;
    Hunger = FMath::Clamp(Hunger + NutritionValue, 0.f, MaxHunger);
    UE_LOG(LogTemp, Log, TEXT("USurvivalComponent: Food consumed +%.1f — Hunger=%.1f"), NutritionValue, Hunger);
}

// ── Thirst ────────────────────────────────────────────────────────────────────

void USurvivalComponent::ConsumeWater(float HydrationValue)
{
    if (HydrationValue <= 0.f) return;
    Thirst = FMath::Clamp(Thirst + HydrationValue, 0.f, MaxThirst);
    UE_LOG(LogTemp, Log, TEXT("USurvivalComponent: Water consumed +%.1f — Thirst=%.1f"), HydrationValue, Thirst);
}

// ── Fear ──────────────────────────────────────────────────────────────────────

void USurvivalComponent::AddFear(float Amount)
{
    if (Amount <= 0.f) return;
    FearLevel = FMath::Clamp(FearLevel + Amount, 0.f, MaxFear);
    UE_LOG(LogTemp, Log, TEXT("USurvivalComponent: Fear +%.1f — FearLevel=%.1f"), Amount, FearLevel);
}

void USurvivalComponent::ReduceFear(float Amount)
{
    if (Amount <= 0.f) return;
    FearLevel = FMath::Clamp(FearLevel - Amount, 0.f, MaxFear);
}

// ── Stat Accessors ────────────────────────────────────────────────────────────

float USurvivalComponent::GetHealthPercent() const
{
    return MaxHealth > 0.f ? Health / MaxHealth : 0.f;
}

float USurvivalComponent::GetHungerPercent() const
{
    return MaxHunger > 0.f ? Hunger / MaxHunger : 0.f;
}

float USurvivalComponent::GetThirstPercent() const
{
    return MaxThirst > 0.f ? Thirst / MaxThirst : 0.f;
}

float USurvivalComponent::GetStaminaPercent() const
{
    return MaxStamina > 0.f ? Stamina / MaxStamina : 0.f;
}

float USurvivalComponent::GetFearPercent() const
{
    return MaxFear > 0.f ? FearLevel / MaxFear : 0.f;
}

bool USurvivalComponent::IsCriticalHealth() const
{
    return Health <= (MaxHealth * 0.2f); // Below 20% = critical
}

bool USurvivalComponent::IsStarving() const
{
    return Hunger <= 0.f;
}

bool USurvivalComponent::IsDehydrated() const
{
    return Thirst <= 0.f;
}

// ── Internal ──────────────────────────────────────────────────────────────────

void USurvivalComponent::OnDeath()
{
    UE_LOG(LogTemp, Error, TEXT("USurvivalComponent: OWNER DIED — Health=0"));
    // Broadcast death event — owning character handles ragdoll/respawn
    // OnOwnerDied.Broadcast(); // Uncomment when delegate is wired
}
