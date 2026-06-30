// SurvivalComponent.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Full implementation of survival stat ticking, drain logic, biome integration.

#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // We manage our own interval via accumulator

    // Default stats — full bars at start
    Stats.Health  = 100.0f; Stats.MaxHealth  = 100.0f;
    Stats.Hunger  = 100.0f; Stats.MaxHunger  = 100.0f;
    Stats.Thirst  = 100.0f; Stats.MaxThirst  = 100.0f;
    Stats.Stamina = 100.0f; Stats.MaxStamina = 100.0f;
    Stats.Fear    = 0.0f;   Stats.MaxFear    = 100.0f;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = ECore_SurvivalState::Healthy;
    bIsAlive = true;
    bIsSprinting = false;
    SurvivalTickAccumulator = 0.0f;
    BiomeThirstMultiplier = 1.0f;
    BiomeHumidityHealBonus = 0.0f;
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAlive) return;

    SurvivalTickAccumulator += DeltaTime;
    if (SurvivalTickAccumulator >= SurvivalTickInterval)
    {
        TickSurvival(SurvivalTickAccumulator);
        SurvivalTickAccumulator = 0.0f;
    }
}

// ─── Core Survival Tick ───────────────────────────────────────────────────────

void USurvivalComponent::TickSurvival(float DeltaTime)
{
    if (!bIsAlive) return;

    const float SprintMultiplier = bIsSprinting ? 3.0f : 1.0f;

    // ── Hunger drain ──────────────────────────────────────────────────────
    const float HungerDrain = bIsSprinting
        ? DrainRates.HungerDrainSprint * DeltaTime
        : DrainRates.HungerDrainRest * DeltaTime;

    Stats.Hunger -= HungerDrain;
    ClampStat(Stats.Hunger, 0.0f, Stats.MaxHunger);
    BroadcastStatChange(ECore_SurvivalStat::Hunger, Stats.Hunger);

    // ── Thirst drain (biome temperature modulated) ────────────────────────
    const float ThirstDrain = bIsSprinting
        ? DrainRates.ThirstDrainSprint * DeltaTime * BiomeThirstMultiplier
        : DrainRates.ThirstDrainRest * DeltaTime * BiomeThirstMultiplier;

    Stats.Thirst -= ThirstDrain;
    ClampStat(Stats.Thirst, 0.0f, Stats.MaxThirst);
    BroadcastStatChange(ECore_SurvivalStat::Thirst, Stats.Thirst);

    // ── Stamina ───────────────────────────────────────────────────────────
    if (bIsSprinting)
    {
        Stats.Stamina -= DrainRates.StaminaDrainSprint * DeltaTime;
        if (Stats.Stamina <= ExhaustionThreshold)
        {
            SetSprinting(false); // Force stop sprinting when exhausted
        }
    }
    else
    {
        // Recover stamina at rest (slower when hungry/thirsty)
        const float RecoveryMult = (Stats.Hunger < StarvationThreshold || Stats.Thirst < DehydrationThreshold)
            ? 0.3f : 1.0f;
        Stats.Stamina += DrainRates.StaminaRecoveryRest * DeltaTime * RecoveryMult;
    }
    ClampStat(Stats.Stamina, 0.0f, Stats.MaxStamina);
    BroadcastStatChange(ECore_SurvivalStat::Stamina, Stats.Stamina);

    // ── Health drain from starvation / dehydration ────────────────────────
    float HealthDelta = 0.0f;

    if (Stats.Hunger <= StarvationThreshold)
    {
        HealthDelta -= DrainRates.HealthDrainStarving * DeltaTime;
    }
    if (Stats.Thirst <= DehydrationThreshold)
    {
        HealthDelta -= DrainRates.HealthDrainDehydrated * DeltaTime;
    }

    // Biome humidity bonus: humid biomes provide slight health regen
    if (BiomeHumidityHealBonus > 0.0f && Stats.Hunger > StarvationThreshold && Stats.Thirst > DehydrationThreshold)
    {
        HealthDelta += BiomeHumidityHealBonus * DeltaTime;
    }

    if (!FMath::IsNearlyZero(HealthDelta))
    {
        Stats.Health += HealthDelta;
        ClampStat(Stats.Health, 0.0f, Stats.MaxHealth);
        BroadcastStatChange(ECore_SurvivalStat::Health, Stats.Health);
    }

    // ── Fear decay (passive calm when no threats) ─────────────────────────
    if (Stats.Fear > 0.0f)
    {
        Stats.Fear -= DrainRates.FearDecayRate * DeltaTime;
        ClampStat(Stats.Fear, 0.0f, Stats.MaxFear);
        BroadcastStatChange(ECore_SurvivalStat::Fear, Stats.Fear);
    }

    // ── Death check ───────────────────────────────────────────────────────
    if (Stats.Health <= 0.0f)
    {
        bIsAlive = false;
        CurrentState = ECore_SurvivalState::Dead;
        OnSurvivalStateChanged.Broadcast(ECore_SurvivalState::Dead);
        OnPlayerDied.Broadcast();
        return;
    }

    // ── Update composite state ────────────────────────────────────────────
    UpdateSurvivalState();
}

