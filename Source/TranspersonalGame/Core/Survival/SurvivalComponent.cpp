// SurvivalComponent.cpp
// Core Systems Programmer #03 — Cycle AUTO_20260620_005
// Survival stats system — hunger, thirst, temperature, stamina, health
// Integrated into TranspersonalCharacter

#include "SurvivalComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

USurvivalComponent::USurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;  // Tick every second (not every frame)

    // Default survival stats
    Stats.Health = 100.0f;
    Stats.MaxHealth = 100.0f;
    Stats.Hunger = 100.0f;
    Stats.MaxHunger = 100.0f;
    Stats.Thirst = 100.0f;
    Stats.MaxThirst = 100.0f;
    Stats.Stamina = 100.0f;
    Stats.MaxStamina = 100.0f;
    Stats.Temperature = 37.0f;  // Normal body temp Celsius
    Stats.Fear = 0.0f;
    Stats.MaxFear = 100.0f;
    Stats.Status = ECore_SurvivalStatus::Healthy;

    // Depletion rates (per second)
    HungerDepletionRate = 0.05f;   // Full hunger lasts ~33 minutes
    ThirstDepletionRate = 0.08f;   // Full thirst lasts ~20 minutes
    StaminaRegenRate = 5.0f;       // Regen 5/s when resting
    StaminaDrainRate = 10.0f;      // Drain 10/s when sprinting
    FearDecayRate = 2.0f;          // Fear decays 2/s when safe

    bIsSprinting = false;
    bIsResting = false;
    bInDanger = false;

    // Critical thresholds
    HungerCriticalThreshold = 20.0f;
    ThirstCriticalThreshold = 20.0f;
    StaminaExhaustedThreshold = 10.0f;
    FearPanicThreshold = 80.0f;
}

void USurvivalComponent::BeginPlay()
{
    Super::BeginPlay();

    // Metabolism timer — runs every 5 seconds for hunger/thirst drain
    GetWorld()->GetTimerManager().SetTimer(
        MetabolismTimerHandle,
        this,
        &USurvivalComponent::TickMetabolism,
        5.0f,
        true,
        5.0f
    );

    // Temperature regulation timer — runs every 10 seconds
    GetWorld()->GetTimerManager().SetTimer(
        TemperatureTimerHandle,
        this,
        &USurvivalComponent::TickTemperature,
        10.0f,
        true,
        10.0f
    );

    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent[%s]: Initialized — survival simulation active"),
        *GetOwner()->GetActorLabel());
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Stamina — real-time drain/regen
    if (bIsSprinting && Stats.Stamina > 0.0f)
    {
        Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - StaminaDrainRate * DeltaTime);
        if (Stats.Stamina <= StaminaExhaustedThreshold)
        {
            OnStaminaExhausted.Broadcast();
            bIsSprinting = false;  // Force stop sprint
        }
    }
    else if (!bIsSprinting && Stats.Stamina < Stats.MaxStamina)
    {
        float RegenMultiplier = bIsResting ? 2.0f : 1.0f;
        Stats.Stamina = FMath::Min(Stats.MaxStamina, Stats.Stamina + StaminaRegenRate * RegenMultiplier * DeltaTime);
    }

    // Fear — decay when not in danger
    if (!bInDanger && Stats.Fear > 0.0f)
    {
        Stats.Fear = FMath::Max(0.0f, Stats.Fear - FearDecayRate * DeltaTime);
    }

    // Update status
    UpdateSurvivalStatus();
}

