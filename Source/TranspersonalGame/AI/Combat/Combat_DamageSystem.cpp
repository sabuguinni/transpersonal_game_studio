#include "Combat_DamageSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"

UCombat_DamageSystem::UCombat_DamageSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize health stats
    HealthStats.MaxHealth = 100.0f;
    HealthStats.CurrentHealth = 100.0f;
    HealthStats.HealthRegenRate = 2.0f;
    HealthStats.DamageReduction = 0.0f;
    HealthStats.bIsInvulnerable = false;
    HealthStats.LastDamageTime = 0.0f;

    InvulnerabilityDuration = 0.5f;
    bAutoRegen = true;
    RegenDelay = 3.0f;
    bShowDamageNumbers = true;
    bScreenShakeOnDamage = true;
    ScreenShakeIntensity = 1.0f;
}

void UCombat_DamageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Ensure health is properly initialized
    if (HealthStats.CurrentHealth <= 0.0f)
    {
        HealthStats.CurrentHealth = HealthStats.MaxHealth;
    }

    // Broadcast initial health
    OnHealthChanged.Broadcast(GetHealthPercent());
}

void UCombat_DamageSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bAutoRegen && IsAlive() && !IsAtFullHealth())
    {
        ProcessHealthRegen(DeltaTime);
    }
}

void UCombat_DamageSystem::TakeDamage(const FCombat_DamageInfo& DamageInfo)
{
    if (!ShouldTakeDamage(DamageInfo))
    {
        return;
    }

    float FinalDamage = CalculateFinalDamage(DamageInfo);
    
    if (FinalDamage <= 0.0f)
    {
        return;
    }

    // Apply damage
    HealthStats.CurrentHealth = FMath::Max(0.0f, HealthStats.CurrentHealth - FinalDamage);
    HealthStats.LastDamageTime = GetWorld()->GetTimeSeconds();

    // Trigger effects
    ShowDamageEffect(DamageInfo);
    
    if (bScreenShakeOnDamage)
    {
        TriggerScreenShake(FinalDamage);
    }

    // Broadcast events
    OnDamageTaken.Broadcast(FinalDamage, DamageInfo);
    OnHealthChanged.Broadcast(GetHealthPercent());

    // Check for death
    if (HealthStats.CurrentHealth <= 0.0f)
    {
        HandleDeath();
    }

    UE_LOG(LogTemp, Warning, TEXT("Actor %s took %.1f damage, health: %.1f/%.1f"), 
           *GetOwner()->GetName(), FinalDamage, HealthStats.CurrentHealth, HealthStats.MaxHealth);
}

void UCombat_DamageSystem::Heal(float HealAmount)
{
    if (!IsAlive() || HealAmount <= 0.0f)
    {
        return;
    }

    float OldHealth = HealthStats.CurrentHealth;
    HealthStats.CurrentHealth = FMath::Min(HealthStats.MaxHealth, HealthStats.CurrentHealth + HealAmount);
    
    float ActualHeal = HealthStats.CurrentHealth - OldHealth;
    
    if (ActualHeal > 0.0f)
    {
        OnHealthChanged.Broadcast(GetHealthPercent());
        UE_LOG(LogTemp, Log, TEXT("Actor %s healed %.1f health, new health: %.1f/%.1f"), 
               *GetOwner()->GetName(), ActualHeal, HealthStats.CurrentHealth, HealthStats.MaxHealth);
    }
}

void UCombat_DamageSystem::SetHealth(float NewHealth)
{
    float ClampedHealth = FMath::Clamp(NewHealth, 0.0f, HealthStats.MaxHealth);
    
    if (ClampedHealth != HealthStats.CurrentHealth)
    {
        HealthStats.CurrentHealth = ClampedHealth;
        OnHealthChanged.Broadcast(GetHealthPercent());
        
        if (HealthStats.CurrentHealth <= 0.0f)
        {
            HandleDeath();
        }
    }
}

void UCombat_DamageSystem::SetMaxHealth(float NewMaxHealth)
{
    if (NewMaxHealth > 0.0f)
    {
        float HealthPercent = GetHealthPercent();
        HealthStats.MaxHealth = NewMaxHealth;
        HealthStats.CurrentHealth = HealthStats.MaxHealth * HealthPercent;
        OnHealthChanged.Broadcast(GetHealthPercent());
    }
}