// ─── State Machine ────────────────────────────────────────────────────────────

void USurvivalComponent::UpdateSurvivalState()
{
    ECore_SurvivalState NewState = ECore_SurvivalState::Healthy;

    if (Stats.Fear >= PanicThreshold)
    {
        NewState = ECore_SurvivalState::Panicking;
    }
    else if (Stats.Stamina <= ExhaustionThreshold)
    {
        NewState = ECore_SurvivalState::Exhausted;
    }
    else if (Stats.Thirst <= DehydrationThreshold)
    {
        NewState = ECore_SurvivalState::Dehydrated;
    }
    else if (Stats.Thirst <= 30.0f)
    {
        NewState = ECore_SurvivalState::Thirsty;
    }
    else if (Stats.Hunger <= StarvationThreshold)
    {
        NewState = ECore_SurvivalState::Starving;
    }
    else if (Stats.Hunger <= 30.0f)
    {
        NewState = ECore_SurvivalState::Hungry;
    }

    if (NewState != CurrentState)
    {
        CurrentState = NewState;
        OnSurvivalStateChanged.Broadcast(CurrentState);
    }
}

// ─── Stat Modifiers ───────────────────────────────────────────────────────────

void USurvivalComponent::ApplyDamage(float Amount, AActor* DamageCauser)
{
    if (!bIsAlive || Amount <= 0.0f) return;

    Stats.Health -= Amount;
    ClampStat(Stats.Health, 0.0f, Stats.MaxHealth);
    BroadcastStatChange(ECore_SurvivalStat::Health, Stats.Health);

    // Taking damage causes fear spike
    const float FearSpike = FMath::Clamp(Amount * 0.5f, 5.0f, 30.0f);
    AddFear(FearSpike);

    if (Stats.Health <= 0.0f)
    {
        bIsAlive = false;
        CurrentState = ECore_SurvivalState::Dead;
        OnSurvivalStateChanged.Broadcast(ECore_SurvivalState::Dead);
        OnPlayerDied.Broadcast();
    }
    else
    {
        UpdateSurvivalState();
    }
}

void USurvivalComponent::HealHealth(float Amount)
{
    if (!bIsAlive || Amount <= 0.0f) return;
    Stats.Health += Amount;
    ClampStat(Stats.Health, 0.0f, Stats.MaxHealth);
    BroadcastStatChange(ECore_SurvivalStat::Health, Stats.Health);
    UpdateSurvivalState();
}

void USurvivalComponent::ConsumeFood(float NutritionValue)
{
    if (!bIsAlive || NutritionValue <= 0.0f) return;
    Stats.Hunger += NutritionValue;
    ClampStat(Stats.Hunger, 0.0f, Stats.MaxHunger);
    BroadcastStatChange(ECore_SurvivalStat::Hunger, Stats.Hunger);
    UpdateSurvivalState();
}

void USurvivalComponent::DrinkWater(float HydrationValue)
{
    if (!bIsAlive || HydrationValue <= 0.0f) return;
    Stats.Thirst += HydrationValue;
    ClampStat(Stats.Thirst, 0.0f, Stats.MaxThirst);
    BroadcastStatChange(ECore_SurvivalStat::Thirst, Stats.Thirst);
    UpdateSurvivalState();
}

void USurvivalComponent::AddFear(float Amount)
{
    if (!bIsAlive || Amount <= 0.0f) return;
    Stats.Fear += Amount;
    ClampStat(Stats.Fear, 0.0f, Stats.MaxFear);
    BroadcastStatChange(ECore_SurvivalStat::Fear, Stats.Fear);
    UpdateSurvivalState();
}

void USurvivalComponent::ReduceFear(float Amount)
{
    if (Amount <= 0.0f) return;
    Stats.Fear -= Amount;
    ClampStat(Stats.Fear, 0.0f, Stats.MaxFear);
    BroadcastStatChange(ECore_SurvivalStat::Fear, Stats.Fear);
    UpdateSurvivalState();
}

void USurvivalComponent::RestoreStamina(float Amount)
{
    if (!bIsAlive || Amount <= 0.0f) return;
    Stats.Stamina += Amount;
    ClampStat(Stats.Stamina, 0.0f, Stats.MaxStamina);
    BroadcastStatChange(ECore_SurvivalStat::Stamina, Stats.Stamina);
    UpdateSurvivalState();
}

