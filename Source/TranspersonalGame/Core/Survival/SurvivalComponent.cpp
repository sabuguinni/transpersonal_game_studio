// SurvivalComponent.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Full implementation of prehistoric survival stats:
//   Hunger, Thirst, Stamina, Fear, Temperature, Health decay

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — sufficient for survival stats

    // Initial stat values
    Health      = 100.f;
    Hunger      = 100.f;
    Thirst      = 100.f;
    Stamina     = 100.f;
    Fear        = 0.f;
    Temperature = 37.f;

    CurrentStatus = ECore_SurvivalStatus::Healthy;
    ThreatLevel   = ECore_ThreatLevel::None;
    bIsSprinting  = false;
    bIsAlive      = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    // Confirm alive at start
    bIsAlive = true;
    bDeathFired = false;
    bHealthCriticalFired = false;
    bStaminaCriticalFired = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent — drives all survival stat decay
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAlive) return;

    TickHunger(DeltaTime);
    TickThirst(DeltaTime);
    TickStamina(DeltaTime);
    TickFear(DeltaTime);
    TickTemperature(DeltaTime);
    TickHealthDecay(DeltaTime);
    UpdateStatus();
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick helpers
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::TickHunger(float DeltaTime)
{
    Hunger = FMath::Max(0.f, Hunger - HungerDrainRate * DeltaTime);
}

void USurvivalComponent::TickThirst(float DeltaTime)
{
    Thirst = FMath::Max(0.f, Thirst - ThirstDrainRate * DeltaTime);
}

void USurvivalComponent::TickStamina(float DeltaTime)
{
    if (bIsSprinting)
    {
        Stamina = FMath::Max(0.f, Stamina - StaminaSprintDrain * DeltaTime);

        // Auto-stop sprint when exhausted
        if (Stamina <= 0.f)
        {
            bIsSprinting = false;
            if (!bStaminaCriticalFired)
            {
                OnStaminaCritical.Broadcast(Stamina);
                bStaminaCriticalFired = true;
            }
        }
    }
    else
    {
        // Recover stamina — slower when hungry/thirsty
        float RecoveryMultiplier = 1.f;
        if (IsStarving())   RecoveryMultiplier *= 0.3f;
        if (IsDehydrated()) RecoveryMultiplier *= 0.3f;

        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRecoveryRate * RecoveryMultiplier * DeltaTime);

        // Reset critical flag once recovered past 20%
        if (Stamina > MaxStamina * 0.2f)
        {
            bStaminaCriticalFired = false;
        }
    }
}

void USurvivalComponent::TickFear(float DeltaTime)
{
    switch (ThreatLevel)
    {
        case ECore_ThreatLevel::None:
            Fear = FMath::Max(0.f, Fear - FearDecayRate * DeltaTime);
            break;
        case ECore_ThreatLevel::Distant:
            Fear = FMath::Min(MaxFear, Fear + 3.f * DeltaTime);
            break;
        case ECore_ThreatLevel::Nearby:
            Fear = FMath::Min(MaxFear, Fear + 8.f * DeltaTime);
            break;
        case ECore_ThreatLevel::Immediate:
            Fear = FMath::Min(MaxFear, Fear + 15.f * DeltaTime);
            break;
        case ECore_ThreatLevel::Attacking:
            Fear = FMath::Min(MaxFear, Fear + 25.f * DeltaTime);
            break;
        default:
            break;
    }
}

void USurvivalComponent::TickTemperature(float DeltaTime)
{
    // Body temperature drifts toward ambient over time
    float Delta = (AmbientTemperature - Temperature) * TemperatureAdaptRate * DeltaTime;
    Temperature = FMath::Clamp(Temperature + Delta, 20.f, 45.f);
}

