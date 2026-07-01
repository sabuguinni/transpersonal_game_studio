// SurvivalComponent.cpp
// Core Systems Programmer #03 — Cycle AUTO_20260701_004
// Full implementation of all survival mechanics

#include "SurvivalComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // tick at 10Hz, accumulate for 1s intervals
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    // Stats already initialised via FCore_SurvivalStats defaults
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: BeginPlay — Health=%.1f Hunger=%.1f Thirst=%.1f"),
        Stats.Health, Stats.Hunger, Stats.Thirst);
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive()) return;

    TickAccumulator += DeltaTime;
    if (TickAccumulator >= TickInterval)
    {
        ProcessSurvivalTick(TickAccumulator);
        TickAccumulator = 0.f;
    }
}

// ── Core tick ────────────────────────────────────────────────────────────────

void USurvivalComponent::ProcessSurvivalTick(float DeltaSeconds)
{
    // Drain hunger
    Stats.Hunger = FMath::Max(0.f, Stats.Hunger - HungerDrainRate * DeltaSeconds);

    // Drain thirst
    Stats.Thirst = FMath::Max(0.f, Stats.Thirst - ThirstDrainRate * DeltaSeconds);

    // Starvation damage
    if (Stats.Hunger <= 0.f)
    {
        Stats.Health = FMath::Max(0.f, Stats.Health - StarvationDamageRate * DeltaSeconds);
    }

    // Dehydration damage (more severe than starvation)
    if (Stats.Thirst <= 0.f)
    {
        Stats.Health = FMath::Max(0.f, Stats.Health - DehydrationDamageRate * DeltaSeconds);
    }

    // Stamina regen when not sprinting
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    bool bIsSprinting = false;
    if (OwnerChar)
    {
        UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement();
        if (MoveComp)
        {
            bIsSprinting = MoveComp->MaxWalkSpeed > 400.f && MoveComp->Velocity.SizeSquared() > 10000.f;
        }
    }

    if (!bIsSprinting && Stats.Stamina < Stats.MaxStamina)
    {
        Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + StaminaRegenRate * DeltaSeconds);
    }

    // Fear decay
    if (Stats.Fear > 0.f)
    {
        Stats.Fear = FMath::Max(0.f, Stats.Fear - FearDecayRate * DeltaSeconds);
    }

    // Temperature effects
    ApplyTemperatureEffects(DeltaSeconds);

    // Death check
    if (Stats.Health <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent: Owner %s has died."),
            GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void USurvivalComponent::ApplyTemperatureEffects(float DeltaSeconds)
{
    // Body temperature drifts toward ambient over time
    const float TempDrift = (AmbientTemperature - Stats.Temperature) * 0.01f * DeltaSeconds;
    Stats.Temperature = FMath::Clamp(Stats.Temperature + TempDrift, 20.f, 45.f);

    // Hypothermia: body temp < 35°C → health drain
    if (Stats.Temperature < 35.f)
    {
        const float Severity = (35.f - Stats.Temperature) / 15.f; // 0..1
        Stats.Health = FMath::Max(0.f, Stats.Health - Severity * 0.5f * DeltaSeconds);
    }

    // Hyperthermia: body temp > 40°C → health drain
    if (Stats.Temperature > 40.f)
    {
        const float Severity = (Stats.Temperature - 40.f) / 5.f; // 0..1
        Stats.Health = FMath::Max(0.f, Stats.Health - Severity * 1.0f * DeltaSeconds);
        // Also drains thirst faster
        Stats.Thirst = FMath::Max(0.f, Stats.Thirst - Severity * 0.5f * DeltaSeconds);
    }
}

// ── Status ────────────────────────────────────────────────────────────────────

ECore_SurvivalStatus USurvivalComponent::GetCurrentStatus() const
{
    if (Stats.Health <= 0.f)          return ECore_SurvivalStatus::Dead;
    if (Stats.Health < 20.f)          return ECore_SurvivalStatus::Critical;
    if (Stats.Temperature < 35.f)     return ECore_SurvivalStatus::Hypothermic;
    if (Stats.Temperature > 40.f)     return ECore_SurvivalStatus::Hyperthermic;
    if (Stats.Thirst < 20.f)          return ECore_SurvivalStatus::Thirsty;
    if (Stats.Hunger < 20.f)          return ECore_SurvivalStatus::Hungry;
    if (Stats.Stamina < 10.f)         return ECore_SurvivalStatus::Exhausted;
    return ECore_SurvivalStatus::Healthy;
}

// ── Modifiers ─────────────────────────────────────────────────────────────────

void USurvivalComponent::ApplyDamage(float Amount)
{
    if (Amount <= 0.f) return;
    Stats.Health = FMath::Max(0.f, Stats.Health - Amount);
    // Damage spikes fear
    Stats.Fear = FMath::Min(100.f, Stats.Fear + Amount * 0.5f);
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: ApplyDamage %.1f → Health=%.1f"), Amount, Stats.Health);
}

void USurvivalComponent::Heal(float Amount)
{
    if (Amount <= 0.f) return;
    Stats.Health = FMath::Min(Stats.MaxHealth, Stats.Health + Amount);
}

void USurvivalComponent::ConsumeFood(float Amount)
{
    if (Amount <= 0.f) return;
    Stats.Hunger = FMath::Min(100.f, Stats.Hunger + Amount);
}

void USurvivalComponent::ConsumeWater(float Amount)
{
    if (Amount <= 0.f) return;
    Stats.Thirst = FMath::Min(100.f, Stats.Thirst + Amount);
}

void USurvivalComponent::DrainStamina(float Amount)
{
    if (Amount <= 0.f) return;
    Stats.Stamina = FMath::Max(0.f, Stats.Stamina - Amount);
}

void USurvivalComponent::RestoreStamina(float Amount)
{
    if (Amount <= 0.f) return;
    Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + Amount);
}

void USurvivalComponent::SetFear(float NewFear)
{
    Stats.Fear = FMath::Clamp(NewFear, 0.f, 100.f);
}

void USurvivalComponent::SetAmbientTemperature(float AmbientCelsius)
{
    AmbientTemperature = FMath::Clamp(AmbientCelsius, -20.f, 60.f);
}
