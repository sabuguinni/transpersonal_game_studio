// SurvivalComponent.cpp
// Transpersonal Game Studio — Agent #4 Performance Optimizer
// Survival stats: health, hunger, thirst, stamina, temperature, fear
// Optimized: tick groups, conditional updates, no per-frame allocations

#include "SurvivalComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    PrimaryComponentTick.TickInterval = 0.25f; // 4Hz — sufficient for survival stats

    // Default stat values
    Health = 100.0f;
    MaxHealth = 100.0f;
    Hunger = 100.0f;
    MaxHunger = 100.0f;
    Thirst = 100.0f;
    MaxThirst = 100.0f;
    Stamina = 100.0f;
    MaxStamina = 100.0f;
    Temperature = 37.0f; // Normal body temp Celsius
    Fear = 0.0f;
    MaxFear = 100.0f;

    // Drain rates per second (real values, applied at 4Hz)
    HungerDrainRate = 0.5f;    // ~3.3 min to empty at rest
    ThirstDrainRate = 0.8f;    // ~2 min to empty at rest
    StaminaDrainRate = 5.0f;   // drains when sprinting
    StaminaRegenRate = 8.0f;   // regens when idle
    FearDecayRate = 2.0f;      // fear decays over time

    bIsSprinting = false;
    bIsStarving = false;
    bIsDehydrated = false;
    bIsExhausted = false;
    bIsDead = false;

    AccumulatedTime = 0.0f;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());

    // Set up 1-second timer for slow drain updates
    GetWorld()->GetTimerManager().SetTimer(
        SlowUpdateTimer,
        this,
        &USurvivalComponent::SlowUpdate,
        1.0f,
        true
    );
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    // Stamina tick — needs higher frequency for responsive feel
    UpdateStamina(DeltaTime);

    // Fear decay
    if (Fear > 0.0f)
    {
        Fear = FMath::Max(0.0f, Fear - FearDecayRate * DeltaTime);
    }
}

void USurvivalComponent::SlowUpdate()
{
    if (bIsDead) return;

    // Hunger drain
    Hunger = FMath::Max(0.0f, Hunger - HungerDrainRate);
    bIsStarving = (Hunger <= 0.0f);

    // Thirst drain
    Thirst = FMath::Max(0.0f, Thirst - ThirstDrainRate);
    bIsDehydrated = (Thirst <= 0.0f);

    // Health damage from starvation/dehydration
    if (bIsStarving || bIsDehydrated)
    {
        float HealthDrain = 0.0f;
        if (bIsStarving) HealthDrain += 1.0f;
        if (bIsDehydrated) HealthDrain += 2.0f; // Dehydration kills faster
        ApplyDamage(HealthDrain);
    }

    // Check death
    if (Health <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        OnDeath.Broadcast();
    }
}

void USurvivalComponent::UpdateStamina(float DeltaTime)
{
    if (!OwnerCharacter) return;

    UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
    if (!Movement) return;

    bool bIsMovingFast = Movement->Velocity.SizeSquared() > (300.0f * 300.0f);
    bIsSprinting = bIsMovingFast && Movement->MaxWalkSpeed > 400.0f;

    if (bIsSprinting && Stamina > 0.0f)
    {
        Stamina = FMath::Max(0.0f, Stamina - StaminaDrainRate * DeltaTime);
        bIsExhausted = (Stamina <= 0.0f);

        // Force slow walk when exhausted
        if (bIsExhausted)
        {
            Movement->MaxWalkSpeed = 200.0f;
        }
    }
    else if (!bIsSprinting && Stamina < MaxStamina)
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenRate * DeltaTime);
        if (bIsExhausted && Stamina > 25.0f)
        {
            bIsExhausted = false;
            Movement->MaxWalkSpeed = 600.0f; // Restore normal speed
        }
    }
}

void USurvivalComponent::ApplyDamage(float DamageAmount)
{
    if (bIsDead || DamageAmount <= 0.0f) return;

    Health = FMath::Max(0.0f, Health - DamageAmount);
    OnHealthChanged.Broadcast(Health, MaxHealth);

    if (Health <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        OnDeath.Broadcast();
    }
}

void USurvivalComponent::Eat(float NutritionValue)
{
    if (bIsDead) return;
    Hunger = FMath::Min(MaxHunger, Hunger + NutritionValue);
    bIsStarving = false;
    OnHungerChanged.Broadcast(Hunger, MaxHunger);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    if (bIsDead) return;
    Thirst = FMath::Min(MaxThirst, Thirst + HydrationValue);
    bIsDehydrated = false;
    OnThirstChanged.Broadcast(Thirst, MaxThirst);

    // Tutorial quest trigger: player found water
    OnWaterConsumed.Broadcast();
}

void USurvivalComponent::AddFear(float FearAmount)
{
    Fear = FMath::Min(MaxFear, Fear + FearAmount);
    OnFearChanged.Broadcast(Fear, MaxFear);
}

void USurvivalComponent::Heal(float HealAmount)
{
    if (bIsDead) return;
    Health = FMath::Min(MaxHealth, Health + HealAmount);
    OnHealthChanged.Broadcast(Health, MaxHealth);
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

bool USurvivalComponent::IsAlive() const
{
    return !bIsDead && Health > 0.0f;
}