void USurvivalComponent::TickHealthDecay(float DeltaTime)
{
    float Damage = 0.f;

    // Starvation damage
    if (Hunger <= 0.f)
    {
        Damage += StarvationDamageRate * DeltaTime;
    }

    // Dehydration damage (faster than starvation)
    if (Thirst <= 0.f)
    {
        Damage += DehydrationDamageRate * DeltaTime;
    }

    // Hypothermia damage (below 34°C)
    if (Temperature < 34.f)
    {
        float Severity = (34.f - Temperature) / 14.f; // 0..1 range
        Damage += Severity * 2.f * DeltaTime;
    }

    // Hyperthermia damage (above 40°C)
    if (Temperature > 40.f)
    {
        float Severity = (Temperature - 40.f) / 5.f;
        Damage += Severity * 2.f * DeltaTime;
    }

    if (Damage > 0.f)
    {
        Health = FMath::Max(0.f, Health - Damage);
    }

    // Critical health broadcast
    if (Health <= MaxHealth * 0.2f && !bHealthCriticalFired)
    {
        OnHealthCritical.Broadcast(Health);
        bHealthCriticalFired = true;
    }
    if (Health > MaxHealth * 0.3f)
    {
        bHealthCriticalFired = false;
    }

    // Death check
    if (Health <= 0.f && !bDeathFired)
    {
        Die();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Status update — determines the dominant survival status
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::UpdateStatus()
{
    ECore_SurvivalStatus NewStatus = ECore_SurvivalStatus::Healthy;

    // Priority order: Critical > Panicking > Dehydrated > Starving > Exhausted > Thirsty > Hungry > Hypothermic > Hyperthermic > Healthy
    if (Health <= MaxHealth * 0.15f)
    {
        NewStatus = ECore_SurvivalStatus::Critical;
    }
    else if (Fear >= MaxFear * 0.8f)
    {
        NewStatus = ECore_SurvivalStatus::Panicking;
    }
    else if (Thirst <= 0.f)
    {
        NewStatus = ECore_SurvivalStatus::Dehydrated;
    }
    else if (Hunger <= 0.f)
    {
        NewStatus = ECore_SurvivalStatus::Starving;
    }
    else if (Stamina <= 0.f)
    {
        NewStatus = ECore_SurvivalStatus::Exhausted;
    }
    else if (Thirst < MaxThirst * 0.3f)
    {
        NewStatus = ECore_SurvivalStatus::Thirsty;
    }
    else if (Hunger < MaxHunger * 0.3f)
    {
        NewStatus = ECore_SurvivalStatus::Hungry;
    }
    else if (Temperature < 34.f)
    {
        NewStatus = ECore_SurvivalStatus::Hypothermic;
    }
    else if (Temperature > 40.f)
    {
        NewStatus = ECore_SurvivalStatus::Hyperthermic;
    }

    if (NewStatus != CurrentStatus)
    {
        CurrentStatus = NewStatus;
        OnStatusChanged.Broadcast(CurrentStatus);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Die
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::Die()
{
    bIsAlive = false;
    bDeathFired = true;
    Health = 0.f;
    OnCharacterDeath.Broadcast();

    // Notify owning character to trigger death animation/ragdoll
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (OwnerChar)
    {
        UCharacterMovementComponent* Movement = OwnerChar->GetCharacterMovement();
        if (Movement)
        {
            Movement->DisableMovement();
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Actions
// ─────────────────────────────────────────────────────────────────────────────

void USurvivalComponent::Eat(float NutritionValue)
{
    if (!bIsAlive) return;
    Hunger = FMath::Min(MaxHunger, Hunger + NutritionValue);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    if (!bIsAlive) return;
    Thirst = FMath::Min(MaxThirst, Thirst + HydrationValue);
}

void USurvivalComponent::Rest(float RestValue)
{
    if (!bIsAlive) return;
    Stamina = FMath::Min(MaxStamina, Stamina + RestValue);
    Fear    = FMath::Max(0.f, Fear - RestValue * 0.5f);
}

void USurvivalComponent::TakeDamage_Survival(float DamageAmount, AActor* DamageCauser)
{
    if (!bIsAlive) return;
    Health = FMath::Max(0.f, Health - DamageAmount);

    // Seeing your attacker spikes fear
    if (DamageCauser)
    {
        Fear = FMath::Min(MaxFear, Fear + DamageAmount * 0.5f);
    }

    if (Health <= 0.f && !bDeathFired)
    {
        Die();
    }
}

void USurvivalComponent::SetSprinting(bool bSprinting)
{
    // Cannot sprint when exhausted or panicking
    if (bSprinting && (Stamina <= 0.f || CurrentStatus == ECore_SurvivalStatus::Exhausted))
    {
        bIsSprinting = false;
        return;
    }
    bIsSprinting = bSprinting;
}

void USurvivalComponent::SetThreatLevel(ECore_ThreatLevel NewThreatLevel)
{
    ThreatLevel = NewThreatLevel;
}

void USurvivalComponent::SetAmbientTemperature(float AmbientCelsius)
{
    AmbientTemperature = FMath::Clamp(AmbientCelsius, -20.f, 60.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Queries
// ─────────────────────────────────────────────────────────────────────────────

FCore_SurvivalSnapshot USurvivalComponent::GetSnapshot() const
{
    FCore_SurvivalSnapshot Snap;
    Snap.Health      = Health;
    Snap.Hunger      = Hunger;
    Snap.Thirst      = Thirst;
    Snap.Stamina     = Stamina;
    Snap.Fear        = Fear;
    Snap.Temperature = Temperature;
    Snap.Status      = CurrentStatus;
    return Snap;
}

bool USurvivalComponent::CanSprint() const
{
    return bIsAlive && Stamina > 0.f && CurrentStatus != ECore_SurvivalStatus::Exhausted;
}

bool USurvivalComponent::IsStarving() const
{
    return Hunger <= 0.f;
}

bool USurvivalComponent::IsDehydrated() const
{
    return Thirst <= 0.f;
}

bool USurvivalComponent::IsPanicking() const
{
    return Fear >= MaxFear * 0.8f;
}

float USurvivalComponent::GetHealthPercent() const
{
    return (MaxHealth > 0.f) ? (Health / MaxHealth) : 0.f;
}

float USurvivalComponent::GetStaminaPercent() const
{
    return (MaxStamina > 0.f) ? (Stamina / MaxStamina) : 0.f;
}
