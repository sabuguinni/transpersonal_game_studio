#include "Combat_DamageSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

UCombat_DamageSystem::UCombat_DamageSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    BaseArmor = 10.0f;
    BlockChance = 0.15f;
    CriticalResistance = 0.1f;
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    HealthRegenRate = 2.0f;
    bIsAlive = true;
    bIsInvulnerable = false;
    InvulnerabilityDuration = 1.0f;
}

void UCombat_DamageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    bIsAlive = true;
    bIsInvulnerable = false;
    
    StartHealthRegen();
}

FCombat_HitResult UCombat_DamageSystem::ApplyDamage(const FCombat_DamageData& DamageData, AActor* DamageSource)
{
    FCombat_HitResult HitResult;
    
    if (!CanTakeDamage())
    {
        return HitResult;
    }
    
    float FinalDamage = DamageData.BaseDamage;
    bool bWasCritical = false;
    bool bWasBlocked = false;
    
    // Roll for block
    if (RollBlock())
    {
        bWasBlocked = true;
        FinalDamage *= 0.5f; // Blocked attacks do half damage
    }
    
    // Roll for critical hit (reduced by critical resistance)
    float EffectiveCritChance = FMath::Max(0.0f, 0.1f - CriticalResistance);
    if (RollCritical(EffectiveCritChance))
    {
        bWasCritical = true;
        FinalDamage *= DamageData.CriticalMultiplier;
    }
    
    // Apply armor reduction
    FinalDamage = CalculateDamageReduction(FinalDamage, DamageData.ArmorPenetration);
    
    // Apply damage
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - FinalDamage);
    
    // Set up hit result
    HitResult.FinalDamage = FinalDamage;
    HitResult.bWasCritical = bWasCritical;
    HitResult.bWasBlocked = bWasBlocked;
    HitResult.HitLocation = GetOwner()->GetActorLocation();
    
    // Broadcast damage event
    OnDamageTaken.Broadcast(FinalDamage, bWasCritical, DamageSource);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    
    // Check for death
    if (CurrentHealth <= 0.0f && bIsAlive)
    {
        Kill();
    }
    
    // Set temporary invulnerability
    if (FinalDamage > 0.0f)
    {
        SetInvulnerable(true);
        GetWorld()->GetTimerManager().SetTimer(InvulnerabilityTimer, this, &UCombat_DamageSystem::EndInvulnerability, InvulnerabilityDuration, false);
    }
    
    return HitResult;
}

void UCombat_DamageSystem::Heal(float HealAmount)
{
    if (!bIsAlive || HealAmount <= 0.0f)
    {
        return;
    }
    
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
    
    if (CurrentHealth != OldHealth)
    {
        OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    }
}

void UCombat_DamageSystem::SetInvulnerable(bool bNewInvulnerable)
{
    bIsInvulnerable = bNewInvulnerable;
    
    if (!bNewInvulnerable && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(InvulnerabilityTimer);
    }
}

void UCombat_DamageSystem::Kill()
{
    if (!bIsAlive)
    {
        return;
    }
    
    bIsAlive = false;
    CurrentHealth = 0.0f;
    StopHealthRegen();
    
    OnDeath.Broadcast(GetOwner());
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UCombat_DamageSystem::Revive(float ReviveHealth)
{
    if (bIsAlive)
    {
        return;
    }
    
    bIsAlive = true;
    CurrentHealth = FMath::Clamp(ReviveHealth, 1.0f, MaxHealth);
    StartHealthRegen();
    
    OnRevive.Broadcast(GetOwner());
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float UCombat_DamageSystem::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
}

bool UCombat_DamageSystem::IsLowHealth() const
{
    return GetHealthPercentage() < 0.25f;
}

bool UCombat_DamageSystem::CanTakeDamage() const
{
    return bIsAlive && !bIsInvulnerable;
}

void UCombat_DamageSystem::StartHealthRegen()
{
    if (GetWorld() && HealthRegenRate > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(HealthRegenTimer, this, &UCombat_DamageSystem::RegenHealth, 1.0f, true);
    }
}

void UCombat_DamageSystem::StopHealthRegen()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HealthRegenTimer);
    }
}

void UCombat_DamageSystem::RegenHealth()
{
    if (bIsAlive && CurrentHealth < MaxHealth && CurrentHealth > 0.0f)
    {
        Heal(HealthRegenRate);
    }
}

void UCombat_DamageSystem::EndInvulnerability()
{
    SetInvulnerable(false);
}

float UCombat_DamageSystem::CalculateDamageReduction(float IncomingDamage, float ArmorPenetration) const
{
    float EffectiveArmor = FMath::Max(0.0f, BaseArmor - ArmorPenetration);
    float DamageReduction = EffectiveArmor / (EffectiveArmor + 100.0f);
    return IncomingDamage * (1.0f - DamageReduction);
}

bool UCombat_DamageSystem::RollCritical(float CritChance) const
{
    return FMath::RandRange(0.0f, 1.0f) < CritChance;
}

bool UCombat_DamageSystem::RollBlock() const
{
    return FMath::RandRange(0.0f, 1.0f) < BlockChance;
}