// ─── Sprint Control ───────────────────────────────────────────────────────────

void USurvivalComponent::SetSprinting(bool bSprinting)
{
    if (!bIsAlive) return;

    if (bSprinting && !CanSprint())
    {
        bIsSprinting = false;
        return;
    }

    bIsSprinting = bSprinting;

    // Propagate sprint speed to CharacterMovementComponent
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (OwnerChar)
    {
        UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement();
        if (MoveComp)
        {
            MoveComp->MaxWalkSpeed = bIsSprinting ? 600.0f : 300.0f;
        }
    }
}

bool USurvivalComponent::CanSprint() const
{
    return bIsAlive
        && Stats.Stamina > ExhaustionThreshold
        && CurrentState != ECore_SurvivalState::Dead
        && CurrentState != ECore_SurvivalState::Exhausted;
}

// ─── Biome Integration ────────────────────────────────────────────────────────

void USurvivalComponent::OnBiomeChanged(float NewTemperature, float NewHumidity)
{
    // Temperature range: 0 (arctic) to 1 (volcanic). Hot = faster thirst.
    // At temperature 0.5 (temperate) multiplier = 1.0
    // At temperature 1.0 (volcanic) multiplier = 2.5
    // At temperature 0.0 (arctic) multiplier = 0.5
    BiomeThirstMultiplier = FMath::Lerp(0.5f, 2.5f, NewTemperature);

    // Humidity range: 0 (desert) to 1 (swamp). Humid = slight health regen.
    // At humidity > 0.6, small passive heal (0.05/s max)
    BiomeHumidityHealBonus = FMath::Max(0.0f, (NewHumidity - 0.6f) * 0.125f);

    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent: Biome changed — Temp=%.2f ThirstMult=%.2f | Humidity=%.2f HealBonus=%.4f"),
           NewTemperature, BiomeThirstMultiplier, NewHumidity, BiomeHumidityHealBonus);
}

// ─── Stat Accessors ───────────────────────────────────────────────────────────

float USurvivalComponent::GetHealthPercent() const
{
    return Stats.MaxHealth > 0.0f ? Stats.Health / Stats.MaxHealth : 0.0f;
}

float USurvivalComponent::GetHungerPercent() const
{
    return Stats.MaxHunger > 0.0f ? Stats.Hunger / Stats.MaxHunger : 0.0f;
}

float USurvivalComponent::GetThirstPercent() const
{
    return Stats.MaxThirst > 0.0f ? Stats.Thirst / Stats.MaxThirst : 0.0f;
}

float USurvivalComponent::GetStaminaPercent() const
{
    return Stats.MaxStamina > 0.0f ? Stats.Stamina / Stats.MaxStamina : 0.0f;
}

float USurvivalComponent::GetFearPercent() const
{
    return Stats.MaxFear > 0.0f ? Stats.Fear / Stats.MaxFear : 0.0f;
}

// ─── Debug ────────────────────────────────────────────────────────────────────

void USurvivalComponent::PrintSurvivalStats() const
{
    UE_LOG(LogTemp, Log, TEXT("=== SurvivalComponent Stats ==="));
    UE_LOG(LogTemp, Log, TEXT("  Health:  %.1f / %.1f  (%.0f%%)"), Stats.Health,  Stats.MaxHealth,  GetHealthPercent()  * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("  Hunger:  %.1f / %.1f  (%.0f%%)"), Stats.Hunger,  Stats.MaxHunger,  GetHungerPercent()  * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("  Thirst:  %.1f / %.1f  (%.0f%%)"), Stats.Thirst,  Stats.MaxThirst,  GetThirstPercent()  * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("  Stamina: %.1f / %.1f  (%.0f%%)"), Stats.Stamina, Stats.MaxStamina, GetStaminaPercent() * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("  Fear:    %.1f / %.1f  (%.0f%%)"), Stats.Fear,    Stats.MaxFear,    GetFearPercent()    * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("  State:   %d | Alive: %s | Sprinting: %s"),
           (int32)CurrentState,
           bIsAlive ? TEXT("YES") : TEXT("NO"),
           bIsSprinting ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("  BiomeThirstMult: %.2f | BiomeHealBonus: %.4f"),
           BiomeThirstMultiplier, BiomeHumidityHealBonus);
}

// ─── Helpers ──────────────────────────────────────────────────────────────────

void USurvivalComponent::BroadcastStatChange(ECore_SurvivalStat Stat, float NewValue)
{
    OnStatChanged.Broadcast(Stat, NewValue);
}

void USurvivalComponent::ClampStat(float& Stat, float Min, float Max)
{
    Stat = FMath::Clamp(Stat, Min, Max);
}
