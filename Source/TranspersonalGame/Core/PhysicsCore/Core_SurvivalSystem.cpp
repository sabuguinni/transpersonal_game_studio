#include "Core_SurvivalSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UCore_SurvivalSystem::UCore_SurvivalSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    CurrentStats = FCore_SurvivalStats();
    Config = FCore_SurvivalConfig();
    CurrentSurvivalState = ECore_SurvivalState::Healthy;
}

void UCore_SurvivalSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize survival state
    UpdateSurvivalState();
    
    // Start survival tick timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            SurvivalTickTimer,
            [this]() { UpdateSurvivalStats(0.1f); },
            0.1f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_SurvivalSystem: Initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_SurvivalSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Additional tick-based updates if needed
    // Main updates are handled by timer for consistent intervals
}

void UCore_SurvivalSystem::UpdateSurvivalStats(float DeltaTime)
{
    // Process stat decay
    ProcessStatDecay(DeltaTime);
    
    // Process health effects from other stats
    ProcessHealthEffects();
    
    // Update survival state
    UpdateSurvivalState();
    
    // Check for death
    CheckForDeath();
}

void UCore_SurvivalSystem::ProcessStatDecay(float DeltaTime)
{
    // Hunger decay
    float OldHunger = CurrentStats.Hunger;
    CurrentStats.Hunger = ClampStat(CurrentStats.Hunger - (Config.HungerDecayRate * DeltaTime));
    if (OldHunger != CurrentStats.Hunger)
    {
        BroadcastStatChange(ECore_SurvivalStat::Hunger, CurrentStats.Hunger);
    }
    
    // Thirst decay (faster than hunger)
    float OldThirst = CurrentStats.Thirst;
    CurrentStats.Thirst = ClampStat(CurrentStats.Thirst - (Config.ThirstDecayRate * DeltaTime));
    if (OldThirst != CurrentStats.Thirst)
    {
        BroadcastStatChange(ECore_SurvivalStat::Thirst, CurrentStats.Thirst);
    }
    
    // Stamina recovery (when not sprinting)
    if (CurrentStats.Stamina < 100.0f)
    {
        float OldStamina = CurrentStats.Stamina;
        CurrentStats.Stamina = ClampStat(CurrentStats.Stamina + (Config.StaminaRecoveryRate * DeltaTime));
        if (OldStamina != CurrentStats.Stamina)
        {
            BroadcastStatChange(ECore_SurvivalStat::Stamina, CurrentStats.Stamina);
        }
    }
    
    // Fear decay (natural calming over time)
    if (CurrentStats.Fear > 0.0f)
    {
        float OldFear = CurrentStats.Fear;
        CurrentStats.Fear = ClampStat(CurrentStats.Fear - (Config.FearDecayRate * DeltaTime));
        if (OldFear != CurrentStats.Fear)
        {
            BroadcastStatChange(ECore_SurvivalStat::Fear, CurrentStats.Fear);
        }
    }
}

void UCore_SurvivalSystem::ProcessHealthEffects()
{
    float HealthDamage = 0.0f;
    
    // Health damage from starvation
    if (CurrentStats.Hunger <= Config.CriticalHungerThreshold)
    {
        HealthDamage += 1.0f; // 1 health per 0.1 seconds when starving
    }
    
    // Health damage from dehydration (more severe)
    if (CurrentStats.Thirst <= Config.CriticalThirstThreshold)
    {
        HealthDamage += 2.0f; // 2 health per 0.1 seconds when dehydrated
    }
    
    // Apply health damage
    if (HealthDamage > 0.0f)
    {
        ModifyHealth(-HealthDamage);
    }
}

void UCore_SurvivalSystem::UpdateSurvivalState()
{
    ECore_SurvivalState NewState = ECore_SurvivalState::Healthy;
    
    // Check critical conditions first
    if (CurrentStats.Health <= Config.CriticalHealthThreshold)
    {
        NewState = ECore_SurvivalState::Dying;
    }
    else if (CurrentStats.Fear >= Config.MaxFearThreshold)
    {
        NewState = ECore_SurvivalState::Terrified;
    }
    else if (CurrentStats.Thirst <= Config.CriticalThirstThreshold)
    {
        NewState = ECore_SurvivalState::Thirsty;
    }
    else if (CurrentStats.Hunger <= Config.CriticalHungerThreshold)
    {
        NewState = ECore_SurvivalState::Hungry;
    }
    else if (CurrentStats.Stamina <= 10.0f)
    {
        NewState = ECore_SurvivalState::Exhausted;
    }
    else if (CurrentStats.Temperature <= 0.0f)
    {
        NewState = ECore_SurvivalState::Cold;
    }
    else if (CurrentStats.Temperature >= 40.0f)
    {
        NewState = ECore_SurvivalState::Hot;
    }
    
    // Update state if changed
    if (NewState != CurrentSurvivalState)
    {
        CurrentSurvivalState = NewState;
        BroadcastStateChange(NewState);
    }
}

void UCore_SurvivalSystem::CheckForDeath()
{
    if (CurrentStats.Health <= 0.0f)
    {
        OnPlayerDeath.Broadcast();
        UE_LOG(LogTemp, Warning, TEXT("Core_SurvivalSystem: Player has died!"));
    }
}

void UCore_SurvivalSystem::ModifyHealth(float Amount)
{
    float OldHealth = CurrentStats.Health;
    CurrentStats.Health = ClampStat(CurrentStats.Health + Amount);
    
    if (OldHealth != CurrentStats.Health)
    {
        BroadcastStatChange(ECore_SurvivalStat::Health, CurrentStats.Health);
        UE_LOG(LogTemp, Log, TEXT("Health modified by %.1f, new value: %.1f"), Amount, CurrentStats.Health);
    }
}