void USurvivalComponent::TickMetabolism()
{
    // Hunger drain
    float PreviousHunger = Stats.Hunger;
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - HungerDepletionRate * 5.0f);

    if (Stats.Hunger <= 0.0f)
    {
        // Starving — deal health damage
        ApplyDamage(1.0f, ECore_DamageType::Starvation);
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent[%s]: STARVING — health damage applied"), *GetOwner()->GetActorLabel());
    }
    else if (Stats.Hunger <= HungerCriticalThreshold && PreviousHunger > HungerCriticalThreshold)
    {
        OnHungerCritical.Broadcast();
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent[%s]: HUNGER CRITICAL — %.1f"), *GetOwner()->GetActorLabel(), Stats.Hunger);
    }

    // Thirst drain
    float PreviousThirst = Stats.Thirst;
    Stats.Thirst = FMath::Max(0.0f, Stats.Thirst - ThirstDepletionRate * 5.0f);

    if (Stats.Thirst <= 0.0f)
    {
        // Dehydrated — deal health damage (faster than starvation)
        ApplyDamage(2.0f, ECore_DamageType::Dehydration);
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent[%s]: DEHYDRATED — health damage applied"), *GetOwner()->GetActorLabel());
    }
    else if (Stats.Thirst <= ThirstCriticalThreshold && PreviousThirst > ThirstCriticalThreshold)
    {
        OnThirstCritical.Broadcast();
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent[%s]: THIRST CRITICAL — %.1f"), *GetOwner()->GetActorLabel(), Stats.Thirst);
    }
}

void USurvivalComponent::TickTemperature()
{
    // Temperature regulation — ambient affects body temp
    float AmbientTemp = GetAmbientTemperature();
    float TempDiff = AmbientTemp - Stats.Temperature;

    // Body slowly moves toward ambient (0.1°C per tick when unprotected)
    Stats.Temperature += TempDiff * 0.02f;
    Stats.Temperature = FMath::Clamp(Stats.Temperature, 20.0f, 45.0f);

    // Hyperthermia / hypothermia damage
    if (Stats.Temperature > 41.0f)
    {
        ApplyDamage(1.5f, ECore_DamageType::Hyperthermia);
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent[%s]: HYPERTHERMIA — temp=%.1f°C"), *GetOwner()->GetActorLabel(), Stats.Temperature);
    }
    else if (Stats.Temperature < 35.0f)
    {
        ApplyDamage(1.0f, ECore_DamageType::Hypothermia);
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent[%s]: HYPOTHERMIA — temp=%.1f°C"), *GetOwner()->GetActorLabel(), Stats.Temperature);
    }
}

void USurvivalComponent::ApplyDamage(float Amount, ECore_DamageType DamageType)
{
    if (Amount <= 0.0f) return;

    float PreviousHealth = Stats.Health;
    Stats.Health = FMath::Max(0.0f, Stats.Health - Amount);

    UE_LOG(LogTemp, Verbose, TEXT("SurvivalComponent[%s]: Damage %.1f (type=%d) health %.1f->%.1f"),
        *GetOwner()->GetActorLabel(), Amount, (int32)DamageType, PreviousHealth, Stats.Health);

    OnHealthChanged.Broadcast(Stats.Health, Stats.MaxHealth);

    if (Stats.Health <= 0.0f && PreviousHealth > 0.0f)
    {
        OnDeath.Broadcast();
        UE_LOG(LogTemp, Log, TEXT("SurvivalComponent[%s]: DEATH"), *GetOwner()->GetActorLabel());
    }
}

void USurvivalComponent::Heal(float Amount)
{
    if (Amount <= 0.0f) return;
    Stats.Health = FMath::Min(Stats.MaxHealth, Stats.Health + Amount);
    OnHealthChanged.Broadcast(Stats.Health, Stats.MaxHealth);
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent[%s]: Healed %.1f — health=%.1f"), *GetOwner()->GetActorLabel(), Amount, Stats.Health);
}

void USurvivalComponent::Eat(float NutritionValue)
{
    if (NutritionValue <= 0.0f) return;
    Stats.Hunger = FMath::Min(Stats.MaxHunger, Stats.Hunger + NutritionValue);
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent[%s]: Ate %.1f nutrition — hunger=%.1f"), *GetOwner()->GetActorLabel(), NutritionValue, Stats.Hunger);
}

