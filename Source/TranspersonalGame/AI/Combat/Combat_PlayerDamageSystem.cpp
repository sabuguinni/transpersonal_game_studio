#include "Combat_PlayerDamageSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UCombat_PlayerDamageSystem::UCombat_PlayerDamageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    BaseDamageResistance = 0.0f;
    CurrentDamageResistance = BaseDamageResistance;
    InvulnerabilityDuration = 1.0f;
    LastDamageTime = -10.0f;
    
    DOTDamagePerSecond = 0.0f;
    DOTRemainingTime = 0.0f;
    DOTSource = nullptr;
    
    TemporaryResistance = 0.0f;
    TemporaryResistanceEndTime = 0.0f;
    
    MaxDamageHistoryEntries = 10;
}

void UCombat_PlayerDamageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    CurrentDamageResistance = BaseDamageResistance;
    LastDamageTime = -InvulnerabilityDuration;
    
    RecentDamageHistory.Empty();
}

void UCombat_PlayerDamageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateDamageOverTime(DeltaTime);
    UpdateTemporaryResistance(DeltaTime);
}

void UCombat_PlayerDamageSystem::ProcessDamage(float DamageAmount, const FVector& HitLocation, AActor* DamageSource)
{
    if (!IsPlayerAlive() || DamageAmount <= 0.0f)
    {
        return;
    }
    
    // Check invulnerability period
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastDamageTime < InvulnerabilityDuration)
    {
        return;
    }
    
    // Calculate final damage after resistance
    float FinalDamage = CalculateFinalDamage(DamageAmount);
    
    // Apply damage
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - FinalDamage);
    LastDamageTime = CurrentTime;
    
    // Create damage instance for history
    FCombat_DamageInstance DamageInstance;
    DamageInstance.DamageAmount = FinalDamage;
    DamageInstance.HitLocation = HitLocation;
    DamageInstance.DamageSource = DamageSource;
    DamageInstance.Timestamp = CurrentTime;
    
    AddDamageToHistory(DamageInstance);
    
    // Broadcast damage event
    OnPlayerDamaged.Broadcast(FinalDamage, DamageSource);
    
    // Check for death
    if (CurrentHealth <= 0.0f)
    {
        TriggerPlayerDeath();
    }
}

void UCombat_PlayerDamageSystem::ApplyInstantDamage(float DamageAmount, AActor* DamageSource)
{
    ProcessDamage(DamageAmount, GetOwner()->GetActorLocation(), DamageSource);
}

void UCombat_PlayerDamageSystem::ApplyDamageOverTime(float DamagePerSecond, float Duration, AActor* DamageSource)
{
    DOTDamagePerSecond = DamagePerSecond;
    DOTRemainingTime = Duration;
    DOTSource = DamageSource;
}

void UCombat_PlayerDamageSystem::HealPlayer(float HealAmount)
{
    if (HealAmount > 0.0f && IsPlayerAlive())
    {
        CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
    }
}

void UCombat_PlayerDamageSystem::SetMaxHealth(float NewMaxHealth)
{
    if (NewMaxHealth > 0.0f)
    {
        float HealthRatio = CurrentHealth / MaxHealth;
        MaxHealth = NewMaxHealth;
        CurrentHealth = MaxHealth * HealthRatio;
    }
}

float UCombat_PlayerDamageSystem::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
}

void UCombat_PlayerDamageSystem::SetDamageResistance(float ResistancePercentage)
{
    BaseDamageResistance = FMath::Clamp(ResistancePercentage, 0.0f, 1.0f);
    CurrentDamageResistance = BaseDamageResistance + TemporaryResistance;
    CurrentDamageResistance = FMath::Clamp(CurrentDamageResistance, 0.0f, 0.95f); // Max 95% resistance
}

void UCombat_PlayerDamageSystem::AddTemporaryResistance(float ResistancePercentage, float Duration)
{
    if (Duration > 0.0f)
    {
        TemporaryResistance = FMath::Clamp(ResistancePercentage, 0.0f, 1.0f);
        TemporaryResistanceEndTime = GetWorld()->GetTimeSeconds() + Duration;
        
        CurrentDamageResistance = BaseDamageResistance + TemporaryResistance;
        CurrentDamageResistance = FMath::Clamp(CurrentDamageResistance, 0.0f, 0.95f);
    }
}

void UCombat_PlayerDamageSystem::UpdateDamageOverTime(float DeltaTime)
{
    if (DOTRemainingTime > 0.0f && DOTDamagePerSecond > 0.0f)
    {
        float DOTDamage = DOTDamagePerSecond * DeltaTime;
        ProcessDamage(DOTDamage, GetOwner()->GetActorLocation(), DOTSource);
        
        DOTRemainingTime -= DeltaTime;
        if (DOTRemainingTime <= 0.0f)
        {
            DOTDamagePerSecond = 0.0f;
            DOTSource = nullptr;
        }
    }
}

void UCombat_PlayerDamageSystem::UpdateTemporaryResistance(float DeltaTime)
{
    if (TemporaryResistanceEndTime > 0.0f)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime >= TemporaryResistanceEndTime)
        {
            TemporaryResistance = 0.0f;
            TemporaryResistanceEndTime = 0.0f;
            CurrentDamageResistance = BaseDamageResistance;
        }
    }
}

void UCombat_PlayerDamageSystem::AddDamageToHistory(const FCombat_DamageInstance& DamageInstance)
{
    RecentDamageHistory.Add(DamageInstance);
    
    // Keep only the most recent entries
    while (RecentDamageHistory.Num() > MaxDamageHistoryEntries)
    {
        RecentDamageHistory.RemoveAt(0);
    }
}

float UCombat_PlayerDamageSystem::CalculateFinalDamage(float BaseDamage) const
{
    float DamageMultiplier = 1.0f - CurrentDamageResistance;
    return BaseDamage * DamageMultiplier;
}

void UCombat_PlayerDamageSystem::TriggerPlayerDeath()
{
    CurrentHealth = 0.0f;
    OnPlayerDeath.Broadcast();
}