void UCore_SurvivalSystem::ModifyHunger(float Amount)
{
    float OldHunger = CurrentStats.Hunger;
    CurrentStats.Hunger = ClampStat(CurrentStats.Hunger + Amount);
    
    if (OldHunger != CurrentStats.Hunger)
    {
        BroadcastStatChange(ECore_SurvivalStat::Hunger, CurrentStats.Hunger);
    }
}

void UCore_SurvivalSystem::ModifyThirst(float Amount)
{
    float OldThirst = CurrentStats.Thirst;
    CurrentStats.Thirst = ClampStat(CurrentStats.Thirst + Amount);
    
    if (OldThirst != CurrentStats.Thirst)
    {
        BroadcastStatChange(ECore_SurvivalStat::Thirst, CurrentStats.Thirst);
    }
}

void UCore_SurvivalSystem::ModifyStamina(float Amount)
{
    float OldStamina = CurrentStats.Stamina;
    CurrentStats.Stamina = ClampStat(CurrentStats.Stamina + Amount);
    
    if (OldStamina != CurrentStats.Stamina)
    {
        BroadcastStatChange(ECore_SurvivalStat::Stamina, CurrentStats.Stamina);
    }
}

void UCore_SurvivalSystem::ModifyFear(float Amount)
{
    float OldFear = CurrentStats.Fear;
    CurrentStats.Fear = ClampStat(CurrentStats.Fear + Amount);
    
    if (OldFear != CurrentStats.Fear)
    {
        BroadcastStatChange(ECore_SurvivalStat::Fear, CurrentStats.Fear);
    }
}

void UCore_SurvivalSystem::SetTemperature(float NewTemperature)
{
    float OldTemperature = CurrentStats.Temperature;
    CurrentStats.Temperature = FMath::Clamp(NewTemperature, -20.0f, 50.0f);
    
    if (OldTemperature != CurrentStats.Temperature)
    {
        BroadcastStatChange(ECore_SurvivalStat::Temperature, CurrentStats.Temperature);
    }
}

float UCore_SurvivalSystem::GetStatValue(ECore_SurvivalStat StatType) const
{
    switch (StatType)
    {
        case ECore_SurvivalStat::Health:
            return CurrentStats.Health;
        case ECore_SurvivalStat::Hunger:
            return CurrentStats.Hunger;
        case ECore_SurvivalStat::Thirst:
            return CurrentStats.Thirst;
        case ECore_SurvivalStat::Stamina:
            return CurrentStats.Stamina;
        case ECore_SurvivalStat::Fear:
            return CurrentStats.Fear;
        case ECore_SurvivalStat::Temperature:
            return CurrentStats.Temperature;
        default:
            return 0.0f;
    }
}

bool UCore_SurvivalSystem::IsStatCritical(ECore_SurvivalStat StatType) const
{
    switch (StatType)
    {
        case ECore_SurvivalStat::Health:
            return CurrentStats.Health <= Config.CriticalHealthThreshold;
        case ECore_SurvivalStat::Hunger:
            return CurrentStats.Hunger <= Config.CriticalHungerThreshold;
        case ECore_SurvivalStat::Thirst:
            return CurrentStats.Thirst <= Config.CriticalThirstThreshold;
        case ECore_SurvivalStat::Fear:
            return CurrentStats.Fear >= Config.MaxFearThreshold;
        case ECore_SurvivalStat::Stamina:
            return CurrentStats.Stamina <= 10.0f;
        case ECore_SurvivalStat::Temperature:
            return CurrentStats.Temperature <= 0.0f || CurrentStats.Temperature >= 40.0f;
        default:
            return false;
    }
}

void UCore_SurvivalSystem::ResetAllStats()
{
    CurrentStats = FCore_SurvivalStats();
    CurrentSurvivalState = ECore_SurvivalState::Healthy;
    
    // Broadcast all stat changes
    BroadcastStatChange(ECore_SurvivalStat::Health, CurrentStats.Health);
    BroadcastStatChange(ECore_SurvivalStat::Hunger, CurrentStats.Hunger);
    BroadcastStatChange(ECore_SurvivalStat::Thirst, CurrentStats.Thirst);
    BroadcastStatChange(ECore_SurvivalStat::Stamina, CurrentStats.Stamina);
    BroadcastStatChange(ECore_SurvivalStat::Fear, CurrentStats.Fear);
    BroadcastStatChange(ECore_SurvivalStat::Temperature, CurrentStats.Temperature);
    
    BroadcastStateChange(CurrentSurvivalState);
}

void UCore_SurvivalSystem::TakeDamage(float DamageAmount, bool bFromDinosaur)
{
    ModifyHealth(-DamageAmount);
    
    // Increase fear when taking damage from dinosaurs
    if (bFromDinosaur)
    {
        ModifyFear(DamageAmount * 2.0f); // Fear increases more than damage taken
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_SurvivalSystem: Took %.1f damage%s"), 
           DamageAmount, bFromDinosaur ? TEXT(" from dinosaur") : TEXT(""));
}

float UCore_SurvivalSystem::ClampStat(float Value, float Min, float Max) const
{
    return FMath::Clamp(Value, Min, Max);
}

void UCore_SurvivalSystem::BroadcastStatChange(ECore_SurvivalStat StatType, float NewValue)
{
    OnSurvivalStatChanged.Broadcast(StatType, NewValue);
}

void UCore_SurvivalSystem::BroadcastStateChange(ECore_SurvivalState NewState)
{
    OnSurvivalStateChanged.Broadcast(NewState);
    
    // Log state changes
    FString StateName = UEnum::GetValueAsString(NewState);
    UE_LOG(LogTemp, Log, TEXT("Core_SurvivalSystem: State changed to %s"), *StateName);
}