void USurvivalComponent::Drink(float HydrationValue)
{
    if (HydrationValue <= 0.0f) return;
    Stats.Thirst = FMath::Min(Stats.MaxThirst, Stats.Thirst + HydrationValue);
    UE_LOG(LogTemp, Log, TEXT("SurvivalComponent[%s]: Drank %.1f hydration — thirst=%.1f"), *GetOwner()->GetActorLabel(), HydrationValue, Stats.Thirst);
}

void USurvivalComponent::AddFear(float FearAmount)
{
    if (FearAmount <= 0.0f) return;
    float PreviousFear = Stats.Fear;
    Stats.Fear = FMath::Min(Stats.MaxFear, Stats.Fear + FearAmount);

    if (Stats.Fear >= FearPanicThreshold && PreviousFear < FearPanicThreshold)
    {
        OnPanic.Broadcast();
        UE_LOG(LogTemp, Warning, TEXT("SurvivalComponent[%s]: PANIC — fear=%.1f"), *GetOwner()->GetActorLabel(), Stats.Fear);
    }
}

void USurvivalComponent::SetSprinting(bool bSprinting)
{
    if (bSprinting && Stats.Stamina <= StaminaExhaustedThreshold)
    {
        return;  // Cannot sprint when exhausted
    }
    bIsSprinting = bSprinting;
}

void USurvivalComponent::SetResting(bool bResting)
{
    bIsResting = bResting;
}

void USurvivalComponent::SetInDanger(bool bDanger)
{
    bInDanger = bDanger;
    if (bDanger)
    {
        AddFear(10.0f);  // Being in danger adds fear
    }
}

FCore_SurvivalStats USurvivalComponent::GetSurvivalStats() const
{
    return Stats;
}

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

bool USurvivalComponent::IsAlive() const
{
    return Stats.Health > 0.0f;
}

bool USurvivalComponent::IsExhausted() const
{
    return Stats.Stamina <= StaminaExhaustedThreshold;
}

bool USurvivalComponent::IsStarving() const
{
    return Stats.Hunger <= 0.0f;
}

bool USurvivalComponent::IsDehydrated() const
{
    return Stats.Thirst <= 0.0f;
}

bool USurvivalComponent::IsInPanic() const
{
    return Stats.Fear >= FearPanicThreshold;
}

void USurvivalComponent::UpdateSurvivalStatus()
{
    ECore_SurvivalStatus NewStatus = ECore_SurvivalStatus::Healthy;

    if (!IsAlive())
    {
        NewStatus = ECore_SurvivalStatus::Dead;
    }
    else if (IsInPanic())
    {
        NewStatus = ECore_SurvivalStatus::Panicking;
    }
    else if (IsStarving() || IsDehydrated())
    {
        NewStatus = ECore_SurvivalStatus::Critical;
    }
    else if (Stats.Hunger <= HungerCriticalThreshold || Stats.Thirst <= ThirstCriticalThreshold || IsExhausted())
    {
        NewStatus = ECore_SurvivalStatus::Struggling;
    }
    else if (Stats.Health < Stats.MaxHealth * 0.5f)
    {
        NewStatus = ECore_SurvivalStatus::Injured;
    }

    if (NewStatus != Stats.Status)
    {
        Stats.Status = NewStatus;
        OnStatusChanged.Broadcast(NewStatus);
        UE_LOG(LogTemp, Log, TEXT("SurvivalComponent[%s]: Status changed to %d"), *GetOwner()->GetActorLabel(), (int32)NewStatus);
    }
}

float USurvivalComponent::GetAmbientTemperature() const
{
    // Default ambient temperature — 25°C (tropical prehistoric environment)
    // In future: query WeatherSystem / BiomeSystem for real ambient temp
    return 28.0f;
}

void USurvivalComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(MetabolismTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(TemperatureTimerHandle);
    Super::EndPlay(EndPlayReason);
}
