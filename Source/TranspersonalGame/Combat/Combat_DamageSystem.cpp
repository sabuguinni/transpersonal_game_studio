#include "Combat_DamageSystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCombat_DamageSystem::UCombat_DamageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    bCanTakeDamage = true;
    DamageReduction = 0.0f;
    InvulnerabilityDuration = 0.5f;
    bIsInvulnerable = false;
    bIsDead = false;
}

void UCombat_DamageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    bIsDead = false;
    bIsInvulnerable = false;
}

void UCombat_DamageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Health regeneration could be added here if needed
}

bool UCombat_DamageSystem::ApplyDamage(const FCombat_DamageInfo& DamageInfo)
{
    if (!ShouldTakeDamage(DamageInfo))
    {
        return false;
    }

    float FinalDamage = CalculateFinalDamage(DamageInfo.DamageAmount);
    
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - FinalDamage);
    
    // Broadcast damage event
    OnDamageTaken.Broadcast(FinalDamage, DamageInfo);
    
    // Trigger Blueprint event
    OnHealthChanged(CurrentHealth, MaxHealth);
    
    // Apply invulnerability if configured
    if (InvulnerabilityDuration > 0.0f)
    {
        SetInvulnerable(true);
        GetWorld()->GetTimerManager().SetTimer(InvulnerabilityTimer, this, &UCombat_DamageSystem::EndInvulnerability, InvulnerabilityDuration, false);
    }
    
    // Check for death
    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        Die();
    }
    
    return true;
}

void UCombat_DamageSystem::Heal(float HealAmount)
{
    if (bIsDead || HealAmount <= 0.0f)
    {
        return;
    }
    
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
    OnHealthChanged(CurrentHealth, MaxHealth);
}

void UCombat_DamageSystem::SetMaxHealth(float NewMaxHealth)
{
    if (NewMaxHealth <= 0.0f)
    {
        return;
    }
    
    float HealthRatio = MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 1.0f;
    MaxHealth = NewMaxHealth;
    CurrentHealth = MaxHealth * HealthRatio;
    
    OnHealthChanged(CurrentHealth, MaxHealth);
}

float UCombat_DamageSystem::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void UCombat_DamageSystem::SetInvulnerable(bool bNewInvulnerable)
{
    bIsInvulnerable = bNewInvulnerable;
    
    if (!bNewInvulnerable && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(InvulnerabilityTimer);
    }
}

void UCombat_DamageSystem::AddDamageImmunity(EDamageType DamageType)
{
    DamageImmunities.AddUnique(DamageType);
}

void UCombat_DamageSystem::RemoveDamageImmunity(EDamageType DamageType)
{
    DamageImmunities.Remove(DamageType);
}

void UCombat_DamageSystem::EndInvulnerability()
{
    bIsInvulnerable = false;
}

void UCombat_DamageSystem::Die()
{
    if (bIsDead)
    {
        return;
    }
    
    bIsDead = true;
    bCanTakeDamage = false;
    CurrentHealth = 0.0f;
    
    // Broadcast death event
    OnDeath.Broadcast(GetOwner());
    
    // Trigger Blueprint event
    OnActorDied();
}

bool UCombat_DamageSystem::ShouldTakeDamage(const FCombat_DamageInfo& DamageInfo) const
{
    // Check basic conditions
    if (!bCanTakeDamage || bIsDead || bIsInvulnerable)
    {
        return false;
    }
    
    // Check damage amount
    if (DamageInfo.DamageAmount <= 0.0f)
    {
        return false;
    }
    
    // Check immunities
    if (DamageImmunities.Contains(DamageInfo.DamageType))
    {
        return false;
    }
    
    return true;
}

float UCombat_DamageSystem::CalculateFinalDamage(float BaseDamage) const
{
    // Apply damage reduction
    float FinalDamage = BaseDamage * (1.0f - FMath::Clamp(DamageReduction, 0.0f, 1.0f));
    
    return FMath::Max(0.0f, FinalDamage);
}