void UCombat_DamageSystem::SetInvulnerable(bool bInvulnerable)
{
    HealthStats.bIsInvulnerable = bInvulnerable;
    
    if (bInvulnerable)
    {
        UE_LOG(LogTemp, Log, TEXT("Actor %s is now invulnerable"), *GetOwner()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Actor %s is no longer invulnerable"), *GetOwner()->GetName());
    }
}

void UCombat_DamageSystem::ResetHealth()
{
    HealthStats.CurrentHealth = HealthStats.MaxHealth;
    HealthStats.LastDamageTime = 0.0f;
    OnHealthChanged.Broadcast(1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Actor %s health reset to full"), *GetOwner()->GetName());
}

void UCombat_DamageSystem::Kill()
{
    if (IsAlive())
    {
        HealthStats.CurrentHealth = 0.0f;
        OnHealthChanged.Broadcast(0.0f);
        HandleDeath();
        
        UE_LOG(LogTemp, Warning, TEXT("Actor %s was killed"), *GetOwner()->GetName());
    }
}

float UCombat_DamageSystem::GetHealthPercent() const
{
    if (HealthStats.MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    
    return HealthStats.CurrentHealth / HealthStats.MaxHealth;
}

bool UCombat_DamageSystem::IsAtFullHealth() const
{
    return FMath::IsNearlyEqual(HealthStats.CurrentHealth, HealthStats.MaxHealth, 0.1f);
}

float UCombat_DamageSystem::CalculateFinalDamage(const FCombat_DamageInfo& DamageInfo) const
{
    float BaseDamage = DamageInfo.DamageAmount;
    
    // Apply damage reduction
    float DamageAfterReduction = BaseDamage * (1.0f - FMath::Clamp(HealthStats.DamageReduction, 0.0f, 0.95f));
    
    // Apply critical hit multiplier
    if (DamageInfo.bIsCriticalHit)
    {
        DamageAfterReduction *= 2.0f;
    }
    
    // Minimum damage threshold
    return FMath::Max(0.0f, DamageAfterReduction);
}

bool UCombat_DamageSystem::ShouldTakeDamage(const FCombat_DamageInfo& DamageInfo) const
{
    // Dead actors don't take damage
    if (!IsAlive())
    {
        return false;
    }
    
    // Invulnerable actors don't take damage
    if (HealthStats.bIsInvulnerable)
    {
        return false;
    }
    
    // Check for invulnerability frames after recent damage
    if (IsRecentlyDamaged())
    {
        return false;
    }
    
    // No self-damage
    if (DamageInfo.DamageSource == GetOwner())
    {
        return false;
    }
    
    return true;
}

void UCombat_DamageSystem::HandleDeath()
{
    if (!IsAlive())
    {
        OnDeath.Broadcast();
        UE_LOG(LogTemp, Warning, TEXT("Actor %s has died"), *GetOwner()->GetName());
    }
}

void UCombat_DamageSystem::ProcessHealthRegen(float DeltaTime)
{
    if (IsRecentlyDamaged())
    {
        return;
    }
    
    float RegenAmount = HealthStats.HealthRegenRate * DeltaTime;
    Heal(RegenAmount);
}

void UCombat_DamageSystem::ShowDamageEffect(const FCombat_DamageInfo& DamageInfo)
{
    if (!bShowDamageNumbers)
    {
        return;
    }
    
    // Log damage for now - in a full game this would spawn floating damage text
    FString DamageText = FString::Printf(TEXT("%.0f"), DamageInfo.DamageAmount);
    if (DamageInfo.bIsCriticalHit)
    {
        DamageText += TEXT(" CRIT!");
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Damage Effect: %s at location %s"), 
           *DamageText, *DamageInfo.HitLocation.ToString());
}

void UCombat_DamageSystem::TriggerScreenShake(float DamageAmount)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get player controller for screen shake
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        return;
    }
    
    // Scale shake intensity based on damage
    float ShakeScale = FMath::Clamp(DamageAmount / 50.0f, 0.1f, 2.0f) * ScreenShakeIntensity;
    
    // For now just log - in full game would use actual camera shake class
    UE_LOG(LogTemp, Log, TEXT("Screen shake triggered with intensity %.2f"), ShakeScale);
}

bool UCombat_DamageSystem::IsRecentlyDamaged() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    float TimeSinceLastDamage = GetWorld()->GetTimeSeconds() - HealthStats.LastDamageTime;
    return TimeSinceLastDamage < InvulnerabilityDuration;
}